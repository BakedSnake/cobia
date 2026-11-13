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
int TOTAL_KEY_COUNT         = 0;

keyval_t settings[MAXSET]   = {0};

static Command_Single l_cmds[] = {
    { "domains",    "List all domains",     show_domains        },
    { "networks",   "List all networks",    show_networks       },
};

static Command_Config cfg_cmds[] = {
    { "bootstrap",  "Bootstrap a domain",   bootstrap_domain    },
    { "netconf",    "configure nat",        configure_network   },
};

static Command dom_cmds[] = {
    {"create",      "Create a new domain",                  create_domain       },
    {"info",        "Show domain info",                     show_domain_info    },
    {"xml",         "Show domain xml",                      show_domain_xml     },
    {"start",       "Start up a domain",                    start_domain        },
    {"stop",        "Shut down a domain",                   stop_domain         },
    {"delete",      "Delete a domain",                      delete_domain       },
    {"shell",       "Connect to shell via serial console",  start_console       },
};

static Command net_cmds[] = {
    {"create",      "Create a new network",                 create_network      },
    {"info",        "Show network info",                    show_network_info   },
    {"start",       "Start up a network",                   start_network       },
    {"stop",        "Shutdown a network",                   stop_network        },
    {"delete",      "Delete a network",                     delete_network      },
    {"leases",      "Show all dhcp leases in a network",    show_dhcp_leases    },
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
    int help = 0;
    opterr = 0; /* silent options */
    const char *arg_str = "hvd:l:n:c:";
    size_t count = sizeof(dom_cmds)/sizeof(dom_cmds[0]);
    while ((opt = getopt_long(argc, argv, arg_str, options, NULL)) != -1) {
        switch (opt) {
            case 'c':
                for (size_t i = 0; i < count; ++i) {
                    if (strcmp(optarg, cfg_cmds[i].name) == 0) {
                        cfg_cmds[i].cmd(argc, argv);
                        break;
                    }
                }
            case 'd':
                for (size_t i = 0; i < count; ++i) {
                    if (strcmp(optarg, dom_cmds[i].name) == 0) {
                        exec_cmd(conn, optarg, argv[argc-1], dom_cmds[i].cmd);
                        break;
                    }
                }
                break;
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
            case 'h':
                help = 1;
                break;
            case '?':
                default:
                break;
        }
    }

    if (help) {
        if (optind < argc) {
            show_cmd_desc(conn, argv[optind]);
        } else {
            fprintf(stdout, "Usage: cobia [options...] <target>\n");
            fprintf(stdout, " -c, --configure <command>     Bootstrap or configure domains\n");
            fprintf(stdout, " -d, --domain <command>        Run a domain command\n");
            fprintf(stdout, " -l, --list <target>           List domains or networks\n");
            fprintf(stdout, " -n, --network <command>       Run a network command\n");
            fprintf(stdout, " -v, --version                 Show version\n");
            fprintf(stdout, " -h, --help                    Show usage\n");
        }

        return 0;
    }

    for (size_t i = 0; i < MAXSET; ++i) {
        free(settings[i].key);
        free(settings[i].val);
    }
    return 0;
}

void show_cmd_desc(virConnectPtr conn, const char *name)
{
    if (strcmp(name, "domain") == 0) {
        fprintf(stdout, "Usage: cobia -d <command> <target>\n");
        size_t i = 0;
        while (1) {
            fprintf(stdout, " %-10s %s\n", dom_cmds[i].name, dom_cmds[i].desc);
            if (strcmp(dom_cmds[i].name, "shell") == 0) break;
            ++i;
        }
    }

    if (strcmp(name, "network") == 0) {
        fprintf(stdout, "Usage: cobia -n <command> <target>\n");
        size_t i = 0;
        while (1) {
            fprintf(stdout, " %-10s %s\n", net_cmds[i].name, net_cmds[i].desc);
            if (strcmp(net_cmds[i].name, "leases") == 0) break;
            ++i;
        }
    }

    if (strcmp(name, "list") == 0) {
        fprintf(stdout, "Usage: cobia -l <command> <target>\n");
        size_t i = 0;
        while (1) {
            fprintf(stdout, " %-10s %s\n", l_cmds[i].name, l_cmds[i].desc);
            if (strcmp(l_cmds[i].name, "networks") == 0) break;
            ++i;
        }
    }

    if (strcmp(name, "configure") == 0) {
        fprintf(stdout, "Usage: cobia -c <command> <target>\n");
        size_t i = 0;
        while (1) {
            fprintf(stdout, " %-10s %s\n", cfg_cmds[i].name, cfg_cmds[i].desc);
            if (strcmp(cfg_cmds[i].name, "netconf") == 0) break;
            ++i;
        }
    }
}

