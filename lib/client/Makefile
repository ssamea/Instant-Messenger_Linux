TARGET = libClient.a

CC =gcc

WDIR =../..
LDIR = ${WDIR}/lib

#SHARED = -shared -fPIC

CFLAGS = -Wall -O3 -D_REENTARNT
CFLAGS += -I./include -I${WDIR}/include

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

#==================================================
all: ${TARGET} clean
#==================================================

libClient.a : $(OBJS)
	ar -rscv $@ $?
	mv $@ ${LDIR}

clean:
	rm -rf core *.o a.out

