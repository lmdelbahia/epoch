To run server in debug mode execute it like this
debug/epoch ./conf 55001

The first parameter indicates the folder of the server private key and the authority file.
    -The private server key is in PEM format.
    -The authority file is a list of programs and paramters that can be run from EPOCH Protocol
    
The second parameter indicate the port in which the server will listen for incomming connections.

To run server in production mode execute it like this
release/epoch ./conf/ 55001 x

The first two parameters are the same as in the debug mode. The third parameter could be any number or letter or string. The fact that there is a third parameter is what tells the server that it has to be run in production mode.

To stop the server it can be used the kill or pkill command sending the SIGINT signal.

Is not recomended to run the server as root user due it can execute programs 'endpoints' that potentially may harm the system. If is needed to run it as other user, it can be executed from a shell that sets the desired effective user id or by any other means.
