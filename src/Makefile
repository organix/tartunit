#
# Makefile for tartunit library (libtartunit.a)
#

INC=	../inc
LIB=	../lib
TART=   ../tart

LIBTART= 	$(TART)/lib/libtart.a
# LIBTARTUNIT=$(LIB)/libtartunit.a
INCS=	   	$(INC)/tartunit.h
# OBJS=		actor.o \
# 			expr.o \
# 			number.o
PROGS=		tartunit

CFLAGS= -I$(INC) -I$(TART)/inc -I$(LIBTART)

# all: $(LIBTARTUNIT) $(PROGS)
all: $(LIBTART) $(PROGS)

test: all
	./tartunit

$(LIBTART):
	cd $(TART)/src; make clean && make
# $(LIBTARTUNIT): $(OBJS)
# 	$(AR) rv $@ $(OBJS)

# $(OBJS): $(INCS)

# tartunit: tartunit.o $(LIBTARTUNIT)
# 	$(CC) $(CFLAGS) -o $@ tartunit.o $(LIBTARTUNIT)
tartunit: tartunit.o $(LIBTART)
	$(CC) $(CFLAGS) -o $@ tartunit.o $(LIBTART)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(LIB)
	mkdir $(LIB)
	rm -f $(PROGS)
	rm -f *.o *~ *.stackdump
	cd $(TART)/src; make clean
