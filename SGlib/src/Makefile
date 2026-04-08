CC=sdcc
AR=sdar

########### BUILD CONFIGURATION ##############
CONFIG= 

############# END OF BUILD CONFIG OPTIONS #############

OPT=--max-allocs-per-node 100000
CFLAGS=-mz80 $(OPT) $(CONFIG)

OUTPUT_LIBS=SGlib.lib SGlib_CV.lib

SRCS_COMMON=SGlib_spriteClip.c SGlib_zx7.c SGlib_debug.c
SRCS_SG=SGlib.c SGlib_zx7_VRAM.c SGlib_bitmap.c SGlib_keyboard.c 
SRCS_CV=SGlib.c SGlib_zx7_VRAM.c SGlib_bitmap.c

# One .rel per common source file. Add target specific .rel dependencies here.
OBJS_COMMON=$(patsubst %.c,%.rel,$(SRCS_COMMON))
OBJS_SG=$(patsubst %.c,%.rel,$(SRCS_SG))
OBJS_CV=$(patsubst %.c,%_CV.rel,$(SRCS_CV))

ALL: $(OUTPUT_LIBS)

# Library outputs
SGlib.lib: $(OBJS_SG) $(OBJS_COMMON)
	sdar r $@ $^

SGlib_CV.lib: $(OBJS_CV) $(OBJS_COMMON)
	sdar r $@ $^

# Common and SG rules
$(OBJS_COMMON) $(OBJS_SG): %.rel: %.c SGlib.h SGlib_common.h
	$(CC) $(CFLAGS) $(PEEP_OPTIONS) -c -o $@ $<

# ColecoVision rules add compiler switch
$(OBJS_CV): %_CV.rel: %.c SGlib.h SGlib_common.h
	$(CC) $(CFLAGS) $(PEEP_OPTIONS) -DTARGET_CV -c -o $@ $<

clean:
	rm -f $(OUTPUT_LIBS) $(OBJS_SG) $(OBJS_CV) $(OBJS_COMMON) *.lst *.sym *.asm
