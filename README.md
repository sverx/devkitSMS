devkitSMS
=========

a collection of tools and code (with a very presumptuous name) to help in SEGA Master System homebrew development, using SDCC

How to code your own programs using devkitSMS:

* download and install SDCC - http://sdcc.sourceforge.net/
* read its manual (it's in SDCC 'doc' folder) - or http://sdcc.sourceforge.net/doc/sdccman.pdf
* make sure that your installation works - details at page 20 of the PDF manual
* place crt0.rel from this package into your SDCC 'lib/z80' folder
* place ihx2sms.exe from this package into your SDCC 'bin' folder
  (if you're not on Windows please compile this tool youself from the source)
* place SMSlib.h and SMSlib.rel in your project folder
  or you can create a folder for SMSlib, place both SMSlib.c and SMSlib.h there and compile it yourself with SDCC:
  sdcc -c -mz80 --std-sdcc99 --fomit-frame-pointer SMSlib.c

How to use SMSlib:

* include SMSlib.h in your sources
* compile your program adding the path to SMSlib.rel as last parameter:
  sdcc -mz80 --std-sdcc99 --fomit-frame-pointer --data-loc 0xC000 your_program.c ..\SMSlib\SMSlib.rel

How to use more than 48KB in your ROM:

* in your program, use the SMSlib provided 'SMS_mapROMBank(n)' macro to map the bank you need
* put your data and compile a separate .c file for each 16KB ROM bank starting from bank2, for example bank2.c, bank3.c etc...
  giving each one a different CONST segment name (I suggest using BANK#):
  sdcc -c -mz80 --std-sdcc99 --constseg BANK2 bank2.c
  sdcc -c -mz80 --std-sdcc99 --constseg BANK3 bank3.c
* compile your program adding a parameter for the linker for each bank (_BANK#) and adding each .rel file to be linked, in ascending order, at the end:
  sdcc -mz80 --std-sdcc99 --fomit-frame-pointer --data-loc 0xC000 -Wl-b_BANK2=0x8000 -Wl-b_BANK3=0x8000 your_program.c ..\SMSlib\SMSlib.rel bank2.rel bank3.rel

How to build the final .sms file

* use the ihx2sms utility included:
  ihx2sms your_program.ihx your_program.sms
  (the size of the ROM will be a multiple of 16KB)
