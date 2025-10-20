makecvmc
========

IHX converter to BINary ColecoVision MegaCart ROMs

```
makecvmc [options] infile.ihx outfile.rom
```

First parameter is the input IHX ([Intel Hex](https://en.wikipedia.org/wiki/Intel_HEX)) file, that is SDCC output. Second parameter is generated output ROM file.

The size of the ROM file will either be 128, 256, 512 or 1024 KiB. No other ROM size is supported by the ColecoVision Mega Cart so no file of other size will be generated.

Optional parameters:

```-emptyfill value``` : Specify the byte value to be used for filling unused areas of the ROM. Default: 0x00

