/*
  makecvmc - IHX converter to BINary for ColecoVision MegaCart

  sverx\2025
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define BYTE_TO_BCD(n)      (((n)/10)*16+((n)%10))

#define MAX_BANKS                      64
#define BANK_SIZE                      0x4000
#define AVAILABLE_BANK_SIZE           (BANK_SIZE-64)
#define CRT0_END                       0x80
#define BASE_ADDR                      0x8000
#define BANKED_ADDR                    0xC000
#define DEFAULT_EMPTY_FILL             0x00

char textnote[]="* NOTE: THIS IS A COLECOVISION MEGACART * created with makecvmc ";

unsigned char buf[MAX_BANKS][BANK_SIZE];
unsigned short used_bank[MAX_BANKS];
unsigned int size,avail_size,used=0;
unsigned char segment=0,highest_segment=0;
unsigned int count, addr, type;
unsigned char emptyfill = DEFAULT_EMPTY_FILL;
char data[256];

FILE *fIN;
FILE *fOUT;

void usage (int ret_value) {
  printf("Usage: makecvmc [options] infile.ihx outfile.rom\n");
  printf("\nSupported options:\n");
  printf(" -emptyfill value           Fill unused memory with specified value. Default: 0x%02x\n", DEFAULT_EMPTY_FILL);

  exit(ret_value);
}

int count_set_bits (unsigned int value) {
  int cnt=0;
  while (value) {
    cnt+=(value & 0x01);
    value>>=1;
  }
  return (cnt);
}

int main(int argc, char const* *argv) {

  unsigned int i;
  char tmp[3];
  int cur_arg=1;

  printf("*** sverx's makecvmc converter ***\n");

  if (argc<3)
    usage(1);

  while (cur_arg<(argc-2)) {
    if (strcmp(argv[cur_arg], "-emptyfill")==0) {
      cur_arg++;
      if (cur_arg >=(argc)) {
        fprintf(stderr, "Missing value for -emptyfill\n");
        usage(1);
      }
      emptyfill = strtol(argv[cur_arg], NULL, 0);
    }
    else {
      fprintf(stderr, "Fatal: can't understand argument '%s'\n",argv[cur_arg]);
      usage(1);
    }
    cur_arg++;
  }

  fIN=fopen(argv[argc-2],"rb");
  if (!fIN) {
    printf("Fatal: can't open input IHX file\n");
    return(1);
  }

  // Initialize buffers with fill value
  memset(buf, emptyfill, MAX_BANKS*BANK_SIZE);
  used_bank[0]=CRT0_END;

  while (!feof(fIN)) {
    fscanf(fIN,":%2x%4x%2x%s\n", &count, &addr, &type, data);

    // printf(":%02x-%04x-%02x-%s\n", count, addr, type, data);

    switch (type) {
      case 0: // DATA

        for (i=0;i<count;i++) {

          strncpy (tmp,&data[i*2],2);
          unsigned char value=strtol(tmp,NULL,16);

          if (addr>=BANKED_ADDR) {
            buf[segment][(addr & 0x3fff)+i]=value;

            // printf("*%02x-%04x\n", buf[dest_addr], dest_addr);

            if (++used_bank[segment]>BANK_SIZE) {
              printf("Fatal: Bank %d overflow.\n", segment);
              return(1);
            }
          } else if (addr>=BASE_ADDR) {
            buf[0][(addr & 0x3fff)+i]=value;

            // printf("*%02x-%04x\n", buf[dest_addr], dest_addr);

            if ((addr>=BASE_ADDR+CRT0_END) && (++used_bank[0]>AVAILABLE_BANK_SIZE)) {
              printf("Fatal: Base ROM overflow.\n");
              return(1);
            }
          }
        }  // end for
        break;

      case 4: // SET SEGMENT

        strncpy (tmp,&data[2],2);
        segment=strtol(tmp,NULL,16);

        if (segment>MAX_BANKS) {
          printf("Fatal: Mega Cart supports only up to 64 banks.\n");
          return(1);
        }

        if (segment>highest_segment)
          highest_segment=segment;

        break;

      case 1: // END (just ignore)
        break;

      default: // everything else (just ignore in case there's something)
        break;
    }

  }
  fclose (fIN);

  if (highest_segment<8) {
    // 128 KiB ROM
    size=8*BANK_SIZE;
    avail_size=7*AVAILABLE_BANK_SIZE+BANK_SIZE;
  } else if (highest_segment<16) {
    // 256 KiB ROM
    size=16*BANK_SIZE;
    avail_size=15*AVAILABLE_BANK_SIZE+BANK_SIZE;
  } else if (highest_segment<32) {
    // 512 KiB ROM
    size=32*BANK_SIZE;
    avail_size=31*AVAILABLE_BANK_SIZE+BANK_SIZE;
  } else {
    // 1024 KiB ROM
    size=64*BANK_SIZE;
    avail_size=63*AVAILABLE_BANK_SIZE+BANK_SIZE;
  }

  for (i=0;i<(size/BANK_SIZE);i++)
    used+=used_bank[i];

  printf("Info: %d bytes used/%d total [%0.2f%%] - size of output ROM file is %d KB\n",used,avail_size,(float)used/(float)avail_size*100, size/1024);

  printf("Info: ");
    printf("[bank0 %d] ",BANK_SIZE-used_bank[0]);
  for (i=1;i<(size/BANK_SIZE);i++)
    printf("[bank%d %d] ",i,AVAILABLE_BANK_SIZE-used_bank[i]);
  printf("bytes free\n");

  fOUT=fopen(argv[argc-1],"wb");
  if (!fOUT) {
    printf("Fatal: can't open output ROM file\n");
    return(1);
  }

  // add text note at end of bank1
  memcpy(&buf[1][AVAILABLE_BANK_SIZE],&textnote,BANK_SIZE-AVAILABLE_BANK_SIZE);

  // mega cart banks are written in reversed order
  i=(size/BANK_SIZE);
  do {
    i--;
    fwrite (&buf[i], BANK_SIZE, 1, fOUT);
  } while (i>0);

  fclose (fOUT);
  return (0);
}
