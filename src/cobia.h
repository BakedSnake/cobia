#include <getopt.h>
#include <libvirt/libvirt.h>

#define KVSIZE 256
#define MAXDEPTH 8
#define MAXKEYS 256

extern int TOTAL_VM_COUNT;
extern int TOTAL_NET_COUNT;

extern char *SRC;

extern char *CD;
extern char *EFI;
extern char *DISK;
extern char *DISKSIZE;

typedef struct {
    const char *key;
    char **dest;
} key_map_t;

typedef struct Domain {
    char *name;
    char *user;
    char *key;
    char *dir;
} Domain;

typedef struct Network {
    char *name;
    char *forward;
    char *bridge;
    char *ip;
    char *netmask;
    char *range_start;
    char *range_end;
} Network;

typedef struct Feature {
    char *enabled;
    char *name;
} Feature;

typedef struct Loader {
    char *readonly;
    char *type;
    char *format;
    char *path;
} Loader;

typedef struct Nvram {
    char *nv_template;
    char *template_format;
    char *format;
    char *path;
} Nvram;

typedef struct Firmware {
    Feature *features;
    Loader loader;
    Nvram nvram;
} Firmware;

typedef struct Boot {
    char *device;
} Boot;

typedef struct Machine {
    char *arch;
    char *kind;
    char *value;
} Machine;

typedef struct Os {
    char *os_firmware;
    char* arch;
} Os;

typedef struct Memory {
    char *unit;
    char *size;
} Memory;

typedef struct Vcpu {
    char *placement;
    char *cores;
} Vcpu;

typedef struct Cpu {
    char *mode;
    char *check;
    char *migratable;
} Cpu;

typedef struct Features {
    char *acpi;
    char *apic;
    char *vmport_state;
    char *smm_state;
} Features;

typedef struct Controller {
    char *type;
    char *index;
    char *model;
} Controller;

typedef struct Disk {
    char *type;
    char *device;
    char *driver_name;
    char *driver_type;
    char *source_file;
    char *target_dev;
    char *target_bus;
} Disk;

typedef struct Sound {
    char *model;
} Sound;

typedef struct Interface {
    char *type;
    char *source_network;
    char *model_type;
    /* Network interface ( there may be missing fields
     * on other kinds of interfaces ) */
} Interface;

typedef struct Graphics {
    char *type;
    char *autoport;
    char *listen_type;
} Graphics;

typedef struct Video {
    char *model_type;
    char *model_heads;
    char *model_primary;
    char *driver_name;
} Video;

typedef struct Channel {
    char *type;
    char *target_type;
    char *target_name;
    char *alias_name;
} Channel;

extern key_map_t settings[20];
extern key_map_t net_settings[7];

extern Domain       DOMAIN;
extern Network      NETWORK;
extern Feature      SECBOOT;
extern Loader       LOADER;
extern Nvram        NVRAM;
extern Firmware     FIRMWARE;
extern Boot         BOOT;
extern Machine      MACHINE;
extern Os           OS;
extern Memory       MEMORY;
extern Vcpu         VCPU;
extern Cpu          CPU;
extern Sound        SOUND;
extern Interface    INTERFACE;
extern Graphics     GRAPHICS;
extern Video        VIDEO;
extern Channel      CHANNEL;

extern Controller*  CONTROLLERS;
extern Disk*        DISKS;

static struct option options[] = {
  {"help",              no_argument,            0, 'h'},
  {"version",           no_argument,            0, 'v'},
  {"domain",            required_argument,      0, 'd'},
  {"list",              required_argument,      0, 'l'},
  {"network",           required_argument,      0, 'n'},
  {"source",            required_argument,      0, 's'},
  {"target",            required_argument,      0, 't'},
  {0,                   0,                      0,  0 }
};

int argHandle(virConnectPtr conn, int argc, char* argv[]);
