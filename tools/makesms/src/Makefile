CC=gcc
LD=$(CC)
CFLAGS=-Wall -O1
LDFLAGS=

all: makesms

makesms: makesms.o
	$(LD) $(LDFLAGS) -o $@ $^
