assets2banks
============

#####How to use assets2banks

```
  assets2banks <asset folder> [--bank1size=<size in bytes>]
```

Using the assets2bank utility you can create a .c source files (with their .h headers file) containing one constant data array for each single file found in the specified dir.
For example:

```
  assets2banks assets
```

  this creates a set of bank**n**.c and bank**n**.h files (starting from n=2) with data taken from the files found inside assets subfolder.
Each array will be named from the original filename, replacing spaces, periods and brackets with an underscore (it doesn't convert any other char so please use only alphanumeric chars).
For each array there will be a #define into the .h file specifying the size in bytes, and it'll be called [arrayname]_size.
Also, there will be an additional #define called [arrayname]_bank for each asset, so you know in which bank your asset has been placed.

Finally, in case you've got some free space in your ROM lower 32 KB, you can ask assets2banks to create a first bank within the space available.
For example, let's suppose you have more than 8 KB free in your lower 32 KB, you can inform assets2banks of this available space using:

```
  assets2banks assets --bank1size=8192
```

the utility will try to allocate assets within this space too, and this might help to save a whole bank.
In this case, also a bank1.c and bank1.h files are created, and these should be compiled and linked with your other object files with no special instructions to the linker, as in:

```
  sdcc -o output.ihx -mz80 --data-loc 0xC000 -Wl-b_BANK2=0x8000 -Wl-b_BANK3=0x8000 --no-std-crt0 crt0_sms.rel main.rel SMSlib.lib bank1.rel bank2.rel bank3.rel
```

(in this case we're assuming we created bank1.rel, bank2.rel and bank3.rel - they should be placed at the end of the list, in ascending order. Note that there is NO -Wl-b_BANK1=0x8000 whatsoever.)
