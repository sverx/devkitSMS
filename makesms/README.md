makesms
=======

IHX converter to BINary Master System / SEGA Game Gear ROMs

```
makesms [-pm|-pp] infile.ihx outfile.sms
```

First parameter is the input IHX ([Intel Hex](https://en.wikipedia.org/wiki/Intel_HEX)) file, that is SDCC output. Second parameter is generated output ROM file.

The size of the ROM will be a multiple of 16 KiB. The utility also calculates the ROM checksum for the output file if the SEGA header has been included in your program.

Optional parameters:

```-pm        ``` : output ROM file will be padded to size that is a multiple of 64 KiB. So output file size would be either 64 KiB or 128 KiB or 192 KiB or 256 KiB or 320 KiB... and so on.

```-pp        ``` : output ROM file will be padded to size that is an exact power of 2. So output file size would be either 16 KiB or 32 KiB or 64 KiB or 128 KiB or 256 KiB... and so on.

