ihx2sms
=======

'broken' ihx converter for multibank SEGA Master System / SEGA Game Gear ROMs

'broken' means that the program it's actually assuming that each new declaration of data at address 0x8000 informs that we have to allocate a new ROM bank in the final binary file

```
ihx2sms [-m file.map] [-pm|-pp] infile.ihx outfile.sms
```

First parameter is the input IHX ([Intel Hex](https://en.wikipedia.org/wiki/Intel_HEX)) file, that is SDCC output. Second parameter is generated output ROM file.

The size of the ROM will be a multiple of 16KB. The utility also calculates the ROM checksum for the output file if the SEGA header has been included in your program.

Optional parameters:

```-m file.map``` : you can specify a map file where the order of segment is listed. You shouldn't need this, unless you're totally sure of what you're doing.

```-pm        ``` : output ROM file will be padded to size that is a multiple of 64KB. So output file size would be either 64KB or 128KB or 192KB or 256KB or 320KB... and so on.

```-pp        ``` : output ROM file will be padded to size that is an exact power of 2. So output file size would be either 16KB or 32KB or 64KB or 128KB or 256KB... and so on.


