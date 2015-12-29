aPLib
======

currently defined functions:

```
void aPLib_depack (void *dest, void *src);               /* depack to RAM */
void aPLib_depack_VRAM (unsigned int dest, void *src);   /* depack directly to VRAM */
```

to compile a VRAM-only version:
```
sdcc -c -mz80 -o aPLib_VRAM.rel -DDEPACK_VRAM_ONLY aPLib.c
```
