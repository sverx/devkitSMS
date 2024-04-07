AS=sdasz80
OUTPUTS=crt0_sg.rel crt0_sms.rel crt0_BASIC.rel

all: $(OUTPUTS)

%.rel: %.s
	$(AS) -g -o $<

clean:
	rm -f $(OUTPUTS)
