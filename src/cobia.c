#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cobia.h"
#include "xml.h"
#include "virt.h"

keyval_t settings[MAXSET]   = {0};

int TOTAL_VM_COUNT          = 0;
int TOTAL_NET_COUNT         = 0;

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
    const char * arg_str = "hvd:l:n:";
    while ((opt = getopt_long(argc, argv, arg_str, options, NULL)) != -1) {
        switch (opt) {
            case 'd':
                if (strcmp(optarg, "create") == 0)
                    exec_cmd(conn, optarg, argv[argc-1], create_domain);

                if (strcmp(optarg, "info") == 0)
                    exec_cmd(conn, optarg, argv[argc-1], show_vm_info);

                if (strcmp(optarg, "xml") == 0) // for debug purposes only
                    exec_cmd(conn, optarg, argv[argc-1], show_domain_xml);

                if (strcmp(optarg, "start") == 0)
                    exec_cmd(conn, optarg, argv[argc-1], start_domain);

                if (strcmp(optarg, "stop") == 0)
                    exec_cmd(conn, optarg, argv[argc-1], stop_domain);

                if (strcmp(optarg, "delete") == 0)
                    exec_cmd(conn, optarg, argv[argc-1], delete_domain);
                break;
            case 'h':
                // TODO
            case 'l':
                if (strcmp(optarg, "domains") == 0)
                    show_domains(conn);

                if (strcmp(optarg, "networks") == 0)
                    show_networks(conn);
                break;
            case 'n':
                if (strcmp(optarg, "create") == 0)
                    exec_cmd(conn, optarg, argv[argc-1], create_network);

                if (strcmp(optarg, "info") == 0)
                    exec_cmd(conn, optarg, argv[argc-1], show_network_info);

                if (strcmp(optarg, "start") == 0)
                    exec_cmd(conn, optarg, argv[argc-1], start_network);

                if (strcmp(optarg, "stop") == 0)
                    exec_cmd(conn, optarg, argv[argc-1], stop_network);

                if (strcmp(optarg, "delete") == 0)
                    exec_cmd(conn, optarg, argv[argc-1], delete_network);

                if (strcmp(optarg, "leases") == 0)
                    exec_cmd(conn, optarg, argv[argc-1], show_dhcp_leases);
                break;
            case 'v':
                // TODO:
                fprintf(stdout, "version: 0.0.0\n");
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
