CONFIG = 
CFLAGS =  -O -Wall -Wshadow $(CONFIG)
CC = gcc
LIBS = -levent -levent_pthreads -lpthread ../modnet_main/modnet_sys.o ../modnet_main/modnet_main.o 

exes = dummy_module 

all:   $(exes)

dummy_module: dummy_module.o
	$(CC) $(CONFIG) -o dummy_module dummy_module.o  $(LIBS)

clean:
	rm -f $(exes) *.o
