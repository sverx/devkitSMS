CC=gcc
LD=$(CC)
CFLAGS=-Wall -O1
LDFLAGS=

all: makecvmc

makecvmc: makecvmc.o
	$(LD) $(LDFLAGS) -o $@ $^
