CC       = gcc
CFLAGS   = -Wall -O1
CHECK_CFLAGS  := $(shell pkg-config --cflags check 2>/dev/null)
CHECK_LDFLAGS := $(shell pkg-config --libs   check 2>/dev/null)

ifeq ($(CHECK_LDFLAGS),)
$(error libcheck not found. Install it first: apt-get install check / brew install check)
endif

all: test_invariant_makecvmc

test_invariant_makecvmc: test_invariant_makecvmc.c
	$(CC) $(CFLAGS) $(CHECK_CFLAGS) -o $@ $< $(CHECK_LDFLAGS)

check: test_invariant_makecvmc
	./test_invariant_makecvmc

clean:
	rm -f test_invariant_makecvmc
