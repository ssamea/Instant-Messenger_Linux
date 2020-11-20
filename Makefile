TARGET = client
TARGET2 = server
WDIR =.


CC = gcc

CFLAGS = -Wall -O3 -D_REENTRANT
CFLAGS += -I${WDIR}/include

LDIR = -L./lib
LIBS = $(LDIR) -lClient -lpthread
LIBS2 = $(LDIR) -lServer -lpthread

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

#====================================================
all: ${TARGET} ${TARGET2} clean
#====================================================

client : $(SRCS)
	$(CC) $(CFLAGS) -o $@ $? $(LIBS)

server : $(SRCS)
	$(CC) $(CFLAGS) -o $@ $? $(LIBS2)
#-----------------------------------------------------
clean:
	rm -f core *.o a.out
#=====================================================
