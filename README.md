devkitSMS
=========

a collection of tools and code (with a *very* presumptuous name) for SEGA Master System / SEGA Game Gear / SEGA SG-1000 homebrew development using 'C' language (and the SDCC compiler).

#####How to code your own programs using devkitSMS:

* download SDCC - version 3.6.9 or newer **required** - get it [here](http://sdcc.sourceforge.net/) - snapshots [here](http://sdcc.sourceforge.net/snap.php) 
* install it ("include files" and "Z80 library" are the only components required, you shouldn't need anything else)
* read its [manual](http://sdcc.sourceforge.net/doc/sdccman.pdf)
* make sure that your installation works - details are at page 20 of the PDF manual
* place `ihx2sms.exe` from this package into your SDCC `bin` folder
  (if you're not on Windows please compile ihx2sms youself from the sources)
* place `assets2banks.exe` and/or `folder2c.exe` from this package into your SDCC `bin` folder
  (both are optional as you can use other tools to convert your data assets. If you're not on Windows please compile `folder2c.c` yourself from the sources. `assets2banks.py` python source is also provided)
* for SMS/GG:  place `crt0_sms.rel` from this package into your project folder (or a crt0 folder on your projects root)
* for SMS: place `SMSlib.h` in your project folder and `SMSlib.lib` in SDCC `lib/z80` folder
* for GG:  place `SMSlib.h` in your project folder and `SMSlib_GG.lib` in SDCC `lib/z80` folder
* for SG-1000: place `crt0_sg.rel` from this package into your project folder (or a crt0 folder on your projects root)
* for SG-1000: place `SGlib.h` and `SGlib.rel` in your project folder (or a SGlib folder on your projects root)
* optionally, if you plan to use PSG music/SFX, place `PSGlib.h` and `PSGlib.rel` in your project folder

#####How to use devkitSMS/SMSlib to code your own SMS/GG program:

* include `SMSlib.h` in your sources
* compile your program:
```
  sdcc -c -mz80 --peep-file peep-rules.txt your_program.c
```
* link your program with `crt0_sms.rel` and the library:
```
  sdcc -o your_program.ihx -mz80 --no-std-crt0 --data-loc 0xC000 crt0_sms.rel your_program.rel SMSlib.lib
```
  note that you should put `crt0_sms.rel` *first*, and you should put the library *after* your code.
  
#####How to use devkitSMS/SGlib to code your own SG program:

* include `SGlib.h` in your sources
* compile your program:
```
  sdcc -c -mz80 your_program.c
```
* link your program with `crt0_sg.rel` and `SGlib.rel` (a.k.a. "the SG library"):
```
  sdcc -o your_program.ihx -mz80 --no-std-crt0 --data-loc 0xC000 crt0_sg.rel your_program.rel SGlib.rel
```
  note that you should put `crt0_sg.rel` *first*, and you should put `SGlib.rel` *after* your code.
  
#####How to use devkitSMS/PSGlib to use PSG audio/SFX in your SMS/GG/SG program:

* include `PSGlib.h` in your sources
* compile your program
```
  sdcc -c -mz80 your_program.c
```
* link your program with proper crt0 and libraries, adding `PSGlib.rel` too after the proper library:
```
  sdcc -o your_program.ihx -mz80 --no-std-crt0 --data-loc 0xC000 crt0_sms.rel your_program.rel SMSlib.lib PSGlib.rel
```

#####How to add external data into your ROM:

* use the assets2banks utility included, for example.
  This is **the suggested way**. Read its [documentation](https://github.com/sverx/devkitSMS/blob/master/assets2banks/README.md)

* alternatively, you use the folder2c utility included
  It creates a .c source file (with its .h header file) containing one constant data array for each single file found in the specified dir:
```
  folder2c assets data [optional ROM bank #]
```
  this creates data.c and data.h from the files found inside assets subfolder.
Each array will be named from the original filename, replacing spaces, periods and brackets with an underscore (it doesn't convert any other char so please use only alphanumeric chars). For each array there will be a #define into the .h file specifying the size in bytes, and it'll be called [arrayname]_size.
If a numerical third parameter is specified (it's optional), its value will be used in the .h file for an additional #define called [arrayname]_bank for each asset.

#####How to use more than 48KB in your ROM ('ROM paging'):

* in your program, use the SMSlib provided `SMS_mapROMBank(n)` macro to map the bank you need (your code should be restrained to the first 32KB as the last 16KB will be paged out)
* put your data into a separate .c file for each 16KB ROM bank starting from bank2, for example bank2.c, bank3.c etc... (you can use the assets2banks and folder2c tools described above) compiling each one with a different CONST segment name, I suggest using `BANK#` for descriptiveness:
```
  sdcc -c -mz80 --constseg BANK2 bank2.c
  sdcc -c -mz80 --constseg BANK3 bank3.c
```
* compile your program:
```
  sdcc -c -mz80 --peep-file peep-rules.txt your_program.c
```
* link all the objects together adding a parameter for the linker for each bank (`_BANK#`) and adding each .rel file to be linked (*proper* crt0 file goes *always* first) then all the bank#.rel files last, in ascending order:
```
  sdcc -o your_program.ihx -mz80 --no-std-crt0 --data-loc 0xC000 -Wl-b_BANK2=0x8000 -Wl-b_BANK3=0x8000 crt0_sms.rel your_program.rel SMSlib.lib bank2.rel bank3.rel
```

#####How to build the final .sms/.gg/.sg file

* use the ihx2sms utility included. Input file is the ihx generated by SDCC linker, the output file is the generated ROM:
```
  ihx2sms your_program.ihx your_program.sms
```
  The size of the ROM will be a multiple of 16KB. The utility also calculates the ROM checksum for the output file if the SEGA header has been included in your program.
