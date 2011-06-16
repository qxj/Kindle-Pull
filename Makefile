### Makefile ---
## Time-stamp: <Julian Qian 2011-06-16 12:47:03>
## Author: junist@gmail.com
## Version: $Id: Makefile,v 0.0 2011/06/13 04:22:13 jqian Exp $
## Keywords:
## X-URL:

.SUFFIXES: .h .c .cpp

# CC=g++ -Wall -g -O0 -static
# CC=arm-none-linux-gnueabi-g++ -Wall -g -O0  -static
CC=arm-none-linux-gnueabi-g++ -Wall -O2  -static

KINDLEPULL=kindlepull
TEST=test

all: ${KINDLEPULL}

test: ${TEST}

KINDLEPULL_O=main.o HttpGet.o Logging.o gunzip.o
TEST_O=test.o HttpGet.o Logging.o gunzip.o


${KINDLEPULL}: ${KINDLEPULL_O}
	$(CC) -o ${KINDLEPULL} ${KINDLEPULL_O}

${TEST}: ${TEST_O}
	$(CC) -o ${TEST} ${TEST_O}

%.o:%.cpp %.h
	$(CC) -o $@ -c $<

%.o:%.cpp
	$(CC) -o $@ -c $<

clean:
	-rm -f *.o
	-rm -f ${KINDLEPULL}
	-rm -f ${TEST}
