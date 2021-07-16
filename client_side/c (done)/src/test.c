/* This program is an example of how to use the EPOCH client api. */
#include <stdio.h>
#include <stdlib.h>
#include <epoch.h>
#include <string.h>
#include <signal.h>

char bf[1000];
int pos = 0;

/* Receive callback. */
void rcv_callback(void *buf, int64_t len)
{    
    if (len) {
        memcpy(bf + pos, buf, len);
        pos += len;
    }
}

/* Send callback. */
void snd_callback(void *buf, int64_t *len)
{
    static int i = 0;
    if (i < 10) {
        sprintf(buf, "%s%d\n", "Send something ", i);
        *len = strlen((char *) buf) + 1;
        i++;
    } else
        *len = 0;
}


int main(void)
{
    /* For EPOCH client usage, see EPOCH API documentation. */
    int rc;
    char ip[16];
    /* Resolv ip from host. */
    rc = resolvhn("google.com", ip, 0, 4);
    printf("%d %s\n", rc, ip);
    /* Initialize the EPOCH structure.*/ 
    struct epoch_s e = { "127.0.0.1", "", 55001, "sessionkey", 10, "", 0, 3};
    /* Server public key. PEM Format. */
    char pubkey[] = "-----BEGIN PUBLIC KEY-----\n"
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAq2K/X7ZSKlrMBIrSY9G/\n"
        "LLB+0injPCX17U7vb8XedQbjBT2AJ+qxmT4VLR1EWnvdUdvxaX9kRahI4hlSnfWl\n"
        "IddPfJRPH97Rk0OlMEQBclhD4WL88T8o4lVu0nuo8UAjqY0As6g6ZCG1s/Wfr64N\n"
        "aSvFr8NAYIaTQ6PbKxiypTythsSAkp5eAMkaje/ZAhkY1h0zMz09eg17veED8dIb\n"
        "R5sc7j05ndDOGucqY4+u9F/CZQNyOysKcFYjtYz/pStBPYY8CcU82SwW0Y2tbzy2\n"
        "j30ADzroySlQw+VjcffrGJao9qea1GWGwGMv4d4baMxrZeid2uB7NMUdljW8owWa\n"
        "VwIDAQAB\n" "-----END PUBLIC KEY-----\n";
    e.pubkey = pubkey;
    /* Run an endpoint. */   
    rc = epoch_endpoint(&e, "/bin/ls -la", "hash_authority", MULTI_THREAD, 3000,
        snd_callback, rcv_callback);
    printf("%s\n", bf);
    printf("%d\n", rc);
}
