/* MIT License. */

/* This file is taken from TF Protocol source code.
    It may have variations to adapt it to EPOCH Protocol. */

/*  programmer: luis miguel
    email: lmdelbahia@gmail.com  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <log.h>
#include <sig.h>
#include <dirent.h>
#include <net.h>

#define MINARGS 3

/* Endpoint auth directory. */ 
char endpts[PATH_MAX];
/* Port for socket listening. */
unsigned short port;

/* Function to become daemon */
static void mkdaemon(void);

int main(int argc, char **argv)
{
    if (argc < MINARGS) {
        wrlog(ELOGDARG, LGC_CRITICAL);
        exit(1);
    }
    strcpy(endpts, *(argv + 1));
    strcat(endpts, "/");
    port = atoi(*(argv + 2));
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGINT);
    pthread_sigmask(SIG_SETMASK, &mask, NULL);
    setsighandler(SIGINT, sigint);
#ifndef DEBUG
    if (argc >= MINARGS + 1)
        mkdaemon();
#endif
    wrlog(SLOGDRUNNING, LGC_INFO);
    startnet();
    return 0;
}

static void mkdaemon(void)
{
    pid_t pid = fork();
    if (pid)
        exit(0);
    else if (pid == -1) {
        wrlog(ELOGDFORK, LGC_CRITICAL);
        exit(-1);
    }
    setsid();
    int fdmax = sysconf(_SC_OPEN_MAX);
    for (; fdmax >= 0; fdmax--)
        close(fdmax);
    umask(0);
    int nulld = open("/dev/null", O_RDWR);
    dup(nulld);
    dup(nulld);
    dup(nulld);
    /* Redundancy call just in case something happened beetwen fork */
    pid = fork();
    if (pid)
        exit(0);
    else if (pid == -1) {
        wrlog(ELOGDFORK, LGC_CRITICAL);
        exit(-1);
    }
}
