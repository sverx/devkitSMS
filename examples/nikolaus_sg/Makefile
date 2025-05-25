CC=sdcc
IHX2SMS=ihx2sms
DEVKITSMS_BASE=../../
SGLIB_BASE=$(DEVKITSMS_BASE)SGlib
SGLIB_INCDIR=$(SGLIB_BASE)/src
PEEP_RULES=$(SGLIB_BASE)/src/peep-rules.txt
CRT0=$(DEVKITSMS_BASE)/crt0/crt0_sg.rel
SGLIB_LIB=$(SGLIB_BASE)/SGlib.rel          # Use distributed lib
#SMSLIB_LIB=$(SMSLIB_BASE)/src/SMSlib.lib      # Use locally compiled lib

CFLAGS=-mz80 -I$(SGLIB_INCDIR) --peep-file $(PEEP_RULES)
LDFLAGS=-mz80 --no-std-crt0 --data-loc 0xC000

PROGNAME=putpixel

OBJS=putpixel.rel

all: $(PROGNAME).sg

%.rel: %.c
	$(CC) $(CFLAGS) -c $<

%.rel: %.c %.h
	$(CC) $(CFLAGS) -c $<

$(PROGNAME).ihx: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(CRT0) $(SGLIB_LIB) $^

$(PROGNAME).sg: $(PROGNAME).ihx
	$(IHX2SMS) $< $@

clean:
ifeq ($(OS),Windows_NT)
	del /Q *.rel *.ihx *.asm *.sym *.lst *.noi *.lk *.map *.sms *.gg *.sc *.sg
else
	rm -f *.rel *.ihx *.asm *.sym *.lst *.noi *.lk *.map *.sms *.gg *.sc *.sg
endif
# When a linking error occurs, sdcc returns an error and make fails. However
# a .ihx output file still gets created. This leads to make thinking that the .ihx
# file is up to date the next time it runs... This forces linking to take place
# every time make is called.
.PHONY: $(PROGNAME).ihx
