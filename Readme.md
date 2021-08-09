Abstract.
The EPOCH protocol is based on the XS_ACE ‘Arbitrary Code Execution’
subsystem of TF Protocol, in fact, is a subset of XS_ACE (see TF Protocol
documentation for further reading). The idea behind EPOCH is to provide an
abstraction layer for communication and encryption allowing the construction
of functionalities or commands ‘Called endpoints in EPOCH’ on top of it.
Those endpoints are programs that can be written in any interpreted or JIT
language, such Python or Java; or any AOT language like C, C++, Pascal,
and so on. As far as concern to the ‘endpoint’, this is, the program running on
top of EPOCH, only matters reading from standard input and writing to the
standard output.

Just wanna use it! C example. (May be you need to install openssl libs)

    git clone https://github.com/lmdelbahia/epoch.git
    cd epoch
    make clean; make
    sudo make install
    cd client_side/c (done)/api/
    make clean; make
    sudo make install
    
    Run the server
    epoch /etc/epoch 55001 x
    
    Create a test program
    nano main.c
    
    Paste this into main.c
    
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
    int es = 0;
    rc = epoch_endpoint(&e, "/bin/ls -la", "hash_authority", MULTI_THREAD, 3000,
        snd_callback, rcv_callback, NULL, &es);
    /* Print the endpoint output. */
    printf("%s\n", bf);
    /* Print the epoch_endpoint return value. */
    printf("%d\n", rc);
    /* Print the endpoint -Program executed at the server- exit status. */
    printf("%d\n", WEXITSTATUS(es));
    return 0;
}

####
    Save main.c and close it.
    
    gcc -o epoch_test main.c -lepoch
    
    Run the program and see the result.
    
    ./epoch_test
    
What does this program do?

    -Connect to the server through EPOCH Protocol.
    -Execute remotely the ls command.
    -Receive the ls output.
    -Print that output
    
For further complex programs, please read the docs. It doen't bite you!
    

You can find in the folders the server written in C and many clientes written in different languages.

To run server in debug mode execute it like this
debug/epoch ./conf 55001

The first parameter indicates the folder of the server private key and the authority file.
    -The private server key is in PEM format. This is a file called "key" without the double quotes in the folder indicated by the first parameter.
    -The authority file is a list of programs and paramters that can be run from EPOCH Protocol. The exact authority file among all of them is specified by the client side.
    
The second parameter indicate the port in which the server will listen for incomming connections.

To run server in production mode execute it like this
release/epoch ./conf/ 55001 x

The first two parameters are the same as in the debug mode. The third parameter could be any number or letter or string. The fact that there is a third parameter is what tells the server that it has to be run in production mode.

To stop the server it can be used the kill or pkill command sending the SIGINT signal.

Is not recomended to run the server as root user due it can execute programs 'endpoints' that potentially may harm the system. If is needed to run it as other user, it can be executed from a shell that sets the desired effective user id or by any other means.
