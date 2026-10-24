#include <libvirt/virterror.h>

#define CONBUF_SIZE 4096

typedef struct {
    virStreamPtr stream;
    volatile int done;
} Conctx;

void quiet_exit_console(void *opaque, virErrorPtr err);

void start_console(virConnectPtr conn, const char *name);

void *console_reader(void *op);
