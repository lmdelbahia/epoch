/* MIT License. */

/* This file is taken from TF Protocol source code.
    It may have variations to adapt it to EPOCH Protocol. */

/*  programmer: luis miguel
    email: lmdelbahia@gmail.com  */

#include <crypto.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <log.h>
#include <sys/stat.h>
#include <unistd.h>

/* Server private key. */
#define PRIVKEY "key"

extern char endpts[PATH_MAX];

/* Encrypt/Decrypt function. */
static void encrypt(struct crypto *cryp, char *data, int len);

void initcrypto(struct crypto *crypt)
{
    crypt->encrypt = encrypt;
    struct stat sa;
    char kf[PATH_MAX];
    sprintf(kf, "%s/%s", endpts, PRIVKEY);
    stat(kf, &sa);
    crypt->privkey = malloc(sa.st_size);
    if (!crypt->privkey) {
        wrlog(ELOGALLOCBUF, LGC_CRITICAL);
        exit(1);
    }
    FILE *fs = fopen(kf, "r+");
    if (fs == NULL) {
        wrlog(ELOGKEYFILE, LGC_CRITICAL);
        exit(1);
    }
    fread(crypt->privkey, sa.st_size, sizeof(char), fs);
    crypt->st = CRYPT_OFF;
    fclose(fs);
}

static void encrypt(struct crypto *cryp, char *data, int len)
{
    if (cryp->st == CRYPT_ON) {
        int c = 0, keyc = 0;
        for (; c < len; c++) {
            if (keyc == cryp->rndlen)
                keyc = 0;
            *(data + c) ^= cryp->rndkey[keyc++];
        }
    }
}

int derankey(struct crypto *crypt)
{
    RSA *rsa = NULL;
    BIO *keybio = BIO_new_mem_buf(crypt->privkey, -1);
    rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
    int pad = RSA_PKCS1_OAEP_PADDING;
    int enbyt = RSA_private_decrypt(sizeof crypt->enkey, crypt->enkey, 
        crypt->rndkey, rsa, pad);
    RSA_free(rsa);
    BIO_free(keybio);
    crypt->rndlen = enbyt;
    return enbyt;
}
