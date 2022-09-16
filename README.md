devkitSMS
=======

a collection of tools and code (with a *very* presumptuous name) for SEGA Master System / SEGA Game Gear / SEGA SG-1000 / SEGA SC-3000 homebrew development using 'C' language (and the SDCC compiler).

### How to code your own programs using devkitSMS:

* download SDCC - version 4.2.0 or newer **required**
* install it ("include files" and "Z80 library" are the only components required, you shouldn't need anything else)
* read its [manual](http://sdcc.sourceforge.net/doc/sdccman.pdf)
* make sure that your installation works - details are at page 20 of the PDF manual
* place `ihx2sms(.exe)` and `makesms(.exe)` from this package into your SDCC `bin` folder
  (if you're on Linux or on Windows, both programs are supplied in the Linux or Windows folders, otherwise you can compile ihx2sms and makesms youself from the provided sources)
* place `assets2banks.exe` (and/or the legacy `folder2c(.exe)` tool) from this package into your SDCC `bin` folder
  (both are optional as you can use other tools to convert your data assets. If you're not on Linux or Windows please compile `folder2c.c` yourself from the sources. `assets2banks.py` python source is also provided for Linux users and for those using a Python interpreter)
* for SMS/GG:  place `crt0_sms.rel` from this package in a crt0 folder in your projects root (or directly into your project folder if you prefer doing so)
* for SMS: place `SMSlib.h` in a SMSlib folder in your projects root (or directly into your project folder if you prefer doing so) and `SMSlib.lib` in SDCC `lib/z80` folder
* for GG: place `SMSlib.h` in a SMSlib folder in your projects root (or directly into your project folder if you prefer doing so) and `SMSlib_GG.lib` in SDCC `lib/z80` folder
* for SG-1000/SC-3000: place `crt0_sg.rel` from this package in a crt0 folder in your projects root (or directly into your project folder if you prefer doing so)
* for SG-1000/SC-3000: place `SGlib.h` and `SGlib.rel` in a SGlib folder in your projects root (or directly into your project folder if you prefer doing so)
* (optional) if you plan to use PSG music/SFX, place `PSGlib.h` and `PSGlib.rel` in a PSGlib folder in your projects root (or directly into your project folder if you prefer doing so)
* (optional) if you plan to use MoonBlaster modules for FM music, place `MBMlib.h` and `MBMlib.rel` in a MBMlib folder in your projects root (or directly into your project folder if you prefer doing so)

### How to use devkitSMS/SMSlib to code your own SMS/GG program:

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
  
### How to use devkitSMS/SGlib to code your own SG-1000/SC-3000 program:

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
  
### How to use devkitSMS/PSGlib to use PSG audio/SFX in your SMS/GG/SG/SC program:

* include `PSGlib.h` in your sources
* compile your program
```
  sdcc -c -mz80 your_program.c
```
* link your program with proper crt0 and libraries, adding `PSGlib.rel` too after the proper library:
```
  sdcc -o your_program.ihx -mz80 --no-std-crt0 --data-loc 0xC000 crt0_sms.rel your_program.rel SMSlib.lib PSGlib.rel
```

### How to add external data into your ROM:

* use the assets2banks utility included, for example.
  This is **the suggested way**. Read its [documentation](https://github.com/sverx/devkitSMS/blob/master/assets2banks/README.md)

* alternatively, you use the folder2c utility included
  It creates a .c source file (with its .h header file) containing one constant data array for each single file found in the specified dir:
```
  folder2c assets data [<ROM bank #>]
```
  this creates data.c and data.h from the files found inside assets subfolder.
Each array will be named from the original filename, replacing spaces, periods and brackets with an underscore (it doesn't convert any other char so please use only alphanumeric chars). For each array there will be a #define into the .h file specifying the size in bytes, and it'll be called [arrayname]_size.
If a numerical third parameter is specified (it's optional), its value will be used in the .h file for an additional #define called [arrayname]_bank for each asset.

### How to use more than 48KiB in your ROM ('ROM paging'):

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

### How to build the final .sms/.gg/.sg file

* use the ihx2sms utility included. Input file is the ihx generated by SDCC linker, the output file is the generated ROM:
```
  ihx2sms your_program.ihx your_program.sms
```
  The size of the ROM will be a multiple of 16KB. The utility also calculates the ROM checksum for the output file if the SEGA header has been included in your program.

### ADVANCED: How to use more than 32KiB of code in your SMS/GG ROM ('banked code'):

SDCC 4.x supports banked code and this means that it's now possible to page in and out code in a transparent manner. To achieve this, banked code should be written in separate source files that have to get allocated in separate code banks. Upon calling a banked function the code will trigger a slot-1 bank change (using trampolines that are 'hidden' in bank 0 code) and will restore the previous bank upon returning from called function. This means that only code in bank 0 won't ever be paged out, but that means that total code won't be restricted to the 32 KiB maximum size.

You need to:
* put banked code in separate source files, marking banked functions as such using the `__banked` SDCC keyword, as in

```
void some_function (void) __banked {
  //  your code here 
}
```
* compile each banked code source file into a specific separate code segment, for instance as in:
```
  sdcc -c -mz80 --codeseg BANK1 banked_code_1.c
  sdcc -c -mz80 --codeseg BANK2 banked_code_2.c
```
* use the provided `crt0_sms.rel` as the *first* module in your linker call
* instruct the linker to place all the banked code at address 0x4000 + the _virtual_ address of the bank, which is banknumber×2¹⁶ (so code segment BANK1 goes at 0x14000, code segment BANK2 goes at 0x24000 and so on...) and the compiled banked code modules in ascending order as in:
```
  sdcc -o your_program.ihx -mz80 --no-std-crt0 --data-loc 0xC000 -Wl-b_BANK1=0x14000 -Wl-b_BANK2=0x24000 crt0_sms.rel SMSlib.lib main.rel banked_code_1.rel banked_code_2.rel
```
* use the newer `makesms` tool to convert the ihx output to the sms file _instead_ of the `ihx2sms` tool, as this works in a different way. Usage is very similar:
```
  makesms your_program.ihx your_program.sms
```


