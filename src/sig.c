/* MIT License. */

/* This file is taken from TF Protocol source code.
    It may have variations to adapt it to EPOCH Protocol. */

/*  programmer: luis miguel
    email: lmdelbahia@gmail.com  */

#include <pthread.h>
#include <sig.h>
#include <stdio.h>
#include <stdlib.h>
#include <log.h>
#include <errno.h>
#include <net.h>

void setsighandler(int signo, void (*handler)(int))
{
    struct sigaction sa;
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(signo, &sa, NULL);
}

void sigint(int signo)
{
    endnet();
    wrlog(SLOGSIGINT, LGC_INFO);
    exit(0);
}
