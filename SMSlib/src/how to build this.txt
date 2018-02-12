set OPT=--max-allocs-per-node 100000

sdcc -o SMSlib.rel -c -mz80 %OPT% --peep-file peep-rules.txt --peep-asm --reserve-regs-iy SMSlib.c
@if %errorlevel% NEQ 0 goto :EOF

sdcc -o SMSlib_GG.rel -c -mz80 %OPT% -DTARGET_GG --peep-file peep-rules.txt --peep-asm --reserve-regs-iy SMSlib.c
@if %errorlevel% NEQ 0 goto :EOF

sdcc -o SMSlib_sprite.rel -c -mz80 %OPT% --peep-file peep-rules.txt --peep-asm SMSlib_sprite.c
@if %errorlevel% NEQ 0 goto :EOF

sdcc -o SMSlib_twosprites.rel -c -mz80 %OPT% --peep-file peep-rules.txt --peep-asm SMSlib_twosprites.c
@if %errorlevel% NEQ 0 goto :EOF

sdcc -o SMSlib_paletteAdv.rel -c -mz80 %OPT% --peep-file peep-rules.txt --peep-asm SMSlib_paletteAdv.c
@if %errorlevel% NEQ 0 goto :EOF

sdcc -o SMSlib_spriteAdv.rel -c -mz80 %OPT% --peep-file peep-rules.txt --peep-asm SMSlib_spriteAdv.c
@if %errorlevel% NEQ 0 goto :EOF

sdcc -o SMSlib_spriteClip.rel -c -mz80 %OPT% --peep-file peep-rules.txt --peep-asm SMSlib_spriteClip.c
@if %errorlevel% NEQ 0 goto :EOF

sdcc -o SMSlib_spriteClip_GG.rel -c -mz80 %OPT% -DTARGET_GG --peep-file peep-rules.txt --peep-asm SMSlib_spriteClip.c
@if %errorlevel% NEQ 0 goto :EOF

sdcc -o SMSlib_loadTileMapArea.rel -c -mz80 %OPT% --peep-file peep-rules.txt --peep-asm SMSlib_loadTileMapArea.c
@if %errorlevel% NEQ 0 goto :EOF

sdcc -o SMSlib_loadTiles.rel -c -mz80 %OPT% --peep-file peep-rules.txt --peep-asm SMSlib_loadTiles.c
@if %errorlevel% NEQ 0 goto :EOF

sdcc -o SMSlib_load1bppTiles.rel -c -mz80 %OPT% --peep-file peep-rules.txt --peep-asm SMSlib_load1bppTiles.c
@if %errorlevel% NEQ 0 goto :EOF

sdcc -o SMSlib_STMcomp.rel -c -mz80 %OPT% --peep-file peep-rules.txt SMSlib_STMcomp.c
@if %errorlevel% NEQ 0 goto :EOF

sdcc -o SMSlib_VRAMmemcpy.rel -c -mz80 %OPT% --peep-file peep-rules.txt --peep-asm SMSlib_VRAMmemcpy.c
@if %errorlevel% NEQ 0 goto :EOF

sdcc -o SMSlib_VRAMmemset.rel -c -mz80 %OPT% --peep-file peep-rules.txt --peep-asm SMSlib_VRAMmemset.c
@if %errorlevel% NEQ 0 goto :EOF

sdcc -o SMSlib_UNSAFE.rel -c -mz80 %OPT% --peep-file peep-rules.txt --peep-asm SMSlib_UNSAFE.c
@if %errorlevel% NEQ 0 goto :EOF

sdcc -o SMSlib_PSGaiden.rel -c -mz80 %OPT% --peep-file peep-rules.txt SMSlib_PSGaiden.c
@if %errorlevel% NEQ 0 goto :EOF

sdcc -o SMSlib_textrenderer.rel -c -mz80 %OPT% --peep-file peep-rules.txt --peep-asm SMSlib_textrenderer.c
@if %errorlevel% NEQ 0 goto :EOF

sdcc -o SMSlib_autotext.rel -c -mz80 %OPT% --peep-file peep-rules.txt --peep-asm SMSlib_autotext.c
@if %errorlevel% NEQ 0 goto :EOF

sdcc -o SMSlib_autotext_GG.rel -c -mz80 %OPT% -DTARGET_GG --peep-file peep-rules.txt --peep-asm SMSlib_autotext.c
@if %errorlevel% NEQ 0 goto :EOF

sdcc -o SMSlib_zx7.rel -c -mz80 %OPT% SMSlib_zx7.c
@if %errorlevel% NEQ 0 goto :EOF

sdcc -o SMSlib_aPLib.rel -c -mz80 %OPT% SMSlib_aPLib.c
@if %errorlevel% NEQ 0 goto :EOF

sdcc -o SMSlib_deprecated.rel -c -mz80 %OPT% --peep-file peep-rules.txt --peep-asm SMSlib_deprecated.c
@if %errorlevel% NEQ 0 goto :EOF


sdar r SMSlib.lib SMSlib.rel SMSlib_sprite.rel SMSlib_twosprites.rel SMSlib_paletteAdv.rel SMSlib_spriteAdv.rel SMSlib_spriteClip.rel SMSlib_PSGaiden.rel SMSlib_STMcomp.rel SMSlib_loadTileMapArea.rel SMSlib_loadTiles.rel SMSlib_load1bppTiles.rel SMSlib_VRAMmemcpy.rel SMSlib_VRAMmemset.rel SMSlib_UNSAFE.rel SMSlib_textrenderer.rel SMSlib_autotext.rel SMSlib_zx7.rel SMSlib_aPLib.rel SMSlib_deprecated.rel
@if %errorlevel% NEQ 0 goto :EOF

sdar r SMSlib_GG.lib SMSlib_GG.rel SMSlib_sprite.rel SMSlib_twosprites.rel SMSlib_paletteAdv.rel SMSlib_spriteAdv.rel SMSlib_spriteClip_GG.rel SMSlib_PSGaiden.rel SMSlib_STMcomp.rel SMSlib_loadTileMapArea.rel SMSlib_loadTiles.rel SMSlib_load1bppTiles.rel SMSlib_VRAMmemcpy.rel SMSlib_VRAMmemset.rel SMSlib_UNSAFE.rel SMSlib_textrenderer.rel SMSlib_autotext_GG.rel SMSlib_zx7.rel SMSlib_aPLib.rel SMSlib_deprecated.rel
@if %errorlevel% NEQ 0 goto :EOF
