/* MIT License. */

/* This file is taken from TF Protocol source code.
    It may have variations to adapt it to EPOCH Protocol. */

/*  programmer: luis miguel
    email: lmdelbahia@gmail.com  */

#ifndef LOG_H
#define LOG_H

#define ELOGDFORK "EPOCH Protocol daemon failed forking the process."
#define ELOGDARG "EPOCH Protocol daemon insufficient arguments."
#define ELOGDCSOCK "EPOCH Protocol daemon failed to create the socket."
#define ELOGDBSOCK "EPOCH Protocol daemon failed to bind the socket."
#define ELOGDLSOCK "EPOCH Protocol daemon failed to listen in the socket."
#define ELOGDASOCK "EPOCH Protocol daemon failed to accept new connection."
#define SLOGDINIT "EPOCH Protocol daemon initialized successfully."
#define SLOGDRUNNING "EPOCH Protocol daemon running successfully."
#define SLOGDSRVS "EPOCH Protocol daemon start to listen for connections."
#define SLOGSIGINT "EPOCH Protocol daemon exiting due SIGINT receive."
#define ELOGKEEPALIVE "EPOCH Protocol daemon falied to set TCP keepalive timeouts."
#define ELOGINITFLAGS "EPOCH Protocol daemon invalid flags."
#define ELOGKEYFILE "EPOCH Protocol private server key missing."
#define ELOGALLOCBUF "EPOCH Protocol failed to allocate a buffer."

/* Enum to indicate the category of the log action */
enum logcat { LGC_CRITICAL, LGC_INFO, LGC_WARNING };

/* This function write to syslog */
void wrlog(const char *msg, enum logcat cat);

#endif
