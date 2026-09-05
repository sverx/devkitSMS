devkitSMS IDE support
=====================

### How to set up your devkitSMS project for official VSCode with C/C++ extension:

* copy into your project folder
  * the folder `ide/.vscode/`
  * the file `.sdcc.h`
* edit `.vscode/c_cpp_properties.json`
  * correct `compilerPath` as needed
  * set an environment variable `DEVKITSMS_BASE=<your devkitSMS folder>`, or just replace all `${env:DEVKITSMS_BASE}`

Forks of VSCode cannot access the C/C++ extension. Instead try installing the clangd extension and following the instructions below.

### How to generate a compilation database for your devkitSMS project:

Example using bear

* copy the file `ide/.bearconfig` into your project folder
* in your new `.bearconfig` correct the compilers path as needed
* add `compile_commands.json` to your project `.gitignore` or equivalent
* generate the compilation database:
```sh
  make clean
  bear -c .bearconfig -- make
```
* regenerate whenever you add a new source file to your project

[Other generation options](https://sarcasm.github.io/notes/dev/compilation-database.html#how-to-generate-a-json-compilation-database)

### How to set up your devkitSMS project for clangd:

* copy files into your project folder:
  * `.clangd`
  * `.sdcc.h`
* have clangd run with arguments: `--query-driver=/usr/bin/sdcc --compile-commands-dir=$YOUR_PROJECT_DIR"`
  * correct `/usr/bin/sdcc` to your compiler path as needed
  * For example in VSCode with clangd extension, copy the folder `ide/.vscode/` into your project folder, or copy the `clangd.arguments` setting from `ide/.vscode/settings.json` into your project's existing `.vscode/settings.json`
