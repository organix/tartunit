#
# Makefile for tartunit library (libtartunit.a)
#

INC=	../inc
LIB=	../lib

LIBTARTUNIT=$(LIB)/libtartunit.a
INCS=	   	$(INC)/tart.h \
		   	$(INC)/actor.h \
		   	$(INC)/expr.h \
		   	$(INC)/number.h
OBJS=		actor.o \
			expr.o \
			number.o
PROGS=		tart

CFLAGS= -I$(INC)

all: $(LIBTARTUNIT) $(PROGS)

test: all
	./tart

$(LIBTARTUNIT): $(OBJS)
	$(AR) rv $@ $(OBJS)

$(OBJS): $(INCS)

tart: tart.o $(LIBTARTUNIT)
	$(CC) $(CFLAGS) -o $@ tart.o $(LIBTARTUNIT)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(LIB)
	mkdir $(LIB)
	rm -f $(PROGS)
	rm -f *.o *~ *.stackdump
