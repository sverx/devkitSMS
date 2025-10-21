devkitSMS
=======

a collection of tools and code (with a *very* presumptuous name) for SEGA Master System / SEGA Game Gear / SEGA SG-1000 / SEGA SC-3000 homebrew development using 'C' language (and the SDCC compiler).

### How to code your own programs using devkitSMS:

* download SDCC - version 4.2.0 or newer **required**
* install it ("include files" and "Z80 library" are the only components required, you shouldn't need anything else)
* read its [manual](http://sdcc.sourceforge.net/doc/sdccman.pdf)
* make sure that your installation works - details are at page 20 of the PDF manual
* place `makesms(.exe)` (and/or the legacy `ihx2sms(.exe)` tool if you need it) and `makecvmc(.exe)` from this package into your SDCC `bin` folder
* place `assets2banks.exe` (and/or the legacy `folder2c(.exe)` tool) from this package into your SDCC `bin` folder
  (both are optional as you can use other tools to convert your data assets)
  `assets2banks.py` python source is also provided for those using a Python interpreter)

note: if you're on Linux or on Windows, all binaries are supplied in the Linux or Windows folders. If you're on a Mac, binaries are kindly provided by Carl-Dixon [here](https://github.com/Carl-Dixon/devkitSMS/tree/DevKitSMSMac) - otherwise you can compile the programs yourself from the provided sources.

* for SMS/GG:  place `crt0_sms.rel` from this package in a crt0 folder in your projects root (or directly into your project folder if you prefer doing so)
* for SMS: place `SMSlib.h` and `SMSlib.lib` in a SMSlib folder in your projects root (or directly into your project folder if you prefer doing so)
* for GG: place `SMSlib.h` and `SMSlib_GG.lib` in a SMSlib folder in your projects root (or directly into your project folder if you prefer doing so)
* for SG-1000/SC-3000: place `crt0_sg.rel` from this package in a crt0 folder in your projects root (or directly into your project folder if you prefer doing so)
* for SG-1000/SC-3000: place `SGlib.h` and `SGlib.lib` in a SGlib folder in your projects root (or directly into your project folder if you prefer doing so)
* for ColecoVision: place `crt0_cv.rel` from this package in a crt0 folder in your projects root (or directly into your project folder if you prefer doing so)
* for ColecoVision: place `SGlib.h` and `SGlib_CV.lib` in a SGlib folder in your projects root (or directly into your project folder if you prefer doing so)
* (optional) if you plan to use PSG music/SFX, place `PSGlib.h` and `PSGlib.lib` (or `PSGlib_CV.lib` for ColecoVision) in a PSGlib folder in your projects root (or directly into your project folder if you prefer doing so)
* (optional) if you plan to use MoonBlaster modules for FM music, place `MBMlib.h` and `MBMlib.rel` in a MBMlib folder in your projects root (or directly into your project folder if you prefer doing so)

### How to use devkitSMS/SMSlib to code your own Master System/Game Gear program:

* include `SMSlib.h` in your sources
* compile your program:
```
  sdcc -c -mz80 --peep-file peep-rules.txt your_program.c
```
* link your program with `crt0_sms.rel` and the SMSlib library:
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
* link your program with `crt0_sg.rel` and `SGlib.lib` (a.k.a. "the SG library"):
```
  sdcc -o your_program.ihx -mz80 --no-std-crt0 --data-loc 0xC000 crt0_sg.rel your_program.rel SGlib.lib
```
  note that you should put `crt0_sg.rel` *first*, and you should put `SGlib.lib` *after* your code.
  
### How to load your devkitSMS/SGlib code onto a BASIC IIIa/IIIb cartridge:

Software written with SGlib can be loaded to a BASIC cartridge on the SC-3000.
The BASIC `LOAD` command loads the program over the cassette interface to address 0x9800.
Once loaded, the program can be run with the `CALL &H9800` command.

* Link with `crt0_BASIC.rel` instead of `crt0_sg.rel`, and used `--code-loc 0x90a0` to place the code after the interrupt handler.
```
  sdcc -o your_program.ihx -mz80 --no-std-crt0 --code-loc 0x98a0 --data-loc 0xC000 crt0_BASIC.rel your_program.rel SGlib.lib
```
* Convert the compiler's hex output to binary
```
  objcopy -Iihex -Obinary your_program.ihx your_program.bin
```
* Convert the binary into an SC-3000 formatted audio file using a tool such as [SC-TapeWave](https://github.com/JoppyFurr/SC-TapeWave)
* Note that the program size is limited. BASIC IIIa has 12 KiB of RAM following the load address. BASIC IIIb has 26 KiB.

### How to use devkitSMS/SGlib to code your own ColecoVision program:

* include `SGlib.h` in your sources
* compile your program using the `TARGET_CV` define:
```
  sdcc -c -mz80 -D TARGET_CV your_program.c
```
* link your program with `crt0_cv.rel` and `SGlib_CV.lib` (a.k.a. "the SG library for ColecoVision"):
```
  sdcc -o your_program.ihx -mz80 --no-std-crt0 --code-loc 0x8080 --data-loc 0x6000 crt0_cv.rel your_program.rel SGlib_CV.lib
```
  note that you should put `crt0_cv.rel` *first*, and you should put `SGlib_CV.lib` *after* your code.

### How to use devkitSMS/PSGlib to use PSG audio/SFX in your SMS/GG/SG/SC program:

* include `PSGlib.h` in your sources
* compile your program
```
  sdcc -c -mz80 your_program.c
```
* link your program with proper crt0 and libraries, adding `PSGlib.lib` too after the proper library:
```
  sdcc -o your_program.ihx -mz80 --no-std-crt0 --data-loc 0xC000 crt0_sms.rel your_program.rel SMSlib.lib PSGlib.lib
```

### How to use devkitSMS/PSGlib to use PSG audio/SFX in your ColecoVision program:

* include `PSGlib.h` in your sources
* compile your program using the `TARGET_CV` define:
```
  sdcc -c -mz80 -D TARGET_CV your_program.c
```
* link your program with with `crt0_cv.rel` and `SGlib_CV.lib` and adding the `PSGlib_CV.lib` library too:
```
  sdcc -o your_program.ihx -mz80 --no-std-crt0 --code-loc 0x8080 --data-loc 0x6000 crt0_cv.rel your_program.rel SGlib_CV.lib PSGlib_CV.lib
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

### How to use more than 48 KiB in your SMS/GG/SG/SC ROM ('SEGA ROM paging'):

* in your program, use the SMSlib provided macro `SMS_mapROMBank(n)` (or the SGlib provided macro `SG_mapROMBank(n)`) to map the bank you need. Note that your code should be restrained to the first 32 KiB as the last 16 KiB will be paged out and any code there won't be accessible unless the bank is manually mapped in the address space. If you need more than 32 KiB of code, read the section titled "How to use more than 32 KiB of code in your SMS/GG/SG/SC ROM" at the end of this document.

* put your data into a separate .c file for each 16 KiB ROM bank starting from bank2, for example bank2.c, bank3.c etc... (you can use the assets2banks and/or folder2c tools described above) compiling each one with a different CONST segment name, I suggest using `BANK#` for descriptiveness:
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
  sdcc -o your_program.ihx -mz80 --no-std-crt0 --data-loc 0xC000 -Wl-b_BANK2=0x28000 -Wl-b_BANK3=0x38000 crt0_sms.rel your_program.rel SMSlib.lib bank2.rel bank3.rel
```

### How to build the final .sms/.gg/.sg file:

* use the makesms utility included. Input file is the ihx generated by SDCC linker, the output file is the generated ROM:
```
  makesms your_program.ihx your_program.sms
```
  The size of the ROM will be a multiple of 16 KiB. The utility also calculates the ROM checksum for the output file if the SEGA header has been included in your program.

### How to use more than 32 KiB in your ColecoVision ROM ('MegaCart ROM paging'):

* in your program, use the SGlib provided `SG_mapROMBank(n)` macro to map the bank you need (your code should be restrained to 16 KiB as the second half of address space will be paged out)

* put your data into a separate .c file for each 16 KiB ROM bank starting from bank1, for example bank1.c, bank2.c etc... (you can use the assets2banks and folder2c tools described above) compiling each one with a different CONST segment name, I suggest using `BANK#` for descriptiveness. Note that only 16,320 bytes (16 KiB - 64 bytes) are available in each ROM bank.
```
  sdcc -c -mz80 --constseg BANK1 bank1.c
  sdcc -c -mz80 --constseg BANK2 bank2.c
```
* compile your program:
```
  sdcc -c -mz80 -D TARGET_CV your_program.c
```
* link all the objects together adding a custom parameter for the linker for each bank (`_BANK#`) and adding each .rel file to be linked (*proper* crt0 file goes *always* first) then all the bank#.rel files last, in ascending order:
```
  sdcc -o your_program.ihx -mz80 --no-std-crt0 --code-loc 0x8080 --data-loc 0x6000 -Wl-b_BANK1=0x1C000 -Wl-b_BANK2=0x2C000 crt0_cv.rel your_program.rel SGlib_CV.lib bank1.rel bank2.rel
```

### How to build the final ColecoVision .col/.rom file:

* if you're not using MegaCart ROM paging, you program and data have to fit in 32 KiB. In this case you can use SDCC's `sdobjcopy` for the final step. Your input file is the ihx generated by the SDCC linker and the output file is the generated .col ROM file:
```
sdobjcopy --input-target=ihex --output-target=binary --pad-to 0x10000 your_program.ihx your_program.col
```
* you may wanto to use `--pad-to 0xC000` if your program and data can fit in 16 KiB, or you could use `--pad-to 0xA000` if your program and data can fit in 8 KiB.

* if you're using ROM paging instead, use the included makecvmc converter utility to create a ColecoVision MegaCart ROM. The input file is the ihx generated by SDCC linker, the output file is the generated ROM:
```
  makecvmc your_program.ihx your_program.rom
```
  The size of the final ROM file will be either 128, 256, 512 or 1024 KiB, as these are the only supported sizes for a ColecoVision MegaCart ROM.

### ADVANCED: How to use more than 32 KiB of code in your SMS/GG/SG/SC ROM ('banked code'):

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
* use the `makesms` tool to convert the ihx file into the sms ROM format. Usage is very simple:
```
  makesms your_program.ihx your_program.sms
```

### Third-party libraries:

A few additional third-party libraries are available and can be used with devkitSMS:

* Raphnet's [inlib](https://github.com/raphnet/inlib) - inlib is an input library for SMS supporting standard controllers, Genesis/MegaDrive controllers (3 and 6 buttons), the Light Phaser, and less common controllers such as the Paddle, the Sports Pad, and even the Mega Mouse (a mouse for megadrive) or the SEGA Graphic Board.
* Psidum's [GSlib](https://github.com/sverx/GSLib) - GSlib is a general purpose 8 way scrolling engine for background, supporting 16×16 pixel metatiles (Development of this library is abandoned but I aim to fix any bug that is found. The link brings you to my forked repository that already has a few fixes).
* Joe's [Banjo](https://github.com/joffb/banjo) - a FM+PSG (YM2413+SN76489) sound driver supporting furnace tracker modules.


### Support devkitSMS:

Check [SUPPORT.md](https://github.com/sverx/devkitSMS/blob/master/SUPPORT.md) to see how you can contribute to development.

