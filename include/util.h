/* MIT License. */

/* This file is taken from TF Protocol source code.
    It may have variations to adapt it to EPOCH Protocol. */

/*  programmer: luis miguel
    email: lmdelbahia@gmail.com  */

#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <inttypes.h>

/* Buffer for unix time in timestamp format. */
#define UXTIMELEN 22
/* Buffer for time in human readble format. */
#define HTIMELEN 32
/* Number of decimal digits for long long type, including the sign and the
    traling 0. */
#define LLDIGITS sizeof(long long) / 2 * 3 + sizeof(long long) + 2
/* Number of decimal digits for int type, including the sign and the traling 0. 
    */
#define INTDIGITS sizeof(int) / 2 * 3 + sizeof(int) + 2
/* Swap byte order of 32 bit integer: Big-to-Little or Little-to-Big endian. */
#define swapbo32(value) value = (value >> 24 & 0xFF) | (value >> 16 & 0xFF) << \
    8 | (value >> 8 & 0xFF) << 16 | (value & 0xFF) << 24;
/* Swap byte order of 64 bit integer: Big-to-Little or Little-to-Big endian. */
#define swapbo64(value) value = (value >> 56 & 0xFF) | (value >> 48 & 0xFF) << \
    8 | (value >> 40 & 0xFF) << 16 | (value >> 32 & 0xFF) << 24 | (value >> 24 \
    & 0xFF) << 32 | (value >> 16 & 0xFF) << 40 | (value >> 8 & 0xFF) << 48 | \
    (value & 0xFF) << 56
/* MS -millisecond- multiplication factor. */
#define MILLISEC 1000
/* Nanosecond multiplication factor. */
#define NANOSEC 1000000000

int isbigendian(void);
/* Write chunk of data to file (fd). */
int64_t writechunk(int fd, char *buf, int64_t len);
/* Read chunk of data from file (fd). */
int64_t readchunk(int fd, char *buf, int64_t len);

#endif
