/* MIT License. */

/* This file is taken from TF Protocol source code.
    It may have variations to adapt it to EPOCH Protocol. */

/*  programmer: luis miguel
    email: lmdelbahia@gmail.com  */

#ifndef TFPROTO_H
#define TFPROTO_H

#include <netinet/in.h>
#include <inttypes.h>
//#include <crypto.h>

/* Default communication buffer length. */
#define COMMBUFLEN 65536
/* Deprecated. */
/* Number of decimal digits of the size of communication buffer. */
#define COMMBUFDIG 3
#undef COMMBUFDIG
/* Daemonize flag. */
#define MKDAEMON 1

/* Contains needed members for communicating potentially used from severals
    modules in the program */
struct comm {
    int sock;
    struct sockaddr_in6 addr;
    socklen_t addrsz;
    char *rxbuf;
    char *txbuf;
    int64_t bufsz;
} extern comm; 

/* Cryptography structure containing crypt and decrypt functions. */
extern struct crypto cryp;

/* Initialize internal data and run mainloop function */
void begincomm(int sock, struct sockaddr_in6 *rmaddr, socklen_t *rmaddrsz);
/* Do some clean-up before terminate process */
void cleanup(void);
/* This function is equivalent to readbuf, except it does not checks for a
    received header for message boundaires. */
int64_t readbuf_ex(char *buf, int64_t len);
/* This function is equivalent to writebuf, except it does not sends a header
    for message boundaires. */
int64_t writebuf_ex(char *buf, int64_t len);
/* End communication and do clean-up terminating the process */
void endcomm(int rs);

#endif
