#include <EXTERN.h>
#include <perl.h>

static PerlInterpreter *my_perl;
static char *cobia_cfg;

static void xs_init(pTHX);

EXTERN_C void boot_DynaLoader(pTHX_ CV* cv);

int bootstrap_domain(int argc, char **argv);

int configure_network(int argc, char **argv);
