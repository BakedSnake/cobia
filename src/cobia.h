#include <getopt.h>
#include <libvirt/libvirt.h>

#define KVSIZE      256
#define MAXDEPTH    8
#define MAXKEYS     256
#define MAXDISK     64
#define MAXSET      64

extern int          TOTAL_VM_COUNT;
extern int          TOTAL_NET_COUNT;
extern char*        SRC;

typedef struct {
    char *key;
    char *val;
} keyval_t;

extern keyval_t disk_settings[MAXDISK];
extern keyval_t settings[MAXSET];
extern keyval_t net_settings[7];


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

char* get_val(keyval_t pair[MAXSET], const char* keyname);

int argHandle(virConnectPtr conn, int argc, char* argv[]);
