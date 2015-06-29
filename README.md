devkitSMS
=========

a collection of tools and code (with a very presumptuous name) to help in SEGA Master System / SEGA Game Gear homebrew development, using SDCC

How to code your own programs using devkitSMS:

* download and install SDCC version 3.5.1 (build #9261 or higher requested - get a snapshot build at http://sdcc.sourceforge.net/snap.php )
* read its manual - http://sdcc.sourceforge.net/doc/sdccman.pdf
* make sure that your installation works - details at page 20 of the PDF manual
* place crt0.rel from this package into your SDCC 'lib/z80' folder
* place ihx2sms.exe from this package into your SDCC 'bin' folder
  (if you're not on Windows please compile this tool youself from the sources)
* place folder2c.exe from this package into your SDCC 'bin' folder
  (optional, you can use other tools to convert your data. If you're not on Windows please compile this tool youself from the sources)
* place SMSlib.h and SMSlib.rel (or SMSlib_GG.rel) in your project folder - or somewhere within reach ;)
  You could also create a SMSlib folder, place both SMSlib.c and SMSlib.h there and compile it yourself with SDCC:
```
  sdcc -c -mz80 --peep-file peep-rules.txt SMSlib.c
```
  note that the additional optimizing peep rules needs to be specified even if you're using requested SDCC version.
  If you want to leave out MegaDrive Pad support you can compile with
```
  sdcc -c -mz80 --peep-file peep-rules.txt -DNO_MD_PAD_SUPPORT SMSlib.c
``` 
  If you need to compile the Game Gear version of the lib:
```  
  sdcc -o SMSlib_GG.rel -c -mz80 --peep-file peep-rules.txt -DTARGET_GG SMSlib.c
```

* optionally, if you plan to use PSG music/SFX, place PSGlib.h and PSGlib.rel in your project folder
  (again you could also create a PSGlib folder, place both PSGlib.c and PSGlib.h there and compile it yourself using the following)
```
  sdcc -c -mz80 --peep-file peep-rules.txt PSGlib.c
```

How to use devkitSMS/SMSlib:

* include SMSlib.h in your sources
* compile your program adding the path to SMSlib.rel:
```
  sdcc -mz80 --data-loc 0xC000 --peep-file peep-rules.txt your_program.c ..\SMSlib\SMSlib.rel
```
  note that you should put all .rel files references after the name of the file you're compiling.
  
How to use devkitSMS/PSGlib:

* include PSGlib.h in your sources
* compile your program adding the path to PSGlib.rel:
```
  sdcc -mz80 --data-loc 0xC000 --peep-file peep-rules.txt your_program.c ..\PSGlib\PSGlib.rel ..\SMSlib\SMSlib.rel
```

How to add external data into your ROM:

* use the folder2c utility included, for example.
  It creates a .c source file (with its .h header file) containing one constant data array for each single file found in the specified dir:
```
  folder2c assets data [optional ROM bank #]
```
  this creates data.c and data.h from the files found inside assets subfolder.
Each array will be named from the original filename, replacing spaces, periods and brackets with an underscore (it doesn't convert any other char so please use only alphanumeric chars). For each array there will be a #define into the .h file specifying the size in bytes, and it'll be called [arrayname]_size.
If a numerical third parameter is specified (it's optional), its value will be used in the .h file for an additional #define called [arrayname]_bank for each asset.

How to use more than 48KB in your ROM:

* in your program, use the SMSlib provided 'SMS_mapROMBank(n)' macro to map the bank you need (your code should be restrained to the first 32KB as the last 16KB will be paged out)
* put your data into a separate .c file for each 16KB ROM bank starting from bank2, for example bank2.c, bank3.c etc... (you can use folder2c described above) giving each one a different CONST segment name. I suggest using BANK# for descriptiveness:
```
  sdcc -c -mz80 --constseg BANK2 bank2.c
  sdcc -c -mz80 --constseg BANK3 bank3.c
```
* compile your program and separately link all the objects together adding a parameter for the linker for each bank (_BANK#) and adding each .rel file to be linked, in ascending order, at the end:
```
  sdcc -c -mz80 --peep-file peep-rules.txt your_program.c
  sdcc -mz80 --data-loc 0xC000 -Wl-b_BANK2=0x8000 -Wl-b_BANK3=0x8000 your_program.rel ..\SMSlib\SMSlib.rel bank2.rel bank3.rel
```

How to build the final .sms file

* use the ihx2sms utility included:
```
  ihx2sms your_program.ihx your_program.sms
```
  The size of the ROM will be a multiple of 16KB. The utility also calculates the ROM checksum for the output file if the SEGA header has been included in your program.
