#Release profile

cflags = -I ./api/include
lflags = -L api/release -L api/debug
obj = release/obj/test.o
	
libs = -lpthread -lepoch
lib_debug = -lepoch_d

CCX = gcc -o
CC = gcc -c


release: $(obj) api_release
	$(CCX) release/test $(obj) $(libs) $(lflags)

release/obj/test.o: src/test.c
	$(CC) src/test.c -o release/obj/test.o $(cflags) 

# Debug profile

dbg = debug/obj/test.o

CCGX = gcc -g -DDEBUG -o
CCG = gcc -g -DDEBUG -c
	
debug: $(dbg) api_debug
	$(CCGX) debug/test $(dbg) $(lib_debug) $(lflags)

debug/obj/test.o: src/test.c
	$(CCG) src/test.c -o debug/obj/test.o $(cflags) 

#Others rules

api_release:
	cd api && $(MAKE)
	
api_debug:
	cd api && $(MAKE) debug
	
prepare:
	mkdir -p debug release debug/obj release/obj

clean:
	rm -R -f $(obj) $(dbg) release/obj/* debug/obj/* release/test debug/test
	cd api && $(MAKE) clean
