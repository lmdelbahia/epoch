#Release profile

cflags = -I ./api/include
lflags = -L api/release -L api/debug
obj = release/obj/test.o
obj2 = release/obj/test2.o

libs = -lpthread -lepoch
lib_debug = -lepoch_d

CCX = gcc -o
CC = gcc -c


release: $(obj) api_release
	$(CCX) release/test $(obj) $(libs) $(lflags)

release/obj/test.o: src/test.c
	$(CC) src/test.c -o release/obj/test.o $(cflags) 

r2: $(obj2) api_release
	$(CCX) release/test2 $(obj2) $(libs) $(lflags)
	
release/obj/test2.o: src/test2.c
	$(CC) src/test2.c -o release/obj/test2.o $(cflags) 
	
# Debug profile

dbg = debug/obj/test.o
dbg2 = debug/obj/test2.o

CCGX = gcc -g -DDEBUG -o
CCG = gcc -g -DDEBUG -c
	
debug: $(dbg) api_debug
	$(CCGX) debug/test $(dbg) $(lib_debug) $(lflags)

debug/obj/test.o: src/test.c
	$(CCG) src/test.c -o debug/obj/test.o $(cflags) 
	
d2: $(dbg2) api_debug
	$(CCGX) debug/test2 $(dbg2) $(lib_debug) $(lflags)
	
debug/obj/test2.o: src/test2.c
	$(CCG) src/test2.c -o debug/obj/test2.o $(cflags) 
	
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
