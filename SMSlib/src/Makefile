CC=sdcc
AR=sdar

########### BUILD CONFIGURATION ##############

# Enables a faster version of SMS_addSprite, which does not return
# a handle and does not check for sprite terminator and sprite
# availability
#CONFIG+=-DNO_SPRITE_CHECKS

# Enable support for Megadrive controllers
#CONFIG+=-DMD_PAD_SUPPORT

# Enable support for VDP type detection
#CONFIG+=-DVDPTYPE_DETECTION

# Build a lib supporting less sprites to save memory. Default 64 (all).
#CONFIG+=-DMAXSPRITES=64

# Omit support for a frame interrupt hook to save a few bytes
# and cycles.
#CONFIG+=-DNO_FRAME_INT_HOOK

############# END OF BUILD CONFIG OPTIONS #############

OPT=--max-allocs-per-node 100000
CFLAGS=-mz80 $(OPT) $(CONFIG)
PEEP_OPTIONS=--peep-file peep-rules.txt --peep-asm

OUTPUT_LIBS=SMSlib.lib SMSlib_GG.lib

# Files that are compiled once and used for SMS and GG
SRCS=SMSlib_aPLib.c SMSlib_autotext.c SMSlib_deprecated.c SMSlib_load1bppTiles.c SMSlib_loadTileMapArea.c SMSlib_paddle.c SMSlib_paletteAdv.c SMSlib_PSGaiden.c  SMSlib_spriteAdv.c SMSlib_sprite.c SMSlib_STMcomp.c SMSlib_textrenderer.c SMSlib_threesprites.c SMSlib_twosprites.c SMSlib_UNSAFE.c SMSlib_UNSAFE_zx7.c SMSlib_VRAMmemcpy_brief.c SMSlib_VRAMmemcpy.c SMSlib_VRAMmemset.c SMSlib_zx7.c

# One .rel per common source file. Add target specific .rel dependencies here.
OBJS_SMS=$(patsubst %.c,%.rel,$(SRCS)) SMSlib.rel SMSlib_autotext.rel SMSlib_spriteClip.rel
OBJS_GG=$(patsubst %.c,%.rel,$(SRCS)) SMSlib_GG.rel SMSlib_autotext_GG.rel SMSlib_spriteClip_GG.rel

ALL: $(OUTPUT_LIBS)

# Library outputs
SMSlib.lib: $(OBJS_SMS)
	sdar r $@ $^

SMSlib_GG.lib: $(OBJS_GG)
	sdar r $@ $^


# Specific rules for some files with particular compilation options or specific
# to a target.

SMSlib.rel: SMSlib.c SMSlib.h peep-rules.txt
	#$(CC) $(CFLAGS) $(PEEP_OPTIONS) --reserve-regs-iy -c -o $@ $<
	$(CC) $(CFLAGS) $(PEEP_OPTIONS) -c -o $@ $<

SMSlib_GG.rel: SMSlib.c SMSlib.h peep-rules.txt
	#$(CC) $(CFLAGS) $(PEEP_OPTIONS) --reserve-regs-iy -c -o $@ $< -DTARGET_GG
	$(CC) $(CFLAGS) $(PEEP_OPTIONS) -c -o $@ $< -DTARGET_GG

SMSlib_autotext_GG.rel: SMSlib_autotext.c SMSlib.h peep-rules.txt
	$(CC) $(CFLAGS) $(PEEP_OPTIONS) -c -o $@ $< -DTARGET_GG

SMSlib_spriteClip_GG.rel: SMSlib_spriteClip.c SMSlib.h peep-rules.txt
	$(CC) $(CFLAGS) $(PEEP_OPTIONS) -c -o $@ $< -DTARGET_GG

# No --peep-asm for the following since it could break timing of handwritten code
SMSlib_STMcomp.rel: SMSlib_STMcomp.c
	$(CC) $(CFLAGS) --peep-file peep-rules.txt -c -o $@ $<

SMSlib_UNSAFE.rel: SMSlib_UNSAFE.c
	$(CC) $(CFLAGS) --peep-file peep-rules.txt -c -o $@ $<

SMSlib_PSGaiden.rel: SMSlib_PSGaiden.c
	$(CC) $(CFLAGS) --peep-file peep-rules.txt -c -o $@ $<

SMSlib_zx7.rel: SMSlib_zx7.c
	$(CC) $(CFLAGS) -c -o $@ $<

SMSlib_UNSAFE_zx7.rel: SMSlib_UNSAFE_zx7.c
	$(CC) $(CFLAGS) -c -o $@ $<

SMSlib_aPLib.rel: SMSlib_aPLib.c
	$(CC) $(CFLAGS) -c -o $@ $<

SMSlib_paddle.rel: SMSlib_paddle.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Default rules
%.rel: %.c SMSlib.h peep-rules.txt
	$(CC) $(CFLAGS) $(PEEP_OPTIONS) -c -o $@ $<

clean:
	rm -f $(OUTPUT_LIBS) $(OBJS_SMS) $(OBJS_GG) *.lst *.sym *.asm

