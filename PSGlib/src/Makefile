CC=sdcc
AR=sdar

CFLAGS=-mz80 --max-allocs-per-node 100000

OUTPUT_LIBS=PSGlib.lib PSGlib_MB.lib PSGlib_NOSFX.lib
SOURCES=PSGlib.c PSGAttenuation.c PSGPlayLoops.c PSGRestoreVolumes.c PSGResume.c

OBJS_STD=$(patsubst %.c,%.rel, $(SOURCES))
OBJS_MB=$(patsubst %.c,%_MB.rel, $(SOURCES))
OBJS_NOSFX=$(patsubst %.c,%_NOSFX.rel, $(SOURCES))

ALL: $(OUTPUT_LIBS)

# Library outputs
PSGlib.lib: $(OBJS_STD)
	sdar r $@ $^

PSGlib_MB.lib: $(OBJS_MB)
	sdar r $@ $^

PSGlib_NOSFX.lib: $(OBJS_NOSFX)
	sdar r $@ $^


# Specific rules for some files with particular compilation options or specific
# to a target.

%.rel: %.c PSGlib_extern.h  PSGlib.h
	$(CC) $(CFLAGS) -c -o $@ $<

%_MB.rel: %.c PSGlib_extern.h  PSGlib.h
	$(CC) $(CFLAGS) -c -o $@ $< -DPSGLIB_MULTIBANK

%_NOSFX.rel: %.c PSGlib_extern.h  PSGlib.h
	$(CC) $(CFLAGS) -c -o $@ $< -DPSGLIB_NOSFXCODE

clean:
	rm -f $(OUTPUT_LIBS) $(OBJS_STD) $(OBJS_MB) $(OBJS_NOSFX) *.lst *.sym *.asm

