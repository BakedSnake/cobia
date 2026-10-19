#include <stdio.h>
#include <string.h>

#include "cobia.h"
#include "xml.h"
#include "virt.h"

int TOTAL_VM_COUNT      = 0;
int TOTAL_NET_COUNT     = 0;

char *SRC               = NULL;
char *TRGT              = NULL;

char *CD                = NULL;
char *DISK              = NULL;
char *DISKSIZE          = NULL;
char *EFI               = NULL;

Domain                  DOMAIN;
Network                 NETWORK;
Feature                 SECBOOT;
Loader                  LOADER;
Nvram                   NVRAM;
Firmware                FIRMWARE;
Boot                    BOOT;
Machine                 MACHINE;
Os                      OS;
Memory                  MEMORY;
Vcpu                    VCPU;
Cpu                     CPU;
Sound                   SOUND;
Interface               INTERFACE;
Graphics                GRAPHICS;
Video                   VIDEO;
Channel                 CHANNEL;

Controller*             CONTROLLERS;
Disk*                   DISKS;

key_map_t settings[20]  = {
    { "domain.boot",                        &BOOT.device                },
    { "domain.cpu",                         &VCPU.cores                 },
    { "domain.devices.cd",                  &CD                         },
    { "domain.devices.disk.size",           &DISKSIZE                   },
    { "domain.devices.disk.source",         &DISK                       },
    { "domain.devices.graphics",            &GRAPHICS.type              },
    { "domain.devices.net",                 &INTERFACE.source_network   },
    { "domain.devices.video.driver",        &VIDEO.driver_name          },
    { "domain.devices.video.model",         &VIDEO.model_type           },
    { "domain.model",                       &INTERFACE.model_type       },
    { "domain.name",                        &DOMAIN.name                },
    { "domain.os.arch",                     &OS.arch                    },
    { "domain.os.bootdev",                  &BOOT.device                },
    { "domain.os.efi",                      &EFI                        },
    { "domain.os.features.secure_boot",     &SECBOOT.enabled            },
    { "domain.os.machine",                  &MACHINE.kind               },
    { "domain.ram",                         &MEMORY.size                },
    { "domain.ssh_key",                     &DOMAIN.key                 },
    { "domain.user",                        &DOMAIN.user                },
    { "domain.vm_dir",                      &DOMAIN.dir                 },
};

key_map_t net_settings[7] = {
    { "network.name",                   &NETWORK.name},
    { "network.forward",                &NETWORK.forward},
    { "network.bridge",                 &NETWORK.bridge},
    { "network.ip.address",             &NETWORK.ip},
    { "network.ip.netmask",             &NETWORK.netmask},
    { "network.ip.dhcp.range.start",    &NETWORK.range_start},
    { "network.ip.dhcp.range.end",      &NETWORK.range_end},
};

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
                        get_nvram_path(TRGT);
                        fprintf(stdout, "%s\n", NVRAM.path);
                        delete_domain(conn, TRGT);
                        TRGT = NULL;
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

    return 0;
}
