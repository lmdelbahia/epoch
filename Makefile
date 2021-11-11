cflags = -D_FILE_OFFSET_BITS=64 -I ./include -Wall -Wno-parentheses \
	-Wno-pointer-sign

#Release profile

obj = release/obj/main.o release/obj/log.o release/obj/sig.o release/obj/net.o \
	release/obj/util.o release/obj/tfproto.o release/obj/crypto.o \
	release/obj/xs_ace.o
	
libs = -lpthread -lcrypto

CCX = gcc -o
CC = gcc -c

release: $(obj) 
	$(CCX) release/epoch $(obj) $(libs)

release/obj/main.o: src/main.c
	$(CC) src/main.c -o release/obj/main.o $(cflags) 

release/obj/log.o: src/log.c include/log.h
	$(CC) src/log.c -o release/obj/log.o $(cflags) 

release/obj/sig.o: src/sig.c include/sig.h
	$(CC) src/sig.c -o release/obj/sig.o $(cflags) 
		
release/obj/net.o: src/net.c include/net.h
	$(CC) src/net.c -o release/obj/net.o $(cflags) 

release/obj/util.o: src/util.c include/util.h
	$(CC) src/util.c -o release/obj/util.o $(cflags) 
			
release/obj/tfproto.o: src/tfproto.c include/tfproto.h
	$(CC) src/tfproto.c -o release/obj/tfproto.o $(cflags) 

release/obj/crypto.o: src/crypto.c include/crypto.h
	$(CC) src/crypto.c -o release/obj/crypto.o $(cflags) 

release/obj/xs_ace.o: src/xs_ace/xs_ace.c include/xs_ace/xs_ace.h
	$(CC) src/xs_ace/xs_ace.c -o release/obj/xs_ace.o $(cflags) 
	
# Debug profile

dbg = debug/obj/main.o debug/obj/log.o debug/obj/sig.o debug/obj/net.o \
	debug/obj/util.o debug/obj/tfproto.o debug/obj/crypto.o \
	debug/obj/xs_ace.o

CCGX = gcc -g -DDEBUG -o
CCG = gcc -g -DDEBUG -c
	
debug: $(dbg) 
	$(CCGX) debug/epoch $(dbg) $(libs)

debug/obj/main.o: src/main.c
	$(CCG) src/main.c -o debug/obj/main.o $(cflags) 

debug/obj/log.o: src/log.c include/log.h
	$(CCG) src/log.c -o debug/obj/log.o $(cflags) 

debug/obj/sig.o: src/sig.c include/sig.h
	$(CCG) src/sig.c -o debug/obj/sig.o $(cflags) 

debug/obj/net.o: src/net.c include/net.h
	$(CCG) src/net.c -o debug/obj/net.o $(cflags) 

debug/obj/util.o: src/util.c include/util.h
	$(CCG) src/util.c -o debug/obj/util.o $(cflags) 
				
debug/obj/tfproto.o: src/tfproto.c include/tfproto.h
	$(CCG) src/tfproto.c -o debug/obj/tfproto.o $(cflags) 

debug/obj/crypto.o: src/crypto.c include/crypto.h
	$(CCG) src/crypto.c -o debug/obj/crypto.o $(cflags) 

debug/obj/xs_ace.o: src/xs_ace/xs_ace.c include/xs_ace/xs_ace.h
	$(CCG) src/xs_ace/xs_ace.c -o debug/obj/xs_ace.o $(cflags) 
	
#Others rules

prepare:
	mkdir -p debug release debug/obj release/obj

install:
	cp release/epoch /usr/bin/
	mkdir -p /etc/epoch
	cp -R conf/* /etc/epoch/

remove:
	pkill -9 epoch
	rm /usr/bin/epoch
	rm -R /etc/epoch

clean:
	rm -R -f $(obj) $(dbg) release/obj/* debug/obj/* release/epoch debug/epoch
