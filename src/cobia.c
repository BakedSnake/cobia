#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cobia.h"
#include "xml.h"
#include "virt.h"

int TOTAL_VM_COUNT          = 0;
int TOTAL_NET_COUNT         = 0;
char *SRC                   = NULL;
char *TRGT                  = NULL;

keyval_t settings[MAXSET]   = {0};

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
    const char * arg_str = "hvd:l:n:s:t:";
    while ((opt = getopt_long(argc, argv, arg_str, options, NULL)) != -1) {
        switch (opt) {
            case 'd':
                if (strcmp(optarg, "create") == 0) {
                    if (SRC != NULL) {
                        fprintf(stdout, "%s:\n", SRC);
                        create_domain(conn, SRC);
                        SRC = NULL;
                    }
                }
                if (strcmp(optarg, "info") == 0) {
                    if (TRGT != NULL) {
                        fprintf(stdout, "%s:\n", TRGT);
                        show_vm_info(TRGT, conn);
                        TRGT = NULL;
                    }
                }
                if (strcmp(optarg, "xml") == 0) {
                    if (TRGT != NULL) {
                        fprintf(stdout, "%s\n", get_domain_xml(conn, TRGT));
                        TRGT = NULL;
                    }
                }
                if (strcmp(optarg, "start") == 0) {
                    if (TRGT != NULL) {
                        start_domain(conn, TRGT);
                        TRGT = NULL;
                    }
                }
                if (strcmp(optarg, "stop") == 0) {
                    if (TRGT != NULL) {
                        stop_domain(conn, TRGT);
                        TRGT = NULL;
                    }
                }
                if (strcmp(optarg, "delete") == 0) {
                    if (TRGT != NULL) {
                        char* nvram = get_nvram_path(TRGT);
                        fprintf(stdout, "%s\n", nvram);
                        delete_domain(conn, TRGT);
                        TRGT = NULL;
                        free(nvram);
                    }
                }
                break;
            case 'h':
                // TODO
            case 'l':
                if (strcmp(optarg, "domains") == 0) {
                    show_domains(conn);
                }
                if (strcmp(optarg, "networks") == 0) {
                    show_networks(conn);
                }
                break;
            case 'n':
                if (strcmp(optarg, "create") == 0) {
                    if (SRC != NULL) {
                        create_network(conn, SRC);
                        SRC = NULL;
                    } else {
                        fprintf(stderr, "Source file not specified\n");
                        return 1;
                    }
                }
                if (strcmp(optarg, "info") == 0) {
                    if (TRGT != NULL) {
                        show_network_info(conn, TRGT);
                        TRGT = NULL;
                    } else {
                        fprintf(stderr, "Target not specified\n");
                        return 1;
                    }
                }
                if (strcmp(optarg, "delete") == 0) {
                    if (TRGT != NULL) {
                        delete_network(conn, TRGT);
                        TRGT = NULL;
                    } else {
                        fprintf(stderr, "Target not specified\n");
                        return 1;
                    }
                }
                break;
            case 's':
                SRC = optarg;
                break;
            case 't':
                TRGT = optarg;
                break;
            case 'v':
                // TODO
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
