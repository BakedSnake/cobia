#include <libvirt/virterror.h>

#define CONBUF_SIZE 4096

typedef struct {
    virStreamPtr stream;
    volatile int done;
} Conctx;

/*
 * quiet_exit_console()
 *
 * Closes   the    console     stream     silently.
 * When the stream is closed it usually  throws  an
 * error,  this  functions   silences  the  closing
 * stream errors when we want.  Which  is  when  we
 * manually close the stream. ( exit the console ).
 *
 * It is  meant  to  be  used  as  an  argument  of
 * virSetErrorFunc().
 *
 */

void quiet_exit_console(void *opaque, virErrorPtr err);

/*
 * start_console()
 *
 * Starts  a  serial  console  connection   to   a
 * specified domain. ( by name ).
 *
 */

void start_console(virConnectPtr conn, const char *name);

/*
 * console_reader()
 *
 * Reads the console buffer.
 * This function is  meant  to  be  provided  to  a
 * p_thread, so we can poll and  read  the  console
 * buffer.
 *
 */

void *console_reader(void *op);
