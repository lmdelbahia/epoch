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
