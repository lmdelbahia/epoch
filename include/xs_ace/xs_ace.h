/* MIT License. */

/* This file is taken from TF Protocol source code.
    It may have variations to adapt it to EPOCH Protocol. */

/*  programmer: luis miguel
    email: lmdelbahia@gmail.com  */

#ifndef XS_ACE_H
#define XS_ACE_H

enum epoch_mode { 
    SINGLE_THREAD_SNDFIRST, SINGLE_THREAD_RCVFIRST, MULTI_THREAD
};

/* Enters the ACE subsystem. */
void xs_ace(enum epoch_mode mode, char *endpt);

#endif
