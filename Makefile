CFLAGS=-g
LBPROGRAMS=bbi-32

HIDLIB=-L. -lhidapi-hidraw -Wl,-rpath,.

all: bbi-32-utils

libhidapi-hidraw.so:
	ln -s libhidapi-hidraw.so.0 libhidapi-hidraw.so

bbi-32-utils: bbi-32.c
	gcc ${CFLAGS} -o bbi-32 bbi-32.c -I.

all-clean:
	rm ${LBPROGRAMS} libhidapi-hidraw.so

clean:
	rm ${PROGRAMS}


