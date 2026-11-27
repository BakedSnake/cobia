#include <getopt.h>
#include <libvirt/libvirt.h>

#define PATHSIZE    1024
#define KVSIZE      256
#define MAXDEPTH    8
#define MAXKEYS     256
#define MAXDISK     64
#define MAXSET      256
#define D_ATTSIZE   32

typedef struct {
    char *name;
    char *desc;
    void (*cmd)(virConnectPtr c, const char *t);
} Command;

typedef struct {
    char *name;
    char *desc;
    void (*cmd)(virConnectPtr c);
} Command_Single;

typedef struct {
    char *name;
    char *desc;
    int (*cmd)(int argc, char **argv);
} Command_Config;

typedef struct {
    char *key;
    char *val;
} keyval_t;

extern keyval_t     settings[MAXSET];

extern int          TOTAL_VM_COUNT;
extern int          TOTAL_NET_COUNT;
extern int          TOTAL_KEY_COUNT;

static struct option options[] = {
  {"help",          no_argument,            0, 'h'},
  {"version",       no_argument,            0, 'v'},
  {"domain",        required_argument,      0, 'd'},
  {"list",          required_argument,      0, 'l'},
  {"network",       required_argument,      0, 'n'},
  {"configure",     required_argument,      0, 'c'},
  {0,               0,                      0,  0 }
};

/*
 * get_val()
 *
 * Returns  the  value  of  a  key   value    pair.
 * Goes throught the key pairs and returns only the
 * value of the provided key.
 *
 */

char* get_val(keyval_t pair[MAXSET], const char *keyname);

/*
 * exec_cmd()
 *
 * Executes  the  main  commands  of  the  program.
 *  Takes  a  function  pointer  to  execute.  This
 * functions is called  by  the  argument  handler,
 * which  provides  the  target  and  the  function
 * pointer.
 *
 */

void exec_cmd(virConnectPtr conn, const char *arg,
        const char *target, void (*cmdf)(virConnectPtr c, const char *t));

/*
 * argHandle()
 *
 * Main argument handler function.
 * Takes care of checking  the  arguments  provided
 * and calls the correct command.
 *
 */

int argHandle(virConnectPtr conn, int argc, char *argv[]);

/*
 * show_cmd_desc()
 *
 * Returns   0   if   there   are    no      erros.
 * Prints   command   description   to      stdout.
 *
 */

void show_cmd_desc(virConnectPtr conn, const char *name);
