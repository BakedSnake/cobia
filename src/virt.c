#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200112L

#include <string.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>
#include <poll.h>
#include <stdbool.h>
#include <string.h>

#include "cobia.h"
#include "virt.h"
#include "xml.h"
#include "yaml.h"

static volatile int suppressed = 0;

void quiet_exit_console(void *opaque, virErrorPtr err) {
    if (!err) return;

    if (suppressed)
        if (err->code == VIR_ERR_OPERATION_FAILED && err->message &&
            strstr(err->message, "stream is closed"))
            return;

    virDefaultErrorFunc(err);
}

void *console_reader(void *op)
{
    int n;
    char buf[CONBUF_SIZE];
    Conctx *ctx = op;
    while(!ctx->done) {
        n = virStreamRecv(ctx->stream, buf, sizeof(buf));
        if (n <= 0) break;

        ssize_t offset = 0;
        while(offset < n) {
            ssize_t w = write(STDOUT_FILENO, buf + offset, n - offset);

            if (w < 0) {
                ctx->done = 1;
                goto done;
            }

            offset += w;
        }
    }

    done:
        ctx->done = 1;
        return NULL;
}

void start_console(virConnectPtr conn, const char* name)
{
    pthread_t ti;
    int raw_set = 0;
    bool thread_started = false;
    struct termios saved, raw;
    Conctx ctx = { .stream = NULL, .done = 0 };

    virDomainPtr dom = get_domain(conn, name);
    if (dom == NULL) goto clean; 

    virSetErrorFunc(NULL, quiet_exit_console);
    ctx.stream = virStreamNew(conn, 0);
    if (!ctx.stream) goto clean;

    int console = virDomainOpenConsole(dom, NULL, ctx.stream, VIR_DOMAIN_CONSOLE_SAFE);
    if (console < 0) goto clean;

    int rc = pthread_create(&ti, NULL, console_reader, &ctx);
    if (rc != 0) goto clean;
    thread_started = true;

    int sc = tcgetattr(STDIN_FILENO, &saved);
    if (sc == 0) {
        raw = saved;
        cfmakeraw(&raw);
        raw_set = (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0);
    }
    
    while (1) {
        if (ctx.done) break;
        struct pollfd pf = { .fd = STDIN_FILENO, .events = POLLIN };

        int pr = poll(&pf, 1, 200);
        if (pr == 0) continue;

        char ch;
        ssize_t n = read(STDIN_FILENO, &ch, 1);

        /* quit when stdin is closed or Ctrl-] is pressed */
        if (n <= 0 || ch == 0x1d) {
            goto done;
        }

        ssize_t offset = 0;
        while (offset < n) {
            ssize_t sent = virStreamSend(ctx.stream, &ch+offset, n-offset);
            if (sent < 0) {
                goto done;
            }

            offset += sent;
        }
    }

    done:
        ctx.done = 1;
        suppressed = 1;
        if (raw_set) tcsetattr(STDIN_FILENO, TCSANOW, &saved);
        fputc('\n', stdout);
        if (ctx.stream) virStreamFinish(ctx.stream);
        if (thread_started) {
            pthread_join(ti, NULL);
            thread_started = false;
        }
        suppressed = 0;

    clean:
        if (thread_started) {
            ctx.done = 1;
            suppressed = 1;
            if (ctx.stream) virStreamAbort(ctx.stream);
            pthread_join(ti, NULL);
            suppressed = 0;
            thread_started = false;
        }
        if (raw_set) tcsetattr(STDIN_FILENO, TCSANOW, &saved);
        if (ctx.stream) virStreamFree(ctx.stream);
        if (dom) virDomainFree(dom);
}

void show_dhcp_leases(virConnectPtr conn, const char* name)
{
    virNetworkDHCPLeasePtr *leases = NULL;
    virNetworkPtr net = get_network(conn, name);

    if (virNetworkGetDHCPLeases(net, NULL, &leases, 0) >= 0) {
        fprintf(stdout, "%-20s %-22s %-24s\n", "Host", "Ip", "Expiry");
        for (size_t i = 0; i < 60; ++i) {fputc('-', stdout);}
        fprintf(stdout, "\n");

        for (size_t i = 0; leases[i] != NULL; ++i) {
            fprintf(stdout, "%-20s %-22s %-24d\n",
                    leases[i]->hostname ? leases[i]->hostname : "( no hostname )",
                    leases[i]->ipaddr ? leases[i]->ipaddr : "( no ip )",
                    (int)leases[i]->expirytime ? (int)leases[i]->expirytime : 0);
            virNetworkDHCPLeaseFree(leases[i]);
        }

        free(leases);
    }
}

void create_network(virConnectPtr conn, const char *name)
{
    parse_yaml(name);
    xmlBufferPtr buf = xmlBufferCreate();
    if (!buf) return;
    const char* xml = create_network_xml(buf);
    printf("%s\n", xml);
    virNetworkDefineXML(conn, xml);
    xmlBufferFree(buf);
}

void delete_network(virConnectPtr conn, const char *name)
{
    virNetworkPtr net = get_network(conn, name);
    if (net != NULL) {
        virNetworkUndefine(net);
        fprintf(stdout, "Network %s was deleted\n", virNetworkGetName(net));
    }
}

void start_network(virConnectPtr conn, const char *name)
{
    virNetworkPtr net = get_network(conn, name);
    if (net != NULL) {
        virNetworkCreate(net);
    } else {
        fprintf(stderr, "Could not start network\n");
    }
}

void stop_network(virConnectPtr conn, const char *name)
{
    virNetworkPtr net = get_network(conn, name);
    if (net != NULL) {
        virNetworkDestroy(net);
    } else {
        fprintf(stderr, "Could not stop network\n");
    }
}

void create_domain(virConnectPtr conn, const char *name)
{
    fprintf(stdout, "%s:\n", name);
    parse_yaml(name);
    xmlBufferPtr buf = xmlBufferCreate();
    if (!buf) return;
    const char* xml = create_domain_xml(buf);
    printf("%s\n", xml);
    virDomainDefineXML(conn, xml);
    xmlBufferFree(buf);
}

void delete_domain(virConnectPtr conn, const char *name)
{
    virDomainPtr *doms = get_all_domains(conn);
    if (doms != NULL) {
        for (int i = 0; i < TOTAL_VM_COUNT; ++i) {
            const char *dom_name = virDomainGetName(doms[i]);
            if (doms[i] != NULL && strcmp(dom_name, name) == 0)
                virDomainUndefineFlags(doms[i], VIR_DOMAIN_UNDEFINE_KEEP_NVRAM);
        }
    }
}

void start_domain(virConnectPtr conn, const char *name)
{
    virDomainPtr *doms = get_all_domains(conn);
    if (doms != NULL) {
        for (int i = 0; i < TOTAL_VM_COUNT; ++i) {
            const char *dom_name = virDomainGetName(doms[i]);
            if (doms[i] != NULL && strcmp(dom_name, name) == 0) {
                virDomainCreate(doms[i]);
            }
        }
    }
}

void stop_domain(virConnectPtr conn, const char *name)
{
    virDomainPtr *doms = get_all_domains(conn);
    if (doms != NULL) {
        for (int i = 0; i < TOTAL_VM_COUNT; ++i) {
            const char *dom_name = virDomainGetName(doms[i]);
            if (doms[i] != NULL && strcmp(dom_name, name) == 0) {
                virDomainDestroy(doms[i]);
            }
        }
    }
}

virDomainPtr *get_all_domains(virConnectPtr conn)
{
    virDomainPtr *domains = NULL;
    TOTAL_VM_COUNT = virConnectListAllDomains(conn, &domains, 0);
    if (TOTAL_VM_COUNT > 0) {
        return domains;
    }

    return NULL;
}

virDomainPtr get_domain(virConnectPtr conn, const char *name)
{
    virDomainPtr *doms = get_all_domains(conn);
    if (doms != NULL) {
        for (int i = 0; i < TOTAL_VM_COUNT; ++i) {
            if (doms[i] != NULL && strcmp(virDomainGetName(doms[i]), name) == 0) {
                return doms[i];
            }
        }
    }

    return NULL;
}

void show_domain_xml(virConnectPtr conn, const char *name)
{
    virDomainPtr dom = get_domain(conn, name);
    if (dom != NULL) {
        fprintf(stdout, "%s\n", virDomainGetXMLDesc(dom, 0));
    }
}

void show_domains(virConnectPtr conn)
{
    virDomainPtr *doms = get_all_domains(conn);
    if (doms != NULL) {
        fprintf(stdout, "Domain List:\n");
        for (int i = 0; i < TOTAL_VM_COUNT; ++i) {
            if (doms[i]) {
                const char *name = virDomainGetName(doms[i]);
                int id = virDomainGetID(doms[i]);
                fprintf(stdout, "  %d - %s\n", id, name);
            }

            virDomainFree(doms[i]);
        }
    }
}

void show_domain_info(virConnectPtr conn, const char *name)
{
    virDomainPtr dom = get_domain(conn, name);
    if (dom != NULL) {
        virDomainInfo dm;
        fprintf(stdout, "%s:\n", name);
        if (virDomainGetInfo(dom, &dm) == 0) {
            fprintf(stdout, "  State: %s\n  Max Memory: %lu\n  CPU nr: %d\n",
                    dm.state == 1 ? "Running" : "Stopped", dm.maxMem, dm.nrVirtCpu);
        }

        if (dm.state == 1) {
            virDomainInterfacePtr *ifaces = NULL;
            int nc = virDomainInterfaceAddresses(dom, &ifaces, VIR_DOMAIN_INTERFACE_ADDRESSES_SRC_LEASE, 0);
            for (int i = 0; i < nc; ++i) {
                for (size_t j = 0; j < ifaces[i]->naddrs; ++j) {
                    fprintf(stdout, "  IP Address: %s\n", ifaces[i]->addrs[j].addr);
                }
            }
        }

        virDomainFree(dom);
    }
}

virNetworkPtr *get_all_networks(virConnectPtr conn)
{
    virNetworkPtr *networks = NULL;
    TOTAL_NET_COUNT = virConnectListAllNetworks(conn, &networks, 0);
    if (TOTAL_NET_COUNT > 0) {
        return networks;
    }

    return NULL;
}

virNetworkPtr get_network(virConnectPtr conn, const char *name)
{
    virNetworkPtr *networks = get_all_networks(conn);
    if (networks != NULL) {
        for (int i = 0; i < TOTAL_NET_COUNT; ++i) {
            if (networks[i] != NULL && strcmp(virNetworkGetName(networks[i]), name) == 0)
                return networks[i];
        }
    }

    return NULL;
}

void show_networks(virConnectPtr conn)
{
    virNetworkPtr *networks = get_all_networks(conn);
    if (networks != NULL) {
        fprintf(stdout, "Network List:\n");
        for (int i = 0; i < TOTAL_NET_COUNT; ++i) {
            if (networks[i]) {
                fprintf(stdout, "  - %s\n", virNetworkGetName(networks[i]));
            }
        }
    }
}

char *get_network_xml(virConnectPtr conn, const char *name)
{
    virNetworkPtr net = get_network(conn, name);
    if (net != NULL) {
        return virNetworkGetXMLDesc(net, 0);
    }

    return NULL;
}

void show_network_info(virConnectPtr conn, const char *name)
{
    char *netxml = get_network_xml(conn, name);
    if (netxml != NULL) {
        xmlDocPtr doc = xmlReadMemory(netxml, strlen(netxml), "n.xml", NULL, XML_PARSE_RECOVER | XML_PARSE_NOBLANKS);
        if (doc == NULL) {
            fprintf(stderr, "Failed to parse network xml\n");
            return;
        }

        fprintf(stdout, "Network Info:\n");
        fprintf(stdout, "  Name: %s\n  UUID: %s\n  Bridge Name: %s\n  Mode: %s\n  IP: %s\n  DHCP Range: %s - %s\n",
                xpath_get(doc, "/network/name"),
                xpath_get(doc, "/network/uuid"),
                xpath_get(doc, "/network/bridge/@name"),
                xpath_get(doc, "/network/forward/@mode"),
                xpath_get(doc, "/network/ip/@address"),
                xpath_get(doc, "/network/ip/dhcp/range/@start"),
                xpath_get(doc, "/network/ip/dhcp/range/@end"));
    }

    virNetworkPtr net = get_network(conn, name);
    if (net != NULL) {
        int state = virNetworkIsActive(net);
        if (state) {
            fprintf(stdout, "  State: Active\n");
        } else {
            fprintf(stdout, "  State: Inactive\n");
        }
    }

    return;
}
