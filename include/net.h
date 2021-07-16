/* MIT License. */

/* This file is taken from TF Protocol source code.
    It may have variations to adapt it to EPOCH Protocol. */

/*  programmer: luis miguel
    email: lmdelbahia@gmail.com  */

#ifndef NET_H
#define NET_H

/* Start network listening and client dispatcher into forked process */
void startnet(void);
/* Close listen server socket */
void endnet(void);

#endif
