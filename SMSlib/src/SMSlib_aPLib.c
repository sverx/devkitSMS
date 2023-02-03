/* **************************************************
   SMSlib - C programming library for the SMS/GG
   ( part of devkitSMS - github.com/sverx/devkitSMS )
   ************************************************** */

/*
*    Z80/SMS aPLib decompression library
*    version 1.2
*    1/12/2008
*    Maxim (http://www.smspower.org/maxim)
*    based on code by Dan Weiss (Dwedit) - see readme.txt
*    C wrapper by sverx
*/

#include "SMSlib.h"

unsigned char aPLib_bits[2];
unsigned char aPLib_LWM;
unsigned int  aPLib_R0;

#pragma save
#pragma disable_warning 85
void SMS_decompressaPLib (const void *src, void *dst) __naked __sdcccall(1) {
  __asm
  ;hl = source
  ;de = dest
  ldi
  xor a
  ld (_aPLib_LWM),a
  inc a
  ld (_aPLib_bits),a

_aploop:
  call ap_getbit
  jp z, _apbranch1
  call ap_getbit
  jr z, _apbranch2
  call ap_getbit
  jr z, _apbranch3
  ;LWM = 0
  xor a
  ld (_aPLib_LWM),a
  ;get an offset
  ld bc,#0
  call ap_getbitbc
  call ap_getbitbc
  call ap_getbitbc
  call ap_getbitbc
  ld a,b
  or c
  jr nz,_apbranch4
  ;  xor a  ;write a 0 ; Maxim: a is zero already (just failed nz test), optimise this line away
  ld (de),a
  inc de
  jr _aploop
_apbranch4:
  ex de,hl ;write a previous bit (1-15 away from dest)
  push hl
    sbc hl,bc
    ld a,(hl)
  pop hl
  ld (hl),a
  inc hl
  ex de,hl
  jr _aploop
_apbranch3:
  ;use 7 bit offset, length = 2 or 3
  ;if a zero is encountered here, it is EOF
  ld c,(hl)
  inc hl
  rr c
  ret z
  ld b,#2
  jr nc,_plus4
  inc b
_plus4:
  ;LWM = 1
  ld a,#1
  ld (_aPLib_LWM),a
  push hl
    ld a,b
    ld b,#0
    ;R0 = c
    ld (_aPLib_R0),bc
    ld h,d
    ld l,e
    or a
    sbc hl,bc
    ld c,a
    ldir
  pop hl
  jr _aploop

_apbranch2:
  ;use a gamma code * 256 for offset, another gamma code for length
  call ap_getgamma
  dec bc
  dec bc
  ld a,(_aPLib_LWM)
  or a
  jr nz,_ap_not_LWM
  ;bc = 2? ; Maxim: I think he means 0
  ld a,b
  or c
  jr nz,_ap_not_zero_gamma
  ;if gamma code is 2, use old R0 offset, and a new gamma code for length
  call ap_getgamma
  push hl
    ld h,d
    ld l,e
    push bc
      ld bc,(_aPLib_R0)
      sbc hl,bc
    pop bc
    ldir
  pop hl
  jr _ap_finishup

_ap_not_zero_gamma:
  dec bc
_ap_not_LWM:
  ;do I even need this code? ; Maxim: seems so, it is broken without it
  ;bc=bc*256+(hl), lazy 16bit way
  ld b,c
  ld c,(hl)
  inc hl
  ld (_aPLib_R0),bc
  push bc
    call ap_getgamma
    ex (sp),hl
    ;bc = len, hl=offs
    push de
      ex de,hl
      ;some comparison junk for some reason
      ; Maxim: optimised to use add instead of sbc
      ld hl,#-32000
      add hl,de
      jr nc,_plus1
      inc bc
_plus1:
      ld hl,#-1280
      add hl,de
      jr nc,_plus2
      inc bc
_plus2:
      ld hl,#-128
      add hl,de
      jr c,_plus3
      inc bc
      inc bc
_plus3:
      ;bc = len, de = offs, hl=junk
    pop hl
    push hl
      or a
      sbc hl,de
    pop de
    ;hl=dest-offs, bc=len, de = dest
    ldir
  pop hl
_ap_finishup:
  ld a,#1
  ld (_aPLib_LWM),a
  jp _aploop
_apbranch1:
  ldi
  xor a
  ld (_aPLib_LWM),a
  jp _aploop

ap_getbit:
  push bc
    ld bc,(_aPLib_bits)
    rrc c
    jr nc,_plus9
    ld b,(hl)
    inc hl
_plus9:
    ld a,c
    and b
    ld (_aPLib_bits),bc
  pop bc
  ret

ap_getbitbc: ;doubles BC and adds the read bit
  sla c
  rl b
  call ap_getbit
  ret z
  inc bc
  ret

ap_getgamma:
  ld bc,#1
_minus3:
  call ap_getbitbc
  call ap_getbit
  jr nz,_minus3
  ret
  __endasm;
}
#pragma restore
