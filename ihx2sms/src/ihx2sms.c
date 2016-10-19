/*
  ihx2sms - 'broken' ihx converter for multibank SEGA Master System / SEGA Game Gear ROMS

  with 'broken' I mean that the program it's actually assuming that each new declaration of data
  at address 0x8000 informs that we have to allocate a new ROM bank in the final SMS file

  sverx\2015

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

FILE *fIN;
FILE *fOUT;

unsigned char buf[1024*1024];
unsigned int size=0,used=0;
int use_additional_banks=0;
unsigned int add_banks=0;
unsigned int count, addr, type;
char data[256];
unsigned int bank_addr=0x8000;

#define BANK_SIZE           0x4000
#define SEGA_HEADER_ADDR    0x7ff0
#define SDSC_HEADER_ADDR    0x7fe0
#define CRT0_END            0x200

#define BYTE_TO_BCD(n) (((n)/10)*16+((n)%10))

int main(int argc, char const* *argv) {
	
  unsigned int i,dest_addr;
  char tmp[3];
  unsigned int checksum=0;
  
  printf("*** sverx's ihx2sms converter ***\n");
	
  if (argc!=3) {
    printf("Usage: ihx2sms infile.ihx outfile.sms\n");
    return(1);
  }
  
  fIN=fopen(argv[1],"rb");
  if (!fIN) {
    printf("Fatal: can't open input IHX file\n");
    return(1);
  }
  
  while (!feof(fIN)) {
    fscanf(fIN,":%2x%4x%2x%s\n", &count, &addr, &type, data);
    
    // printf(":%02x-%04x-%02x-%s\n", count, addr, type, data);
    
    switch (type) {
      case 0: // DATA
      
        if (addr==bank_addr) {
          if (use_additional_banks)
            add_banks++;
          else
            use_additional_banks=1;
        }
      
        for (i=0;i<count;i++) {

          if ((addr+i)>=bank_addr)
            dest_addr=addr+i+add_banks*BANK_SIZE;
          else
            dest_addr=addr+i;  
          
          strncpy (tmp,&data[i*2],2); 
          buf[dest_addr]=strtol(tmp,NULL,16);
          
          // printf("*%02x-%04x\n", buf[dest_addr], dest_addr);
          
          if (dest_addr>=size)
            size=dest_addr+1;
        }
        
        if (addr+count>0xC000) {
          printf("Fatal: allocating ROM at or past 0xC000\n");
          return(1);
        }
        
        if (addr>=CRT0_END)
          used+=count;

        break;
        
      case 1: // END (just ignore)
  	break;  
  	
    }
  	
  }
  fclose (fIN);

  if (size%BANK_SIZE)
    size=BANK_SIZE*((size/BANK_SIZE)+1);
    
  used+=CRT0_END;

  printf("Info: %d bytes used/%d total [%0.2f%%] - size of output ROM is %d KB\n",used,size,(float)used/(float)size*100,size/1024);

  if (size>=32*1024) {
    /* check/update SDSC header date */
    if (!strncmp("SDSC",(char *)&buf[SDSC_HEADER_ADDR],4)) {
      if (!memcmp("\0\0\0\0",&buf[SDSC_HEADER_ADDR+6],4)) {
        time_t curr_time = time(NULL);
        struct tm *compile_time = localtime(&curr_time);
        buf[SDSC_HEADER_ADDR+6]=BYTE_TO_BCD(compile_time->tm_mday);
        buf[SDSC_HEADER_ADDR+7]=BYTE_TO_BCD(compile_time->tm_mon+1);
        buf[SDSC_HEADER_ADDR+8]=BYTE_TO_BCD((compile_time->tm_year+1900)%100);
        buf[SDSC_HEADER_ADDR+9]=BYTE_TO_BCD((compile_time->tm_year+1900)/100);
        printf("Info: release date in SDSC header updated\n");
      }
    }

    /* check/update SEGA header checksum */
    if (!strncmp("TMR SEGA",(char *)&buf[SEGA_HEADER_ADDR],8)) {
      for (i=0;i<SEGA_HEADER_ADDR;i++)
        checksum+=buf[i];
      buf[SEGA_HEADER_ADDR+10]=checksum&0x00FF;
      buf[SEGA_HEADER_ADDR+11]=checksum>>8;
      printf("Info: SEGA header found, checksum updated\n");
    } else {
      printf("Warning: SEGA header NOT found, ROM won't be bootable on european/american Master System\n");
    }
  }

  fOUT=fopen(argv[2],"wb");
  if (!fOUT) {
    printf("Fatal: can't open output SMS file\n");
    return(1);
  }

  fwrite (&buf, 1, size, fOUT);
  fclose (fOUT);

  return (0);
}
