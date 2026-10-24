#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <poll.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>

#include "console.h"
#include "virt.h"

static volatile int suppressed = 0;

void quiet_exit_console(void *opaque, virErrorPtr err) {
    if (!err) return;

    if (suppressed)
        if (err->code == VIR_ERR_OPERATION_FAILED && err->message &&
            strstr(err->message, "stream is closed"))
            return;

    virDefaultErrorFunc(err);
}

void *console_reader(void *op)
{
    int n;
    char buf[CONBUF_SIZE];
    Conctx *ctx = op;
    while(!ctx->done) {
        n = virStreamRecv(ctx->stream, buf, sizeof(buf));
        if (n <= 0) break;

        ssize_t offset = 0;
        while(offset < n) {
            ssize_t w = write(STDOUT_FILENO, buf + offset, n - offset);

            if (w < 0) {
                ctx->done = 1;
                goto done;
            }

            offset += w;
        }
    }

    done:
        ctx->done = 1;
        return NULL;
}

void start_console(virConnectPtr conn, const char* name)
{
    pthread_t ti;
    int raw_set = 0;
    bool thread_started = false;
    struct termios saved, raw;
    Conctx ctx = { .stream = NULL, .done = 0 };

    virDomainPtr dom = get_domain(conn, name);
    if (dom == NULL) goto clean; 

    virSetErrorFunc(NULL, quiet_exit_console);
    ctx.stream = virStreamNew(conn, 0);
    if (!ctx.stream) goto clean;

    int console = virDomainOpenConsole(dom, NULL, ctx.stream, VIR_DOMAIN_CONSOLE_SAFE);
    if (console < 0) goto clean;

    int rc = pthread_create(&ti, NULL, console_reader, &ctx);
    if (rc != 0) goto clean;
    thread_started = true;

    int sc = tcgetattr(STDIN_FILENO, &saved);
    if (sc == 0) {
        raw = saved;
        cfmakeraw(&raw);
        raw_set = (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0);
    }
    
    while (1) {
        if (ctx.done) break;
        struct pollfd pf = { .fd = STDIN_FILENO, .events = POLLIN };

        int pr = poll(&pf, 1, 200);
        if (pr == 0) continue;

        char ch;
        ssize_t n = read(STDIN_FILENO, &ch, 1);

        /* quit when stdin is closed or Ctrl-] is pressed */
        if (n <= 0 || ch == 0x1d) {
            goto done;
        }

        ssize_t offset = 0;
        while (offset < n) {
            ssize_t sent = virStreamSend(ctx.stream, &ch+offset, n-offset);
            if (sent < 0) {
                goto done;
            }

            offset += sent;
        }
    }

    done:
        ctx.done = 1;
        suppressed = 1;
        if (raw_set) tcsetattr(STDIN_FILENO, TCSANOW, &saved);
        fputc('\n', stdout);
        if (ctx.stream) virStreamFinish(ctx.stream);
        if (thread_started) {
            pthread_join(ti, NULL);
            thread_started = false;
        }
        suppressed = 0;

    clean:
        if (thread_started) {
            ctx.done = 1;
            suppressed = 1;
            if (ctx.stream) virStreamAbort(ctx.stream);
            pthread_join(ti, NULL);
            suppressed = 0;
            thread_started = false;
        }
        if (raw_set) tcsetattr(STDIN_FILENO, TCSANOW, &saved);
        if (ctx.stream) virStreamFree(ctx.stream);
        if (dom) virDomainFree(dom);
}
