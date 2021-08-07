cflags = -D_FILE_OFFSET_BITS=64 -I ./include

#Release profile

obj = release/obj/epoch.o
	
libs = -lpthread -lcrypto

CCX = gcc -shared -fPIC -o
CC = gcc -c

release: $(obj) 
	$(CCX) release/libepoch.so $(obj) $(libs)

release/obj/epoch.o: src/epoch.c include/epoch.h
	$(CC) src/epoch.c -o release/obj/epoch.o $(cflags) 

# Debug profile

dbg = debug/obj/epoch.o

CCGX = gcc -g -DDEBUG -shared -fPIC -o
CCG = gcc -g -DDEBUG -c
	
debug: $(dbg) 
	$(CCGX) debug/libepoch_d.so $(dbg) $(libs)

debug/obj/epoch.o: src/epoch.c include/epoch.h
	$(CCG) src/epoch.c -o debug/obj/epoch.o $(cflags) 

#Others rules

prepare:
	mkdir -p debug release debug/obj release/obj

install:
	cp release/libepoch.so /usr/lib/
	cp debug/libepoch_d.so /usr/lib/
	cp include/epoch.h /usr/include/

remove:
	rm /usr/lib/libepoch.so
	rm /usr/lib/libepoch_d.so
	rm /usr/include/epoch.h

clean:
	rm -R -f $(obj) $(dbg) release/obj/* debug/obj/* release/libepoch.so debug/libepoch.so
