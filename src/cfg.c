#define _POSIX_C_SOURCE 200112L
#include "cfg.h"

static PerlInterpreter *my_perl = NULL;
static char *cobia_cfg = "/usr/local/bin/cobia-cfg";

EXTERN_C void xs_init(pTHX)
{
    char *file = __FILE__;
    newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);
}

int configure_network(char *iface)
{
    int perl_argc = 5;
    char *perl_args[] = { "", cobia_cfg, "--netconf", "--iface", iface, NULL };
    PERL_SYS_INIT(&perl_argc, NULL);
    my_perl = perl_alloc();
    if (my_perl == NULL) {
        PERL_SYS_TERM();
        return 1;
    }

    perl_construct(my_perl);
    PL_exit_flags |= PERL_EXIT_DESTRUCT_END;

    perl_parse(my_perl, xs_init, perl_argc, perl_args, NULL);
    perl_run(my_perl);

    perl_destruct(my_perl);
    perl_free(my_perl);
    PERL_SYS_TERM();
    return 0;
}
