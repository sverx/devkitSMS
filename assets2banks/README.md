###assets2banks

##How to use assets2banks

```
assets2banks <asset folder> [--bank1size=<size in bytes>]
```

Using the assets2banks utility you can create .c source files (and their .h header files) containing one constant data array for each single file found in the specified asset folder.
For example:

```
assets2banks assets
```

  this creates a set of bank*n*.c and bank*n*.h files (starting from n=2) with data taken from the files found inside *assets* subfolder.
Each array will be named from the original filename, replacing spaces, periods and brackets with an underscore (it doesn't convert any other char so please use only alphanumeric chars).

For each array there will be a #define into the .h file specifying the size in bytes, and it'll be called [arrayname]_size.
Also, there will be an additional #define called [arrayname]_bank for each asset, so you know in which bank your asset has been placed.

Finally, in case you've got some free space in your ROM lower 32 KB, you can ask assets2banks to create a data block which can fit up to the whole space available.
For example, let's suppose you have more than 8 KB free in your lower 32 KB, you can inform assets2banks of this available space using:

```
assets2banks assets --bank1size=8192
```

the utility will try to allocate assets within this space too, and this might help to save a whole bank.
When using this option, also a bank1.c and bank1.h files will be created, and these should be compiled and linked with your other object files with no special instructions to the linker, as in this example:

```
sdcc -o output.ihx -mz80 --data-loc 0xC000 -Wl-b_BANK2=0x8000 -Wl-b_BANK3=0x8000 --no-std-crt0 crt0_sms.rel main.rel SMSlib.lib bank1.rel bank2.rel bank3.rel
```

(we assumed we had bank1.rel, bank2.rel and bank3.rel - then they should be placed at the end of the object list, in ascending order. Note that there is no '-Wl-b_BANK1=0x8000'.)

Finally, this option can even be used to allocate an asset bigger than 16 KB, of course only if the available space is enough.

Beside that, assets2banks behavior regarding specific assets can optionally be configured using a config file that should be placed in the very same asset folder.

This **must** be named *assets2banks.cfg* and it can define/contain:
 * grouping of assets.
Listed assets will be allocated in the same bank (groups are defined using open and close curly braces "{" and "}")
 * asset format attribute.
For instance:
```
:format unsigned int
```
will create a **const unsigned int** array instead of a **const unsigned char** array.
(Asset attributes starts with ':' and there's at most one attribute for each new line. They are relative to the last previously listed asset)
 * overwrite array elements.
By using:
```
:overwrite <start> <length> <value> [<value>[...]]
```
*length* array elements from *start* will be replaced with the listed value(s), repeating them if needed.
Accept decimal values, or 0x-prefixed hex values.
There's also a shorter form. By using:
```
:overwrite <index> <value>
```
the element element at *index* will be replaced with the provided value.

Is it also possible to specify more than one overwrite for a single asset, of course they are relative to the last previously listed asset.
 * line comments (using the # as first char on the line)
 * (empty lines are ignored)

Here's an example of a short (but feature complete) configuration file:

```
# line comment and an empty line next

# next line starts an asset group:
{ 
asset1 (tiles).bin 
asset1 (tilemap).bin 
:format unsigned int 
# 'asset1 (tilemap).bin' will be a const unsigned int array
}
# asset group complete

# other assets (ungrouped) 
asset2 (palette).bin
:overwrite 0 0x00
# 'asset2 (palette).bin' the zero-indexed element of the array will be set to 0x00

asset2 (tiles).psgcompr
asset3 (tilemap).bin 
:format unsigned int

asset4.bin
:format unsigned int
:overwrite 32 128 0x010a 0x01FF 0x0123
# 'asset4.bin' 128 elements starting from array[32] will be set to the values listed
# (values list will be reitered until 128 array elements are replaced)
```

Of course all the assets in the asset folder which are not mentioned in the config file will be handled as ungrouped and without special attributes.
A config file is absolutely not mandatory, and if you don't need grouping and/or special attributes there's no advantage in having one.
