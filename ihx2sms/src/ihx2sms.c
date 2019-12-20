/*
  ihx2sms - 'broken' ihx converter for multibank SEGA Master System / SEGA Game Gear ROMS

  with 'broken' I mean that the program it's actually assuming that each new declaration of data
  at address 0x8000 informs that we have to allocate a new ROM bank in the final SMS file

  sverx\2015-2019

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define BANK_SIZE           0x4000
#define BANK_ADDR           0x8000
#define MAX_SLOT2_BANKS     62
#define SEGA_HEADER_ADDR    0x7ff0
#define SDSC_HEADER_ADDR    0x7fe0
#define CRT0_END            0x200

#define SEGA_HEADER_ADDR_16K    0x3ff0

#define BYTE_TO_BCD(n)      (((n)/10)*16+((n)%10))

unsigned char buf[1024*1024];
unsigned int size=0;
unsigned int used=CRT0_END,used_low=CRT0_END;
int use_additional_banks=0;
unsigned int add_banks=0;
unsigned int count, addr, type;
char data[256];
unsigned char map_loc[MAX_SLOT2_BANKS];
unsigned int num_map_loc=0;
FILE *fIN;
FILE *fOUT;


int get_slot2_bank_order(const char* map_file) {

    enum map_parse_states {
        PARSE_SEGMENT_NAMES,
        PARSE_SEGMENT_ORDER,
    };

    char map_bank_name[MAX_SLOT2_BANKS][128];
    char line[128];
    int parse_state = PARSE_SEGMENT_NAMES;
    int nameidx = 0;
    int locidx = 0;

    FILE* fMAP = fopen(map_file,"rb");
    if (!fMAP) {
        printf("Fatal: can't open MAP file\n");
        return 1;
    }
    memset(map_loc, 0, MAX_SLOT2_BANKS);

    while (!feof(fMAP)) {
        fgets(line, sizeof(line), fMAP);

        if (parse_state == PARSE_SEGMENT_NAMES) {
            // search for segments assigned to 0x8000 (ROM slot 2)
            char* def = strstr(line, "=0x8000");
            if (def) {
                int len = def - line;
                strncpy(map_bank_name[nameidx], line, len);
                map_bank_name[nameidx++][len] = '\0';
                if (nameidx >= MAX_SLOT2_BANKS) {
                    printf("Fatal: Exceeded %d slot2 segment definitions\n", MAX_SLOT2_BANKS);
                    return 1;
                }
            }
            
            if (feof(fMAP)) {
                if (nameidx == 0) {
                    return 0; // no slot2 segments found
                }
                // start from the top now that we know what to look for
                fseek(fMAP, 0, SEEK_SET);
                parse_state = PARSE_SEGMENT_ORDER;
            }
        } else if (parse_state == PARSE_SEGMENT_ORDER) {
            // the order the code areas appear == data order in the ihx file
            int i;
            for (i = 0; i < nameidx; i++) {
                if (strstr(line, map_bank_name[i]) == line && strstr(line, "=0x8000") == 0) {
                    map_loc[locidx++] = i;
                    if (locidx >= MAX_SLOT2_BANKS) {
                        printf("Fatal: Exceeded %d slot2 code areas\n", MAX_SLOT2_BANKS);
                        return 1;
                    }
                }
            }
        }
    }

    fclose(fMAP);
    if (nameidx != locidx) {
        // parser is bugged or we weren't reading a map file?
        printf("Fatal: segment definition count != area count\n");
        return 1;
    }
    num_map_loc = locidx;

    return 0;
}

int main(int argc, char const* *argv) {
  
  unsigned int i,dest_addr;
  char tmp[3];
  unsigned int checksum=0;
  int using_map=0;
  
  printf("*** sverx's ihx2sms converter ***\n");

  if (argc==5 && strcmp(argv[1], "-m")==0) {
    using_map=1;
  }

  if (argc!=3 && !using_map) {
    printf("Usage: ihx2sms [-m file.map] infile.ihx outfile.sms\n");
    return(1);
  }
  
  fIN=fopen(argv[using_map ? 3 : 1],"rb");
  if (!fIN) {
    printf("Fatal: can't open input IHX file\n");
    return(1);
  }

  if (using_map) {
    int ret=get_slot2_bank_order(argv[2]);
    if (ret) {
      return ret;
    }
  }

  while (!feof(fIN)) {
    fscanf(fIN,":%2x%4x%2x%s\n", &count, &addr, &type, data);
    
    // printf(":%02x-%04x-%02x-%s\n", count, addr, type, data);
    
    switch (type) {
      case 0: // DATA
      
        if (addr==BANK_ADDR) {
          if (use_additional_banks)
            add_banks++;
          else
            use_additional_banks=1;
          if (add_banks>=MAX_SLOT2_BANKS) {
            printf("Fatal: Exceeded %d slot2 banks\n", MAX_SLOT2_BANKS);
            return 1;
          }
        }
      
        for (i=0;i<count;i++) {
          if ((addr+i)>=BANK_ADDR) {
            if (using_map) {
              dest_addr=addr+i+map_loc[add_banks]*BANK_SIZE;
            } else {
              dest_addr=addr+i+add_banks*BANK_SIZE;
            }
          }
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
        
        if (addr>=CRT0_END) {
          used+=count;
          if (addr<BANK_ADDR)
            used_low+=count;
        }

        break;
        
      case 1: // END (just ignore)
    break;  
    
    }
    
  }
  fclose (fIN);

  if (size%BANK_SIZE)
    size=BANK_SIZE*((size/BANK_SIZE)+1);
  
  if (size>32*1024)
    printf("Info: %d bytes used/%d total [%0.2f%%] - %d bytes used in banks 0,1 [%0.2f%%] - size of output ROM is %d KB\n",used,size,(float)used/(float)size*100, used_low,(float)used_low/((float)32*1024)*100, size/1024);
  else
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
  } else { 
    /* it's probably a 16 KB ROM, check/update SEGA header checksum */
    if (!strncmp("TMR SEGA",(char *)&buf[SEGA_HEADER_ADDR_16K],8)) {
      for (i=0;i<SEGA_HEADER_ADDR_16K;i++)
        checksum+=buf[i];
      buf[SEGA_HEADER_ADDR_16K+10]=checksum&0x00FF;
      buf[SEGA_HEADER_ADDR_16K+11]=checksum>>8;
      printf("Info: SEGA header found, checksum updated\n");
    } else {
      printf("Warning: SEGA header NOT found, ROM won't be bootable on european/american Master System\n");
    }
  }

  fOUT=fopen(argv[using_map ? 4 : 2],"wb");
  if (!fOUT) {
    printf("Fatal: can't open output SMS file\n");
    return(1);
  }

  fwrite (&buf, 1, size, fOUT);
  fclose (fOUT);

  return (0);
}
