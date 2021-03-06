/* MIT License. */

/*  programmer: luis miguel
    email: lmdelbahia@gmail.com  */

#ifndef EPOCH_H
#define EPOCH_H

#include <inttypes.h>

/* Default communication buffer. */
#define DEFCOMBUF 65536
/* Max ipv4 length. */
#define EPOCH_IPV4 16
/* Max ipv6 length. */
#define EPOCH_IPV6 30
/* Session key max length in bytes. Must be less than the RSA key length
    modulus minus used padding. In this case is 2048 / 8 - 42 = 214 bytes. */
#define EPOCH_KEYMAX 2048 / 8 - 42
/* Endpoint executed successfully. */
#define EPOCH_SUCCESS 0
/* Error trying connecting to the server. */
#define EPOCH_ECONNECT -1
/* Error sending data. Connection ended. */
#define EPOCH_ESEND -2
/* Error receiving data. Connection ended. */
#define EPOCH_ERECV -3
/* Buffer null pointer. */
#define EPOCH_EBUFNULL -4
/* Error in endpoint authentication process. */
#define EPOCH_EENDPTAUTH -5
/* Error initializing multi-thread environment. */
#define EPOCH_EMTINIT -6
/* Error calling a function with wrong communication semantics. */
#define EPOCH_EEXFLAG -7
/* Signal sent out of context. */
#define EPOCH_ESIGCONXT -8
/* Invalid minimum session key length. */
#define EPOCH_EKEYMIN -9

enum epoch_mode { 
    SINGLE_THREAD_SNDFIRST, SINGLE_THREAD_RCVFIRST, MULTI_THREAD, DETACHED
};

/* Obscure type which contains the network connection. */
struct netconn;

struct epoch_s {
    /* IPv4 server address. */
    char ipv4[EPOCH_IPV4];
    /* IPv6 server address. */
    char ipv6[EPOCH_IPV6];
    /* Server listening port. */
    unsigned short port;
    /* Session key for encryption ops. */
    char key[EPOCH_KEYMAX];
    /* Session key length. */
    unsigned short keylen;
    /* Server public key. */
    char *pubkey;
    /* Timeout -in seconds- for network read. */
    int rdtmout;
    /* Timeout -in seconds- for host connecting. */
    int cntmout;
    /* Obscure network state. */
    struct netconn *nc;
};

/* Callback function prototypes for sending and recving data. */
typedef void (*callback_snd)(void *sndbuf, int64_t *sndlen);
typedef void (*callback_rcv)(void *rcvbuf, int64_t rcvlen);

/* Run synchronously remote endpoint. */
int epoch_endpoint(struct epoch_s *e, const char *endpt, const char *auth, 
     enum epoch_mode mode, int64_t bufsz, callback_snd snd_callback, 
     callback_rcv rcv_callback, const char *wd, int *es);
/* Run Endpoint in expert mode. */
int epoch_endpoint_ex(struct epoch_s *e, const char *endpt, const char *auth, 
    int64_t bufsz, const char *wd);
/* Run Endpoint in detached mode. */
int epoch_endpoint_bk(struct epoch_s *e, const char *endpt, const char *auth, 
    const char *wd);
/* Endpoint expert mode sender function. */
int epoch_send_ex(struct epoch_s *e, void *buf, int64_t len);
/* Endpoint expert mode receiver function. */
int epoch_recv_ex(struct epoch_s *e, void *buf, int64_t *len);
/* Ends Endpoint expert mode communication. */
int epoch_fin_ex(struct epoch_s *e, int *es);
/* Resolv server from host name. On failure return non-zero. */
int resolvhn(const char *host, char *ip, int v6, int timeout);
/* Send signal to the endpoint. */
int epoch_signal(struct epoch_s *e, int signo);

#endif
