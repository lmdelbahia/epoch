/* MIT License. */

/* This file is taken from TF Protocol source code.
    It may have variations to adapt it to EPOCH Protocol. */

/*  programmer: luis miguel
    email: lmdelbahia@gmail.com  */

#include <stdio.h>
#include <stdlib.h>
#include <tfproto.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <crypto.h>
#include <util.h>
#include <fcntl.h>
#include <sys/types.h>
#include <xs_ace/xs_ace.h>

#define STRENDPTPM 8192
#define WLD_ANYBIN '*'
#define WLD_ANYARG '%'
#define SKIP_LINE '#'

/* Instance of "struct comm". global for entire programe usage. */
struct comm comm;
/* Cryptography structures containing crypt and decrypt functions. */
struct crypto cryp_rx;
struct crypto cryp_tx;

/* Main loop until client send "end" message to terminate comm. */
static void mainloop(void);
/* The actual socket sender funtction. */
static int64_t sndbuf(char *buf, int64_t len);
/* The actual socket receiver funtction. */
static int64_t rcvbuf(char *buf, int64_t len);
/* Check authentication token. And get binary path and arg. */
static int chkauth(const char *endpt, char *out);
/* Return status to client. */
static void returnst(int64_t value);
/* Return buffer size. */
static int64_t getbufsz(const char *endpt);
/* Get execution mode. */
static enum epoch_mode getexmode(const char *endpt);
/* Wait for gracefully close. */
static void grclose(void);
/* Set working directory. */
static void swd(const char *endpt);

void begincomm(int sock, struct sockaddr_in6 *rmaddr, socklen_t *rmaddrsz)
{
    comm.sock = sock;
    comm.addr = *rmaddr;
    comm.addrsz = *rmaddrsz;
    fcntl(comm.sock, F_SETOWN, getpid());
    if (readbuf_ex(cryp_rx.enkey, RSA_KEYLEN) == -1)
        endcomm(1);
    if (derankey(&cryp_rx) == -1) 
        endcomm(1);
    dup_crypt(&cryp_tx, &cryp_rx);
    cryp_rx.st = CRYPT_ON;
    cryp_tx.st = CRYPT_ON;
    cryp_rx.pack = CRYPT_UNPACK;
    cryp_tx.pack = CRYPT_PACK;
    mainloop();
    cleanup();
    exit(0);
}

static void mainloop(void)
{
    int64_t hdr;
    if (readbuf_ex((char *) &hdr, sizeof hdr) == -1)
        endcomm(1);
    if (!isbigendian())
        swapbo64(hdr);
    comm.rxbuf = malloc(hdr + 1);
    if (!comm.rxbuf)
        endcomm(1);
    memset(comm.rxbuf, 0, hdr + 1);
    if (readbuf_ex(comm.rxbuf, hdr) == -1)
        endcomm(1);
    char endpt[PATH_MAX];
    *endpt = '\0';
    returnst(chkauth(comm.rxbuf, endpt));
    int64_t bufsz;
    if ((bufsz = getbufsz(comm.rxbuf)) == -1)
        endcomm(1);
    enum epoch_mode mode = getexmode(comm.rxbuf);
    swd(comm.rxbuf);
    if (mode != DETACHED) {
        comm.txbuf = malloc(bufsz);
        if (!comm.txbuf)
            endcomm(1);
        free(comm.rxbuf);
        comm.rxbuf = malloc(bufsz);
        if (!comm.rxbuf)
            endcomm(1);
        comm.bufsz = bufsz;
    }
    hdr = 0;
    if (writebuf_ex((char *) &hdr, sizeof hdr) == -1)
        endcomm(1);
    xs_ace(mode, endpt);
}

void cleanup(void)
{
    if (comm.rxbuf)
        free(comm.rxbuf);
    if (comm.txbuf)
        free(comm.txbuf);
    close(comm.sock);
}

int64_t readbuf_ex(char *buf, int64_t len)
{
    return rcvbuf(buf, len);
}

int64_t writebuf_ex(char *buf, int64_t len)
{
    return sndbuf(buf, len);
}

static int64_t sndbuf(char *buf, int64_t len)
{
    if (cryp_tx.encrypt)
        cryp_tx.encrypt(&cryp_tx, buf, len);
    int64_t wb = 0;
    int64_t written = 0;
    while (len > 0) {
        do
            wb = write(comm.sock, buf + written, len);
        while (wb == -1 && errno == EINTR);
        if (wb == -1)
            return -1;
        len -= wb;
        written += wb;
    }
    return written;
}

static int64_t rcvbuf(char *buf, int64_t len)
{
    int64_t rb = 0;
    int64_t readed = 0;
    while (len > 0) {
        do
            rb = read(comm.sock, buf + readed, len);
        while (rb == -1 && errno == EINTR);
        if (rb == 0 || rb == -1)
            return -1;
        len -= rb;
        readed += rb;
    }
    if (cryp_rx.encrypt)
        cryp_rx.encrypt(&cryp_rx, buf, readed);
    return readed;
}

void endcomm(int rs)
{
    cleanup();
    exit(rs);
}

static int chkauth(const char *endpt, char *out)
{
    char endptcp[STRENDPTPM];
    strcpy(endptcp, endpt);
    char *auth = strstr(endptcp, "auth{");
    if (!auth) 
        return -1;
    auth += strlen("auth{");
    char *tok = strtok(auth, "}");
    if (!tok)
        return -1;
    char path[PATH_MAX];
    extern char endpts[PATH_MAX];
    strcpy(path, endpts);
    strcat(path, tok);
    FILE *binfl = fopen(path, "r");
    if (!binfl)
        return -1;
    strcpy(endptcp, endpt);
    auth = strstr(endptcp, "endpoint{");
    if (!auth) {
        fclose(binfl);
        return -1;
    }
    auth += strlen("endpoint{");
    tok = strtok(auth, "}");
    if (!tok) {
        fclose(binfl);
        return -1;
    }
    while (fgets(path, sizeof path, binfl) != NULL) {
        char *chpt = strchr(path, '\n');
        if (chpt) 
            *chpt = '\0';
        if (*path == SKIP_LINE)
            continue;
        else if (*path == WLD_ANYBIN) {
            strcpy(out, tok);
            fclose(binfl);
            return 0;
        } else if (*path == WLD_ANYARG) {
            char bin[LINE_MAX], auth[LINE_MAX];
            strcpy(bin, tok);
            strcpy(auth, path + 1);
            char *bintok = strtok(bin, " ");
            char *authtok = strtok(auth, " ");
            if (bintok && authtok && !strcmp(bintok, authtok)) {
                strcpy(out, tok);
                fclose(binfl);
                return 0;
            }
        } else if (!strcmp(path, tok)) {
            tok = strtok(path, "");
            if (tok) {
                strcpy(out, tok);
                fclose(binfl);
                return 0;
            }
        }
    }
    fclose(binfl);
    return -1;
}

static void returnst(int64_t value)
{
    if (value == -1) {
        if (!isbigendian())
            swapbo64(value);
        if (writebuf_ex((char *) &value, sizeof value) == -1)
            endcomm(1);
        grclose();
        cleanup();
        exit(0);
    }
}

static int64_t getbufsz(const char *endpt)
{
    char endptcp[STRENDPTPM];
    strcpy(endptcp , endpt);
    char *bsz = strstr(endptcp, "bufsz{");
    if (!bsz) 
        return -1;
    bsz += strlen("bufsz{");
    char *tok = strtok(bsz, "}");
    if (!tok)
        return -1; 
    int64_t bufsz = atoll(tok);
    return bufsz > COMMBUFLEN ? bufsz : COMMBUFLEN;
}

static enum epoch_mode getexmode(const char *endpt)
{
    char endptcp[STRENDPTPM];
    strcpy(endptcp , endpt);
    char *bsz = strstr(endptcp, "mode{");
    if (!bsz) 
        return -1;
    bsz += strlen("mode{");
    char *tok = strtok(bsz, "}");
    if (!tok)
        return -1; 
    if (!strcmp(tok, "single_thread_sndfirst"))
        return SINGLE_THREAD_SNDFIRST;
    else if (!strcmp(tok, "single_thread_rcvfirst"))
        return SINGLE_THREAD_RCVFIRST;
    else if (!strcmp(tok, "multi_thread"))
        return MULTI_THREAD;
    else
        return DETACHED;
}

static void grclose(void)
{
    int64_t hdr;
    struct timeval tv;
    tv.tv_sec = 60;
    tv.tv_usec = 0;
    setsockopt(comm.sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
    readbuf_ex((char *) &hdr, sizeof hdr);
}

static void swd(const char *endpt)
{
    char endptcp[STRENDPTPM];
    strcpy(endptcp , endpt);
    char *bsz = strstr(endptcp, "wd{");
    if (!bsz) 
        return;
    bsz += strlen("wd{");
    char *tok = strtok(bsz, "}");
    if (!tok)
        return;
    chdir(tok);
}
