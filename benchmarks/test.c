#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>

ucontext_t ctx1, ctx2;

void foo() {
    while (1) {
        printf("foo\n");
        sleep(1);
        swapcontext(&ctx1, &ctx2);
    }
}

void bar() {
    while (1) {
        printf("bar\n");
        sleep(1);
        swapcontext(&ctx2, &ctx1);
    }
}

void timer_handler(int sig) {
    swapcontext(&ctx1, &ctx2);
}

int main() {
    // Set up the timer
    struct sigaction sa;
    struct itimerval timer;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &timer_handler;
    sigaction(SIGPROF, &sa, NULL);

    timer.it_value.tv_sec = 1;
    timer.it_value.tv_usec = 0;
    timer.it_interval = timer.it_value;
    setitimer(ITIMER_PROF, &timer, NULL);

    // Set up the contexts
    getcontext(&ctx1);
    ctx1.uc_stack.ss_sp = malloc(8192);
    ctx1.uc_stack.ss_size = 8192;
    ctx1.uc_link = &ctx2;
    makecontext(&ctx1, foo, 0);

    getcontext(&ctx2);
    ctx2.uc_stack.ss_sp = malloc(8192);
    ctx2.uc_stack.ss_size = 8192;
    ctx2.uc_link = &ctx1;
    makecontext(&ctx2, bar, 0);

    // Start the program by jumping to the first context
    setcontext(&ctx1);

    return 0;
}
