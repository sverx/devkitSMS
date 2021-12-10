/*
  makesms - IHX converter to BINary SEGA Master System / SEGA Game Gear ROMS

  sverx\2020
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define BYTE_TO_BCD(n)      (((n)/10)*16+((n)%10))

#define BANK_SIZE           0x4000
#define CRT0_END            0x200
#define BANK_ADDR           0x4000

#define SEGA_HEADER_ADDR_16K    0x3ff0
#define SDSC_HEADER_ADDR_16K    0x3fe0

unsigned char buf[4*1024*1024];    // generated ROM max: 4MB
unsigned short used_bank[256];
unsigned int size=0;
unsigned char segment=0;
unsigned char padding_type=0;
unsigned int used=CRT0_END,used_low=CRT0_END;
unsigned int count, addr, type;

char data[256];

FILE *fIN;
FILE *fOUT;

void usage (int ret_value) {
  printf("Usage: makesms [-pm|-pp] infile.ihx outfile.sms\n");
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

  unsigned int i,dest_addr,dest_bank;
  char tmp[3];
  unsigned int checksum=0;
  int cur_arg=1;

  printf("*** sverx's makesms converter ***\n");

  if (argc<3)
    usage(1);

  while (cur_arg<(argc-2)) {
    if (strcmp(argv[cur_arg], "-pm")==0)
      padding_type=1;
    else if (strcmp(argv[cur_arg], "-pp")==0)
      padding_type=2;
    else {
      printf("Fatal: can't understand argument '%s'\n",argv[cur_arg]);
      usage(1);
    }
    cur_arg++;
  }

  fIN=fopen(argv[argc-2],"rb");
  if (!fIN) {
    printf("Fatal: can't open input IHX file\n");
    return(1);
  }

  while (!feof(fIN)) {
    fscanf(fIN,":%2x%4x%2x%s\n", &count, &addr, &type, data);

    // printf(":%02x-%04x-%02x-%s\n", count, addr, type, data);

    switch (type) {
      case 0: // DATA

          if (addr>=BANK_ADDR) {
            dest_addr=(segment*16*1024)+(addr & 0x3fff);
            dest_bank=segment;
          } else {
            dest_addr=addr;
            dest_bank=0;
          }

        for (i=0;i<count;i++) {

          strncpy (tmp,&data[i*2],2);
          buf[dest_addr+i]=strtol(tmp,NULL,16);

          // printf("*%02x-%04x\n", buf[dest_addr], dest_addr);

          if ((dest_addr+i)>=size)
            size=(dest_addr+i)+1;
        }

        if (addr>=CRT0_END) {
          used+=count;
          if (addr<BANK_ADDR)
            used_low+=count;
        }

        used_bank[dest_bank]+=count;

        if (used_bank[dest_bank]>BANK_SIZE) {
          printf("Fatal: Bank %d overflow.\n", dest_bank);
          return(1);
        }

        break;

      case 4: // SET SEGMENT

        strncpy (tmp,&data[2],2);
        segment=strtol(tmp,NULL,16);
        break;

      case 1: // END (just ignore)
        break;

      default: // everything else (just ignore in case there's something)
        break;
    }

  }
  fclose (fIN);

  if (size%BANK_SIZE)
    size=BANK_SIZE*((size/BANK_SIZE)+1);      // make BIN size exact multiple of BANK_SIZE

  if (size>BANK_SIZE) {
    printf("Info: %d bytes used/%d total [%0.2f%%] - %d bytes used in bank 0 [%0.2f%%] - size of output ROM is %d KB\n",used,size,(float)used/(float)size*100, used_low,(float)used_low/((float)16*1024)*100, size/1024);
    printf("Info: ");
    for (i=0;i<size/BANK_SIZE;i++)
          printf("[bank%d %d] ",i,BANK_SIZE-((i==0)?used_low:used_bank[i]));
    printf("bytes free\n");
  } else
    printf("Info: %d bytes used/%d total [%0.2f%%] - size of output ROM is %d KB\n",used,size,(float)used/(float)size*100,size/1024);



  /* check/update SDSC header date */
  if (!strncmp("SDSC",(char *)&buf[SDSC_HEADER_ADDR_16K],4)) {
    if (!memcmp("\0\0\0\0",&buf[SDSC_HEADER_ADDR_16K+6],4)) {
      time_t curr_time = time(NULL);
      struct tm *compile_time = localtime(&curr_time);
      buf[SDSC_HEADER_ADDR_16K+6]=BYTE_TO_BCD(compile_time->tm_mday);
      buf[SDSC_HEADER_ADDR_16K+7]=BYTE_TO_BCD(compile_time->tm_mon+1);
      buf[SDSC_HEADER_ADDR_16K+8]=BYTE_TO_BCD((compile_time->tm_year+1900)%100);
      buf[SDSC_HEADER_ADDR_16K+9]=BYTE_TO_BCD((compile_time->tm_year+1900)/100);
      printf("Info: release date in SDSC header updated\n");
    }
  }

  /* check/update SEGA header checksum */
  if (!strncmp("TMR SEGA",(char *)&buf[SEGA_HEADER_ADDR_16K],8)) {
    for (i=0;i<SEGA_HEADER_ADDR_16K;i++)
      checksum+=buf[i];
    buf[SEGA_HEADER_ADDR_16K+10]=checksum&0x00FF;
    buf[SEGA_HEADER_ADDR_16K+11]=checksum>>8;
    printf("Info: SEGA header found, checksum updated\n");
  } else {
    printf("Warning: SEGA header NOT found, ROM won't be bootable on european/american Master System\n");
  }

  fOUT=fopen(argv[argc-1],"wb");
  if (!fOUT) {
    printf("Fatal: can't open output SMS file\n");
    return(1);
  }

  if (padding_type!=0) {
    if (padding_type==1) {
      while (size%(64*1024)!=0)
        size+=BANK_SIZE;
    } else if (padding_type==2) {
      while (count_set_bits(size)!=1)
        size+=BANK_SIZE;
    }
    printf("Info: ROM size padded to %d KB\n",size/1024);
  }

  fwrite (&buf, 1, size, fOUT);
  fclose (fOUT);

  return (0);
}
