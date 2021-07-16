/* MIT License. */

/* This file is taken from TF Protocol source code.
    It may have variations to adapt it to EPOCH Protocol. */

/*  programmer: luis miguel
    email: lmdelbahia@gmail.com  */

#include <util.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <ctype.h>
#include <dirent.h>

int isbigendian(void)
{
    int value = 1;
    char *pt = (char *) &value;
    if (*pt == 1)
        return 0;
    else
        return 1;
}

int64_t writechunk(int fd, char *buf, int64_t len)
{
    int64_t wb;
    int64_t written = 0;
    do {
        do 
            wb = write(fd, buf + written, len - written);
        while (wb == -1 && errno == EINTR);
        written += wb;
    } while (wb != -1 && len - written > 0);
    return wb;
}

int64_t readchunk(int fd, char *buf, int64_t len)
{
    int64_t rb;
    do
        rb = read(fd, buf, len);
    while (rb == -1 && errno == EINTR);
    return rb;
}
