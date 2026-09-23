makecvmc
========

IHX to BINary converter for ColecoVision MegaCart ROMs

```
makecvmc [options] infile.ihx outfile.rom
```

First parameter is the input IHX ([Intel Hex](https://en.wikipedia.org/wiki/Intel_HEX)) file, that is the SDCC output. Second parameter is the generated output ROM file.

The size of the ROM file will either be 64¹, 128, 256, 512 or 1024 KiB.

[¹] 64 KiB MegaCart ROMs are supported by most emulators. If needed, a 128 KiB MegaCart ROM can be created by appending the 64 KiB ROM twice.

Optional parameters:

```-emptyfill value``` : Specify the byte value to be used for filling unused areas of the ROM. Default: 0x00
