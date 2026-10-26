#define _LARGEFILE64_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cobia.h"
#include "xml.h"
#include "virt.h"
#include "console.h"
#include "cfg.h"

int TOTAL_VM_COUNT          = 0;
int TOTAL_NET_COUNT         = 0;

keyval_t settings[MAXSET]   = {0};

static Command_Single l_cmds[] = {
    { "domains",    show_domains        },
    { "networks",   show_networks       },
};

static Command dom_cmds[] = {
    {"create",      create_domain       },
    {"info",        show_domain_info    },
    {"xml",         show_domain_xml     },
    {"start",       start_domain        },
    {"stop",        stop_domain         },
    {"delete",      delete_domain       },
    {"shell",       start_console       },
};

static Command net_cmds[] = {
    {"create",      create_network      },
    {"info",        show_network_info   },
    {"start",       start_network       },
    {"stop",        stop_network        },
    {"delete",      delete_network      },
    {"leases",      show_dhcp_leases    },
};

char* get_val(keyval_t pair[MAXSET], const char* keyname)
{
    for (size_t i = 0; i < MAXSET; ++i) {
        if (pair[i].key == NULL) return NULL;
        if (strcmp(pair[i].key, keyname) == 0) {
            if (pair[i].val != NULL) return pair[i].val;
        }
    }

    return NULL;
}

void exec_cmd(virConnectPtr conn, const char* arg,
        const char* target, void (*cmdf)(virConnectPtr c, const char* t))
{
    if (strcmp(target, arg) != 0) {
        if (strcmp(arg, "delete") == 0) {
            char* nvram = get_nvram_path(target);
            fprintf(stdout, "%s\n", nvram);
            free(nvram);
        }

        cmdf(conn, target);
    } else {
        fprintf(stderr, "target not specified\n");
        return;
    }
}

int main(int argc, char **argv) {
    virConnectPtr conn;

    if (argc < 2) {
        fprintf(stderr, "Please provide argument.\n");
        return 1;
    }

    conn = virConnectOpen("qemu:///system");
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to 'qemu:///system'.\n");
        return 1;
    }

    int e = argHandle(conn, argc, argv);
    if (e != 0) return 1;

    virConnectClose(conn);
    return 0;
}

int argHandle(virConnectPtr conn, int argc, char* argv[])
{
    int opt;
    const char * arg_str = "hvd:l:n:c:";
    size_t count = sizeof(dom_cmds)/sizeof(dom_cmds[0]);
    while ((opt = getopt_long(argc, argv, arg_str, options, NULL)) != -1) {
        switch (opt) {
            case 'c':
                fprintf(stdout, "%s\n", optarg);
                configure_network(optarg);
                break;
            case 'd':
                for (size_t i = 0; i < count; ++i) {
                    if (strcmp(optarg, dom_cmds[i].name) == 0) {
                        exec_cmd(conn, optarg, argv[argc-1], dom_cmds[i].cmd);
                        break;
                    }
                }
                break;
            case 'h':
                // TODO
            case 'l':
                for (size_t i = 0; i < count; ++i) {
                    if (strcmp(optarg, l_cmds[i].name) == 0) {
                        l_cmds[i].cmd(conn);
                        break;
                    }
                }
                break;
            case 'n':
                for (size_t i = 0; i < count; ++i) {
                    if (strcmp(optarg, net_cmds[i].name) == 0) {
                        exec_cmd(conn, optarg, argv[argc-1], net_cmds[i].cmd);
                        break;
                    }
                }
                break;
            case 'v':
                fprintf(stdout, "Cobia version: 0.0.0\n");
                break;
            case '?':
                default:
                break;
        }
    }

    for (size_t i = 0; i < MAXSET; ++i) {
        free(settings[i].key);
        free(settings[i].val);
    }
    return 0;
}
