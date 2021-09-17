/* MIT License. */

/*  programmer: luis miguel
    email: lmdelbahia@gmail.com  */

#include <epoch.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <openssl/ssl.h>
#include <time.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/time.h>
#include <signal.h>

#define swapbo32(value) value = (value >> 24 & 0xFF) | (value >> 16 & 0xFF) << \
    8 | (value >> 8 & 0xFF) << 16 | (value & 0xFF) << 24;
#define swapbo64(value) value = (value >> 56 & 0xFF) | (value >> 48 & 0xFF) << \
    8 | (value >> 40 & 0xFF) << 16 | (value >> 32 & 0xFF) << 24 | (value >> 24 \
    & 0xFF) << 32 | (value >> 16 & 0xFF) << 40 | (value >> 8 & 0xFF) << 48 | \
    (value & 0xFF) << 56
/* RSA key lenght modulus. */
#define RSA_KEYLEN 2048 / 8

/* The posible states of encrypt system. */
enum cryptst { CRYPT_OFF, CRYPT_ON };

struct netconn {
    int sock;
    struct sockaddr_in addr;
    struct sockaddr_in6 addr6;
    enum cryptst cst;
    char *rxbuf;
    char *txbuf;
    int64_t bufsz;
    pthread_barrier_t barrier;
    callback_rcv rcv;
    callback_snd snd;
    int rdth_rc;
    int wrth_rc;
    int sndflag;
    int rcvflag;
    char stdline[LINE_MAX];
};

struct rsvparams {
    const char *host;
    char ip[EPOCH_IPV6];
    int v6;
    pthread_cond_t *cond;
    pthread_mutex_t *mut;
    volatile int done;
};

static int64_t writebuf_ex(struct epoch_s *e, char *buf, int64_t len);
static int64_t readbuf_ex(struct epoch_s *e, char *buf, int64_t len);
static void encrypt(struct epoch_s *e, char *data, int len);
static int procdata(struct epoch_s *e, callback_snd snd_callback, 
    callback_rcv rcv_callback, enum epoch_mode mode, int *es);
static int isbigendian(void);
static int sendskey(struct epoch_s *e);
static void freecomm(struct epoch_s *e);
static int connecttm(struct epoch_s *e);
static void *thresolv(void *pp);
static int sndloop(struct epoch_s *e, callback_snd snd_callback);
static int rcvloop(struct epoch_s *e, callback_rcv rcv_callback);
static void conbuilder(char *con, const char *endpt, const char *auth, 
    enum epoch_mode mode, const char *wd, int64_t bufsz);
static void blksigpipe(int blk, sigset_t *oldmask);
static void *rdthread(void *pp);
static void *wrthread(void *pp);
static int recves(struct epoch_s *e, int *es);
static int epoch_epcore(struct epoch_s *e, const char *endpt, const char *auth, 
     enum epoch_mode mode, int64_t bufsz, const char *wd, int ex);

int epoch_endpoint(struct epoch_s *e, const char *endpt, const char *auth, 
     enum epoch_mode mode, int64_t bufsz, callback_snd snd_callback, 
     callback_rcv rcv_callback, const char *wd, int *es)
{
    int rc = epoch_epcore(e, endpt, auth, mode, bufsz, wd, 0); 
    if (rc)
        return rc;
    rc = procdata(e, snd_callback, rcv_callback, mode, es);
    freecomm(e);
    return rc;
}

int epoch_endpoint_ex(struct epoch_s *e, const char *endpt, const char *auth, 
    int64_t bufsz, const char *wd)
{
    return epoch_epcore(e, endpt, auth, MULTI_THREAD, bufsz, wd, 1); 
}

int epoch_endpoint_bk(struct epoch_s *e, const char *endpt, const char *auth, 
    const char *wd)
{
    int rc = epoch_epcore(e, endpt, auth, DETACHED, DEFCOMBUF, wd, 1);
    if (rc)
        return rc;
    freecomm(e);
    return rc;
}

static int64_t writebuf_ex(struct epoch_s *e, char *buf, int64_t len)
{
    encrypt(e, buf, len);
    int64_t wb = 0;
    int64_t written = 0;
    sigset_t oldset;
    blksigpipe(1, &oldset);
    while (len > 0) {
        do
            wb = write(e->nc->sock, buf + written, len);
        while (wb == -1 && errno == EINTR);
        if (wb == -1)
            return -1;
        len -= wb;
        written += wb;
    }
    blksigpipe(0, &oldset);
    return written;
}

static int64_t readbuf_ex(struct epoch_s *e, char *buf, int64_t len)
{
    int64_t rb = 0;
    int64_t readed = 0;
    while (len > 0) {
        do
            rb = read(e->nc->sock, buf + readed, len);
        while (rb == -1 && errno == EINTR);
        if (rb == 0 || rb == -1)
            return -1;
        len -= rb;
        readed += rb;
    }
    encrypt(e, buf, readed);
    return readed;
}

static void encrypt(struct epoch_s *e, char *data, int len)
{
    if (e->nc->cst == CRYPT_ON) {
        int c = 0, keyc = 0;
        for (; c < len; c++) {
            if (keyc == e->keylen)
                keyc = 0;
            *(data + c) ^= e->key[keyc++];
        }
    }
}

static int procdata(struct epoch_s *e, callback_snd snd_callback, 
    callback_rcv rcv_callback, enum epoch_mode mode, int *es)
{
    int rc;
    int64_t hdr = 0;
    if (mode == SINGLE_THREAD_SNDFIRST) {
        if (sndloop(e, snd_callback) == -1)
            return EPOCH_ESEND;
        if (rcvloop(e, rcv_callback) == -1)
            return EPOCH_ERECV;
        if (recves(e, es) == -1)
            return EPOCH_ERECV;
        /* Finishing the two-way handsaheke. */
        writebuf_ex(e, (char *) &hdr, sizeof hdr);
    } else if (mode == SINGLE_THREAD_RCVFIRST) {
        if (rcvloop(e, rcv_callback) == -1)
            return EPOCH_ERECV;
        if (sndloop(e, snd_callback) == -1)
            return EPOCH_ESEND;
        if (recves(e, es) == -1)
            return EPOCH_ERECV;
        /* Finishing the two-way handsaheke. */
        readbuf_ex(e, (char *) &hdr, sizeof hdr);
    } else {
        pthread_t rdth, wrth;
        e->nc->snd = snd_callback;
        e->nc->rcv = rcv_callback;
        int rs = pthread_barrier_init(&e->nc->barrier, NULL, 3);
        if (rs)
            return EPOCH_EMTINIT;
        rs = pthread_create(&wrth, NULL, wrthread, e);
        if (rs) {
            pthread_barrier_destroy(&e->nc->barrier);
            return EPOCH_EMTINIT;
        }
        rs = pthread_create(&rdth, NULL, rdthread, e);
        if (rs) {
            pthread_barrier_destroy(&e->nc->barrier);
            return EPOCH_EMTINIT;
        }
        pthread_barrier_wait(&e->nc->barrier);
        pthread_barrier_destroy(&e->nc->barrier);
        if (e->nc->rdth_rc)
            return EPOCH_ERECV;
        if (e->nc->wrth_rc)
            return EPOCH_ESEND;
        if (recves(e, es) == -1)
            return EPOCH_ERECV;
        /* Finishing the two-way handsaheke. Reverse order at server side. */
        writebuf_ex(e, (char *) &hdr, sizeof hdr);
        hdr = 0;
        readbuf_ex(e, (char *) &hdr, sizeof hdr);
    }
    return EPOCH_SUCCESS;
}

static int isbigendian(void)
{
    int value = 1;
    char *pt = (char *) &value;
    if (*pt == 1)
        return 0;
    else
        return 1;
}

static int sendskey(struct epoch_s *e)
{
    char enkey[RSA_KEYLEN];
    BIO *keybio = NULL;
    keybio = BIO_new_mem_buf(e->pubkey, -1);
    RSA *rsa = NULL;
    rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);
    int pad = RSA_PKCS1_OAEP_PADDING;
    int enbyt = RSA_public_encrypt(e->keylen, e->key, enkey, rsa, pad);
    RSA_free(rsa);
    BIO_free(keybio);
    int64_t hdr = enbyt;
    if (!isbigendian())
        swapbo64(hdr);
    if (writebuf_ex(e, (char *) &hdr, sizeof hdr) == -1)
        return EPOCH_ESEND;
    if (writebuf_ex(e, enkey, enbyt) == -1)
        return EPOCH_ESEND;
    return 0;
}

static void freecomm(struct epoch_s *e)
{
    if (e->nc) {
        close(e->nc->sock);
        if (e->nc->rxbuf && e->nc->rxbuf != e->nc->stdline)
            free(e->nc->rxbuf);
        if (e->nc->txbuf)
            free(e->nc->txbuf); 
        free(e->nc);
        e->nc = NULL;
    }
}

static int connecttm(struct epoch_s *e)
{
    int flags = fcntl(e->nc->sock, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(e->nc->sock, F_SETFL, flags);
    fd_set set;
    FD_ZERO(&set);
    FD_SET(e->nc->sock, &set);
    struct timeval timeout;
    timeout.tv_sec = 4;
    timeout.tv_usec = 0;
    if (connect(e->nc->sock, (struct sockaddr *) &e->nc->addr, 
        sizeof e->nc->addr)) {
        int rc = select(FD_SETSIZE, NULL, &set, NULL, &timeout);
        if (!rc)
            return EPOCH_ECONNECT;
        else if (FD_ISSET(e->nc->sock, &set)) {
            if (getpeername(e->nc->sock, NULL, NULL) == -1 && errno == ENOTCONN)
                return EPOCH_ECONNECT;
            flags &= ~O_NONBLOCK;
            fcntl(e->nc->sock, F_SETFL, flags);
        }
    }
    return 0;
}

int resolvhn(const char *host, char *ip, int v6, int timeout)
{
    pthread_cond_t cond;
    pthread_mutex_t mut;
    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&mut, NULL);
    struct rsvparams params;
    params.host = host;
    params.v6 = v6;
    params.mut = &mut;
    params.cond = &cond;
    params.done = 0;
    pthread_mutex_lock(&mut);
    pthread_t th;
    int rc = pthread_create(&th, NULL, thresolv, &params);
    if (timeout > 0) {
        struct timespec ts;
        struct timeval now;
        gettimeofday(&now, NULL);
        ts.tv_sec = now.tv_sec + timeout;
        ts.tv_nsec = now.tv_usec * 1000;
        pthread_cond_timedwait(&cond, &mut, &ts);
    } else
        pthread_cond_wait(&cond, &mut);
    pthread_cancel(th);
    pthread_mutex_destroy(&mut);
    pthread_cond_destroy(&cond);
    if (params.done) {
        strcpy(ip, params.ip);
        return 0;
    }
    return -1;
}

static void *thresolv(void *pp)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    struct rsvparams *rsv = pp;
    struct addrinfo hints;
    struct addrinfo *rs;
    memset(&hints, 0, sizeof hints);
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    if (rsv->v6) {
        hints.ai_family = AF_INET6;
        if (getaddrinfo(rsv->host, NULL, &hints, &rs) != 0) {
            pthread_mutex_lock(rsv->mut);
            pthread_cond_signal(rsv->cond);
            pthread_mutex_unlock(rsv->mut);
            return NULL;
        }
        inet_ntop(AF_INET6, &((struct sockaddr_in6 *) rs->ai_addr)->sin6_addr, 
            rsv->ip, sizeof (struct sockaddr_in6));
    } else {
        hints.ai_family = AF_INET;
        if (getaddrinfo(rsv->host, NULL, &hints, &rs) != 0) {
            pthread_mutex_lock(rsv->mut);
            pthread_cond_signal(rsv->cond);
            pthread_mutex_unlock(rsv->mut);
            return NULL;
        }
        inet_ntop(AF_INET, &((struct sockaddr_in *) rs->ai_addr)->sin_addr, 
            rsv->ip, sizeof (struct sockaddr_in));
    }
    freeaddrinfo(rs);
    rsv->done = 1;
    pthread_mutex_lock(rsv->mut);
    pthread_cond_signal(rsv->cond);
    pthread_mutex_unlock(rsv->mut);
}

static int sndloop(struct epoch_s *e, callback_snd snd_callback)
{
    int64_t hdr = 0, len = 0;
    do {
        if (snd_callback)
            snd_callback(e->nc->txbuf, &len);
        len = len < 0 ? 0 : len; 
        len = len > e->nc->bufsz ? e->nc->bufsz : len;
        hdr = len;
        if (!isbigendian())
            swapbo64(hdr);
        if (writebuf_ex(e, (char *) &hdr, sizeof hdr) == -1)
            return -1;
        if (len)
            if (writebuf_ex(e, e->nc->txbuf, len) == -1)
                return -1;
    } while (len != 0);
    e->nc->sndflag = 1;
    return 0;
}

static int rcvloop(struct epoch_s *e, callback_rcv rcv_callback)
{
    int64_t hdr;
    do {
        if (readbuf_ex(e, (char *) &hdr, sizeof hdr) == -1)
            return -1;
        if (!isbigendian())
            swapbo64(hdr);
        if (hdr && readbuf_ex(e, e->nc->rxbuf, hdr) == -1)
            return -1;
        if (rcv_callback)
            if (hdr)
                rcv_callback(e->nc->rxbuf, hdr);
            else
                rcv_callback(NULL, hdr);
    } while (hdr != 0);
    return 0;
}

static void conbuilder(char *con, const char *endpt, const char *auth, 
    enum epoch_mode mode, const char *wd, int64_t bufsz)
{
    strcpy(con, "endpoint{");
    strcat(con, endpt);
    strcat(con, "}auth{");
    strcat(con, auth);
    strcat(con, "}mode{");
    switch (mode) {
    case SINGLE_THREAD_SNDFIRST:
        strcat(con, "single_thread_sndfirst}");
        break;
    case SINGLE_THREAD_RCVFIRST:
        strcat(con, "single_thread_rcvfirst}");
        break;
    case MULTI_THREAD:
        strcat(con, "multi_thread}");
        break;
    case DETACHED:
        strcat(con, "detached}");
        break;
    }
    if (wd) {
        strcat(con, "wd{");
        strcat(con, wd);
        strcat(con, "}");
    }
    sprintf(con + strlen(con), "%s%lld%s", "bufsz{", bufsz, "}");
}

void blksigpipe(int blk, sigset_t *oldmask)
{
    if (blk) {
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGPIPE);
        pthread_sigmask(SIG_SETMASK, &mask, oldmask);
    } else
        pthread_sigmask(SIG_SETMASK, oldmask, NULL);
}

static void *rdthread(void *pp)
{
    struct epoch_s *e = pp;
    e->nc->rdth_rc = rcvloop(pp, e->nc->rcv);
    pthread_barrier_wait(&e->nc->barrier);
    return NULL;
}

static void *wrthread(void *pp)
{
    struct epoch_s *e = pp;
    e->nc->wrth_rc = sndloop(pp, e->nc->snd);
    pthread_barrier_wait(&e->nc->barrier);
    return NULL;
}

static int recves(struct epoch_s *e, int *es)
{
    if (readbuf_ex(e, (char *) es, sizeof *es) == -1)
        return -1;
    if (!isbigendian())
        swapbo32(*es); 
    return 0;
}

static int epoch_epcore(struct epoch_s *e, const char *endpt, const char *auth, 
     enum epoch_mode mode, int64_t bufsz, const char *wd, int ex)
{
    e->nc = malloc(sizeof(struct netconn));
    if (!e->nc)
        return EPOCH_EBUFNULL;
    if (bufsz < DEFCOMBUF)
        bufsz = DEFCOMBUF;
    if (!ex) {
        e->nc->rxbuf = malloc(bufsz);
        e->nc->txbuf = malloc(bufsz);
        if (!e->nc->rxbuf || !e->nc->txbuf) {
            freecomm(e);
            return EPOCH_EBUFNULL;
        }
    } else {
        e->nc->rxbuf = e->nc->stdline;
        e->nc->txbuf = NULL;
    }
    e->nc->bufsz = bufsz;
    e->nc->cst = CRYPT_OFF;
    e->nc->sock = 0;
    if (strlen(e->ipv4) > 0) {
        e->nc->sock = socket(AF_INET, SOCK_STREAM, 0);
        e->nc->addr.sin_family = AF_INET;
        e->nc->addr.sin_port = htons(e->port);
        inet_pton(AF_INET, e->ipv4, &e->nc->addr.sin_addr);
        if (e->cntmout > 0) {
            if (connecttm(e) != 0) {
                freecomm(e);
                return EPOCH_ECONNECT;
            }
        } else if (connect(e->nc->sock, (struct sockaddr *) &e->nc->addr, 
            sizeof e->nc->addr) == -1) {
            freecomm(e);
            return EPOCH_ECONNECT;
        }
    } else if (strlen(e->ipv6) > 0) {
        e->nc->sock = socket(AF_INET6, SOCK_STREAM, 0);
        e->nc->addr6.sin6_family = AF_INET6;
        e->nc->addr6.sin6_port = htons(e->port);
        inet_pton(AF_INET6, e->ipv6, &e->nc->addr6.sin6_addr);
        if (e->cntmout > 0) {
            if (connecttm(e) != 0) {
                freecomm(e);
                return EPOCH_ECONNECT;
            }
        } else if (connect(e->nc->sock, (struct sockaddr *) &e->nc->addr, 
            sizeof e->nc->addr) == -1) {
            freecomm(e);
            return EPOCH_ECONNECT;
        }
    } else {
        freecomm(e);
        return EPOCH_ECONNECT;
    }
    if (e->rdtmout > 0) {
        struct timeval tv;
        tv.tv_sec = e->rdtmout;
        tv.tv_usec = 0;
        setsockopt(e->nc->sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
    }
    int rc;
    if ((rc = sendskey(e)) != 0) {
        freecomm(e);
        return rc;
    }
    e->nc->cst = CRYPT_ON;
    e->nc->sndflag = 0;
    e->nc->rcvflag = 0;
    conbuilder(e->nc->rxbuf, endpt, auth, mode, wd, bufsz);
    int epdlen = strlen(e->nc->rxbuf);
    int64_t hdr = epdlen;
    if (!isbigendian())
        swapbo64(hdr);
    if (writebuf_ex(e, (char *) &hdr, sizeof hdr) == -1) {
        freecomm(e);
        return EPOCH_ESEND;
    }
    if (writebuf_ex(e, e->nc->rxbuf, epdlen) == -1) {
        freecomm(e);
        return EPOCH_ESEND;
    }
    if (ex)
        e->nc->rxbuf = NULL;
    if (readbuf_ex(e, (char *) &hdr, sizeof hdr) == -1) {
        freecomm(e);
        return EPOCH_ERECV;
    }
    if (hdr) {
        freecomm(e);
        return EPOCH_EENDPTAUTH;
    }
    return 0;
}

int epoch_send_ex(struct epoch_s *e, void *buf, int64_t len)
{
    if (!e->nc->sndflag) {
        int64_t hdr = 0;
        len = len < 0 ? 0 : len; 
        len = len > e->nc->bufsz ? e->nc->bufsz : len;
        hdr = len;
        if (!isbigendian())
            swapbo64(hdr);
        if (writebuf_ex(e, (char *) &hdr, sizeof hdr) == -1)
            return EPOCH_ESEND;
        if (len) {
            if (writebuf_ex(e, buf, len) == -1)
                return EPOCH_ESEND;
        } else
            e->nc->sndflag = 1;
    } else
        return EPOCH_EEXFLAG;
    return 0;    
}

int epoch_recv_ex(struct epoch_s *e, void *buf, int64_t *len)
{
    int64_t hdr;
    if (!e->nc->rcvflag) {
        if (readbuf_ex(e, (char *) &hdr, sizeof hdr) == -1)
            return EPOCH_ERECV;
        if (!isbigendian())
            swapbo64(hdr);
        *len = hdr;
        if (hdr) {
            if (readbuf_ex(e, buf, hdr) == -1)
                return EPOCH_ERECV;
        } else
            e->nc->rcvflag = 1;
    } else
        return EPOCH_EEXFLAG;
    return 0;    
}

int epoch_fin_ex(struct epoch_s *e, int *es)
{
    int64_t hdr = 0;
    if (recves(e, es) == -1) {
        freecomm(e);
        return EPOCH_ERECV;
    }
    /* Finishing the two-way handsaheke. Reverse order at server side. */
    writebuf_ex(e, (char *) &hdr, sizeof hdr);
    hdr = 0;
    readbuf_ex(e, (char *) &hdr, sizeof hdr);
    freecomm(e);
    return 0;
}

int epoch_signal(struct epoch_s *e, int signo)
{
    if (!e->nc->sndflag) {
        int64_t hdr = -1;
        if (!isbigendian())
            swapbo64(hdr);
        if (writebuf_ex(e, (char *) &hdr, sizeof hdr) == -1)
            return EPOCH_ESEND;
        hdr = signo;
        if (!isbigendian())
            swapbo64(hdr);
        if (writebuf_ex(e, (char *) &hdr, sizeof hdr) == -1)
            return EPOCH_ESEND;
        return 0;
    }
    return EPOCH_ESIGCONXT;
}
