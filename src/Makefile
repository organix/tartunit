#
# Makefile for tartunit library (libtartunit.a)
#

INC=	../inc
LIB=	../lib

LIBTARTUNIT=$(LIB)/libtartunit.a
INCS=	   	$(INC)/tart.h \
		   	$(INC)/actor.h \
		   	$(INC)/expr.h \
		   	$(INC)/number.h \
		   	$(INC)/tartunit.h
OBJS=		actor.o \
			expr.o \
			number.o
PROGS=		tartunit

CFLAGS= -I$(INC)

all: $(LIBTARTUNIT) $(PROGS)

test: all
	./tartunit

$(LIBTARTUNIT): $(OBJS)
	$(AR) rv $@ $(OBJS)

$(OBJS): $(INCS)

tartunit: tartunit.o $(LIBTARTUNIT)
	$(CC) $(CFLAGS) -o $@ tartunit.o $(LIBTARTUNIT)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(LIB)
	mkdir $(LIB)
	rm -f $(PROGS)
	rm -f *.o *~ *.stackdump
