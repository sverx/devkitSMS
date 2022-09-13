CC=sdcc
AR=sdar

########### BUILD CONFIGURATION ##############


############# END OF BUILD CONFIG OPTIONS #############
OPT=
CFLAGS=-mz80 $(OPT) $(CONFIG)
PEEP_OPTIONS=

OUTPUT_LIBS=PSGlib.lib PSGlib_multibank.lib

# One .rel per common source file. Add target specific .rel dependencies here.
OBJS_STD=PSGlib.rel
OBJS_MB=PSGlib_multibank.rel

ALL: $(OUTPUT_LIBS)

# Library outputs
PSGlib.lib: $(OBJS_STD)
	sdar r $@ $^

PSGlib_multibank.lib: $(OBJS_MB)
	sdar r $@ $^


# Specific rules for some files with particular compilation options or specific
# to a target.

PSGlib.rel: PSGlib.c PSGlib.h
	$(CC) $(CFLAGS) $(PEEP_OPTIONS) -c -o $@ $<

PSGlib_multibank.rel: PSGlib.c PSGlib.h
	$(CC) $(CFLAGS) $(PEEP_OPTIONS) -c -o $@ $< -DPSGLIB_MULTIBANK

# Default rules
%.rel: %.c SMSlib.h peep-rules.txt
	$(CC) $(CFLAGS) $(PEEP_OPTIONS) -c -o $@ $<

clean:
	rm -f $(OUTPUT_LIBS) $(OBJS_STD) $(OBJS_MB) *.lst *.sym *.asm

