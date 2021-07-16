/* MIT License. */

/* This file is taken from TF Protocol source code.
    It may have variations to adapt it to EPOCH Protocol. */

/*  programmer: luis miguel
    email: lmdelbahia@gmail.com  */

#ifndef SIG_H
#define SIG_H

#include <signal.h>

/* Set signal handler for the specified "signo" signal */
void setsighandler(int signo, void (*handler)(int));
/* Signal handler for SIGINT signal. */
void sigint(int signo);

#endif 
