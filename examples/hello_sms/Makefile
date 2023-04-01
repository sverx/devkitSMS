CC=sdcc
IHX2SMS=ihx2sms
DEVKITSMS_BASE=../../
SMSLIB_BASE=$(DEVKITSMS_BASE)/SMSlib
SMSLIB_INCDIR=$(SMSLIB_BASE)/src
PEEP_RULES=$(SMSLIB_BASE)/src/peep-rules.txt
CRT0=$(DEVKITSMS_BASE)/crt0/crt0_sms.rel
SMSLIB_LIB=$(SMSLIB_BASE)/SMSlib.lib          # Use distributed lib
#SMSLIB_LIB=$(SMSLIB_BASE)/src/SMSlib.lib      # Use locally compiled lib

CFLAGS=-mz80 -I$(SMSLIB_INCDIR) --peep-file $(PEEP_RULES)
LDFLAGS=-mz80 --no-std-crt0 --data-loc 0xC000

PROGNAME=hello

OBJS=main.rel

all: $(PROGNAME).sms

%.rel: %.c
	$(CC) $(CFLAGS) -c $<

%.rel: %.c %.h
	$(CC) $(CFLAGS) -c $<

$(PROGNAME).ihx: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(CRT0) $(SMSLIB_LIB) $^

$(PROGNAME).sms: $(PROGNAME).ihx
	$(IHX2SMS) $< $@

clean:
	rm -f *.rel *.ihx *.asm *.sym *.lst *.noi *.lk *.map *.sms *.gg

# When a linking error occurs, sdcc returns an error and make fails. However
# a .ihx output file still gets created. This leads to make thinking that the .ihx
# file is up to date the next time it runs... This forces linking to take place
# every time make is called.
.PHONY: $(PROGNAME).ihx
