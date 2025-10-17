# How to use assets2banks

## assets2banks command line options

```
assets2banks <asset folder> [--firstbank=<number>[,<size>]][--compile][--singleheader[=<filename>]][--exclude=<filename>][--allowsplitting][--banksize=<size>]
```

Using the *assets2banks* utility you can create .c source files and their respective .h header files containing one constant data array for each single file found in the specified asset folder.

* Using `--singleheader` you'll generate a single `assets2banks.h` file (or any name you prefer giving to it) instead of one single .h header file for each bank.

* If you add the `--compile` option to your command line, object files (RELs) will be generated in place of the C source files, so you won't have to compile them yourself.

* If the asset folder contains files you wish *assets2banks* to ignore (for instance, a file called `.gitignore`) you can exclude each file using the command line `--exclude` option.

* Default bank size is 16 KiB, but you can use `--banksize=` if you need a different size.

* Assets and assetgroups bigger than a single bank are also supported using the `--allowsplitting` option.

Example usage:
```
assets2banks assets
```
this creates a set of bank*n*.c and bank*n*.h files (starting from n=2) with data taken from the files found inside *assets* subfolder.

Each array will be named from the original filename, replacing spaces, periods and brackets with an underscore (it doesn't convert any other char so please use only alphanumeric chars).

For each array there will be a `#define` into the .h file specifying the size in bytes, and it'll be called *arrayname*_size.

Also, there will be an additional `#define` called *arrayname*_bank for each asset, so you know in which bank your asset has been placed.

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

Of course you can combine all the options if you wish. For instance this:
```
assets2banks assets --firstbank=6 --singleheader=assets.h --compile
```
creates a set of this creates a set of bank*n*.rel (compiled objects) files starting from n=6 and a *single assets.h* header file.

If an asset (or an assetgroup) is bigger than 16 kB, it's also possible to split that to multiple banks using the `--allowsplitting` option. For example using:
```
assets2banks assets --singleheader=assets.h --compile --allowsplitting
```
if the size of a `bigfile` asset found in the assets folder is more than 16 kB, the program will split it into multiple parts called `bigfile_PART0`, `bigfile_PART1`, etc. and fit the parts in successive banks.

## assets2banks config file options

assets2banks' behavior regarding specific assets can optionally be configured using a config file that should be placed in the very same asset folder.

This **must** be named *assets2banks.cfg* and it can contain what follows:

 * empty lines (will be ignored)
 * comments (begin the comment using the # char on *any* line)

### assets grouping

You can create a group of assets, and the listed assets will be allocated in the same bank. Asset groups are defined using open and close curly braces (`{` and `}`) on their own lines.

For instance:
```
{
some_file.bin
some_other_file.bin
}
```
will make sure that the two listed files will be kept together.

### asset attributes

Assets attributes are instruction for handling a specific asset and always refer to the last previously listed asset.

All asset attributes starts with `:` and there's at most *one* attribute for each new line.

Here are all the possible attributes.

### asset attribute :format

The :format attribute makes it possible to define the asset data as an array of `unsigned int` instead of the regular `unsigned char` type.

Example:
```
background_tilemap.bin
:format unsigned int
```
will create a `const unsigned int background_tilemap_bin[]` array instead of a `const unsigned char background_tilemap_bin[]` array.

### asset attribute :overwrite

The :overwrite attribute can be used to replace elements in the asset.

By using:
```
:overwrite <start> <length> <value> [<value>[...]]
```
*length* array elements from *start* will be replaced with the listed value(s), repeating them if needed. Accept decimal values, or 0x-prefixed hex values.

There's also a shorter form to replace a single element.
```
:overwrite <index> <value>
```
the element at *index* will be replaced with the provided value.

Is it also possible to specify more than one overwrite for a single asset, of course they are always relative to the last previously listed asset.

Example usage:
```
sprite_palette.bin
:overwrite 0 0x00
```
The first value in the sprite_palette_bin array will be replaced by _0x00_.

### asset attribute :modify

The :modify attribute can be used to update elements in the asset.

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

Example usage:
```
my_tilemap.bin
:format unsigned int
:modify OR 20 4 0x1000
```
The 20th, 21th, 22th, 23th value in the my_tilemap_bin **const unsigned int** array will be ORed with _0x1000_.

### asset attribute :header (or :prepend)

The :header attribute can be used to prepend (add leading values) to an array.

By using:
```
:header <value> [<value>[...]]
```
the provided value(s) will be placed *before* the asset's own data. Accept decimal values, or 0x-prefixed hex values.

Example usage:
```
headerless_data.bin
:header 0x38 0x21 0x00 0x11
```

### asset attribute :append (or :footer)

The :append attribute can be used to append (add trailing values) to an array.

By using:
```
:append <value> [<value>[...]]
```
the provided value(s) will be placed *after* the asset's data.

### asset attribute :text (or :data)

The :text attribute can be used when the asset file is a text file containing data, instead of a being a binary file, and thus needs to be parsed.

Example usage:
```
sin_x_LUT.txt
:text
```

Inside the text file the values can be expressed in decimal or in 0x-prefixed hex form, and negative numbers are accepted down to -128 for char arrays and -32768 for int arrays.

Values can be separated by any number of spaces, tabs, newlines, commas, semicolumns or brakets of any kind. Comments are supported, starting with a # sign. An example of a valid text file could be:
```
# this is some data in text form

-6 -5 -0x0004 ( -0x03 -0x2 -1    # hex and/or negative is also supported
{0, 1, 2, 3, 4}

[5  6  7  8  9]                  # note that no specific separators or brakets are needed
10,,,11,,,12,,,13

[14] 15 [16] 17 ]]               
018; 019; {020}; (021);          # leading zeroes in decimal values are accepted too
0x16, 0x17, 0x18         0x19
```
In this example, the text file is turned into an array containing every value from -6 to 25. Note how there's no specific format requirement.

### asset attribute :alias

The :alias attribute can be used to give a different name to the array created from the data of an asset.

By using:
```
:alias <new name>
```
the provided name will be used instead.

Example usage:
```
sprites_final_Final_FINAL.bin
:alias sprites
```
will generate an array named _sprites_ even if the asset name is something else.

### asset attribute :exclude (or :ignore)

The :exclude attribute can be used to skip importing some file that's in the asset folder

Example usage:
```
debug_data_test.bin
:ignore
```
no array will be generated for the file *debug_data_test.bin*.

### asset attribute :segment

The :segment attribute can be used to import only part(s) of a *binary* asset, instead of importing the whole file (which is the default behavior when no :segment attribute is present).
More than one :segment attribute can be used on a single assets, if needed.

By using:
```
:segment <length>
```
you can import only _length_ bytes from the file. A negative value means file_size-length.

By using:
```
:segment <length> skip <count>
```
you can import only _length_ bytes from the file, skipping the first _count_ bytes. If _length_ is omitted, the whole file is imported after skipping the first _count_ bytes.

Example usage:
```
blob1.bin
:segment 1024

blob2.bin
:segment 128 skip 256

blob3.bin
:segment skip 32
```
will import only the first 1024 bytes from _blob1.bin_, then 128 bytes from _blob2.bin_ after skipping the first 256 bytes, and the whole _blob3.bin_ file dropping the first 32 bytes.

### short but complete example of all the features

Here's an example of a short (but feature complete) configuration file:
```
# assets2banks.cfg example file
# this is a comment

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
:alias BGpalette
# the zero-indexed element of the array will be set to 0x00, and the array will be called BGpalette instead of asset2__palette__bin

asset2 (tiles).psgcompr
asset3 (tilemap).bin 
:format unsigned int
:modify OR 0 16 0x1000
# 'asset3 (tilemap).bin' first 16 elements (starting from 0) will be ORed with 0x1000 bit mask

my_notes.txt  # I keep track of some important information in here!
:ignore
# 'my_notes.txt' will be ignored

asset4.bin
:format unsigned int
:overwrite 32 128 0x010a 0x01FF 0x0123
# 'asset4.bin' 128 elements starting from array[32] will be set to the values listed
# (values list will be reitered until 128 array elements are replaced)

random_values_table.txt
:text
# this is a text file that will be parsed into an array

somedata.bin
:segment skip 32
:modify AND 0xFE
:header 0xF5 0xC9
:append 0x00
# takes 'somedata.bin' skipping the first 32 bytes from it, clears the last bit of each element, prepends a 2 bytes header 0xF5 0xC9 and then appends one 0x00 byte to it
```
Of course all the assets in the asset folder which are not mentioned in the config file will be handled as ungrouped and without special attributes.

A config file is absolutely not mandatory, and if you don't need grouping and/or special attributes there's no advantage in having one.

## advanced usage

In case you've got some free space in your ROM lower 32 KB, you can ask assets2banks to create a data block which can fit up to the whole space available.

For example, let's suppose you have more than 8 KB free in your lower 32 KB, you can make assets2banks use this available space for bank1:
```
assets2banks assets --firstbank=1,8192 --compile
```
the utility will try to allocate assets within this space too, and this might help to save a whole bank.

When using this option, also a *bank1.c* (and possibly a *bank1.h*) files will be created, and these should be compiled and linked with your other object files with a slightly different instruction to the linker, because we want the BANK1 segment to end up together with our code so we need to use `-Wl-b_BANK1=0x0000` as in this example:
```
sdcc -o output.ihx -mz80 --data-loc 0xC000 -Wl-b_BANK1=0x0000 -Wl-b_BANK2=0x8000 -Wl-b_BANK3=0x8000 --no-std-crt0 crt0_sms.rel main.rel SMSlib.lib bank1.rel bank2.rel bank3.rel
```
(we assumed we had created *bank1.rel*, *bank2.rel* and *bank3.rel* - these should be placed at the end of the object list, in ascending order)

Alternatively, one could also compile the *bank1.c* using SDCC (instead of using assets2banks' `--compile` option) without assigning it any segment name, in that case the above linker call becomes:
```
sdcc -o output.ihx -mz80 --data-loc 0xC000 -Wl-b_BANK2=0x8000 -Wl-b_BANK3=0x8000 --no-std-crt0 crt0_sms.rel main.rel SMSlib.lib bank1.rel bank2.rel bank3.rel
```
(note how there are no specific instructions to the linker regarding `BANK1` because that segment name wouldn't in fact exist at all)

Finally, this option can even be used to allocate an asset or assetgroup bigger than 16 KB, if the available space is enough, or it could be even used to keep all your assets together, as for instance if you want to create a 32 KiB only ROM. You can thus use this:
```
assets2banks assets --firstbank=1,32768  --compile
```
and the linker call would then be:
```
sdcc -o output.ihx -mz80 --data-loc 0xC000 -Wl-b_BANK1=0x0000 --no-std-crt0 crt0_sms.rel main.rel SMSlib.lib bank1.rel
```
or, alternatively, you could compile the generated *bank1.c* using SDCC (instead of using assets2banks' `--compile` option) without assigning it any segment name, and simply link using this:
```
sdcc -o output.ihx -mz80 --data-loc 0xC000 --no-std-crt0 crt0_sms.rel main.rel SMSlib.lib bank1.rel
```

