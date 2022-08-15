## assets2banks

### How to use assets2banks

```
assets2banks <asset folder> [--firstbank=<number>[,<size>]][--compile][--singleheader[=<filename>]][--exclude=<filename>][--allowsplitting]
```

Using the assets2banks utility you can create .c source files and their respective .h header files containing one constant data array for each single file found in the specified asset folder.
(Adding --singleheader you'll generate a single assets2banks.h file (or any name you prefer giving to it) instead of one single .h header file for each bank)
Also, if you add the --compile option to your command line, object files (RELs) will be generated in place of the C source files, so you won't have to compile them yourself.

If the asset folder contains files you wish assets2banks to ignore (for instance, a file called .gitignore) you can exclude each file using the --exclude option.

Assets and assetgroups bigger than a single bank (16 kB) are now supported using the --allowsplitting option.

Example usage:

```
assets2banks assets
```

this creates a set of bank*n*.c and bank*n*.h files (starting from n=2) with data taken from the files found inside *assets* subfolder.
Each array will be named from the original filename, replacing spaces, periods and brackets with an underscore (it doesn't convert any other char so please use only alphanumeric chars).

For each array there will be a #define into the .h file specifying the size in bytes, and it'll be called [arrayname]_size.
Also, there will be an additional #define called [arrayname]_bank for each asset, so you know in which bank your asset has been placed.

Generating RELs instead of C source files:

```
assets2banks assets --compile
```

this creates a set of bank*n*.rel and bank*n*.h files (starting from n=2) with data taken from the files found inside *assets* subfolder.
The data in the object files will be inside a BANK*n* segment, ready as you needed them for the linking phase.

Generating a single header file instead of separate header files for each bank:

```
assets2banks assets --singleheader
```
this creates a set of bank*n*.rel files (starting from n=2) and a *single* assets2banks.h header file with data taken from the files found inside *assets* subfolder.

In case you need the banks count to start from a different number (default is 2) for instance when you need to allocate other code banks before the data banks, you can do that using:

```
assets2banks assets --firstbank=6
```

this creates a set of bank*n*.c and bank*n*.h files starting from n=6 as specified.

Finally, in case you've got some free space in your ROM lower 32 KB, you can ask assets2banks to create a data block which can fit up to the whole space available.
For example, let's suppose you have more than 8 KB free in your lower 32 KB, you can make assets2banks use this available space for bank1:

```
assets2banks assets --firstbank=1,8192
```

the utility will try to allocate assets within this space too, and this might help to save a whole bank.
When using this option, also a bank1.c and bank1.h files will be created, and these should be compiled and linked with your other object files with no special instructions to the linker, as in this example:

```
sdcc -o output.ihx -mz80 --data-loc 0xC000 -Wl-b_BANK2=0x8000 -Wl-b_BANK3=0x8000 --no-std-crt0 crt0_sms.rel main.rel SMSlib.lib bank1.rel bank2.rel bank3.rel
```

(we assumed we had bank1.rel, bank2.rel and bank3.rel - then they should be placed at the end of the object list, in ascending order. Note that there is no '-Wl-b_BANK1=0x8000'.)

Finally, this option can even be used to allocate an asset bigger than 16 KB, of course only if the available space is enough.

Of course you can combine all the options if you wish. For instance this:

```
assets2banks assets --firstbank=6 --singleheader=assets.h --compile
```

creates a set of this creates a set of bank*n*.rel (compiled objects) files starting from n=6 and a *single assets.h* header file.

If an asset (or an assetgroup) is bigger than 16 kB, it's also possible to split that to multiple banks using the --allowsplitting option. For example using:

```
assets2banks assets --singleheader=assets.h --compile --allowsplitting
```

if the size of the 'bigfile' asset found in the assets folder is more than 16 kB, the program will split it into multiple parts called `bigfile_PART0`, `bigfile_PART1`, etc. and fit the parts in successive banks.

Beside that, assets2banks behavior regarding specific assets can optionally be configured using a config file that should be placed in the very same asset folder.

This **must** be named *assets2banks.cfg* and it can define/contain:
 * grouping of assets

Listed assets will be allocated in the same bank (groups are defined using open and close curly braces "{" and "}")

 * asset format attribute

(all asset attributes starts with ':' and there's at most one attribute for each new line. They always refer to the last previously listed asset)

For instance:
```
:format unsigned int
```
will create a **const unsigned int** array instead of a **const unsigned char** array. Note that the format attribute, when present, should always be the **first** attribute of an asset.

 * overwrite array elements

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
the element at *index* will be replaced with the provided value.

Is it also possible to specify more than one overwrite for a single asset, of course they are relative to the last previously listed asset.

 * modify array elements

By using:
```
:modify <action> <start> <length> <value> [<value>[...]]
```
*length* array elements from *start* will be modified using the listed action and value(s), repeating them if needed.
Accept decimal values, or 0x-prefixed hex values. Valid actions are 'add', 'and', 'or' and 'xor'.
There are also two shorter forms. By using:
```
:modify <action> <index> <value>
```
the element at *index* will be modified using the provided action and value.

Also, by using:
```
:modify <action> <value>
```
all the array elements will be modified using the provided action and value.

Is it also possible to specify more than one modify attribute for a single asset.

 * add leading values (an header) to your asset

By using:
```
:header <value> [<value>[...]]
```
the provided value(s) will be placed *before* the asset's data.

 * add trailing values (append data) to your asset

By using:
```
:append <value> [<value>[...]]
```
the provided value(s) will be placed *after* the asset's data.

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
:modify OR 0 16 0x1000
# 'asset3 (tilemap).bin' first 16 elements (starting from 0) will be ORed with 0x1000 bit mask

asset4.bin
:format unsigned int
:overwrite 32 128 0x010a 0x01FF 0x0123
# 'asset4.bin' 128 elements starting from array[32] will be set to the values listed
# (values list will be reitered until 128 array elements are replaced)

somedata.bin
:modify AND 0xFE
:header 0xF5 0xC9
:append 0x00
# clears last bit of each element, prepends a 2 bytes header to your 'somedata.bin' asset, then append one final byte to it
```

Of course all the assets in the asset folder which are not mentioned in the config file will be handled as ungrouped and without special attributes.
A config file is absolutely not mandatory, and if you don't need grouping and/or special attributes there's no advantage in having one.
