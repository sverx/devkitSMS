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
#define MAX_ROM_SIZE        (4*1024*1024)  // generated ROM max: 4MB
#define DEFAULT_EMPTY_FILL  0x00

#define SEGA_HEADER_ADDR_32K    0x7ff0
#define SDSC_HEADER_ADDR_32K    0x7fe0

#define SEGA_HEADER_ADDR_16K    0x3ff0
#define SDSC_HEADER_ADDR_16K    0x3fe0

unsigned char buf[MAX_ROM_SIZE];
unsigned short used_bank[256];
unsigned int size=0;
unsigned char segment=0;
unsigned char padding_type=0;
unsigned int used=CRT0_END;
unsigned int count, addr, type;
unsigned char emptyfill = DEFAULT_EMPTY_FILL;

struct merge {
  char *filename;
  unsigned int src;
  unsigned int len;
  unsigned int dst;
};

#define MAX_MERGES  8
struct merge merges[MAX_MERGES];
int num_merges;

char data[256];

FILE *fIN;
FILE *fOUT;

void usage (int ret_value) {
  printf("Usage: makesms [options] infile.ihx outfile.sms\n");
  printf("\nSupported options:\n");
  printf(" -pm                        Pad ROM size to a multiple of 64kB\n");
  printf(" -pp                        Pad ROM size to a power of 2\n");
  printf(" -mbin file:src:len:dst     Merge contents of binary file into ROM before\n");
  printf("                            updating header and checksum. Offsets and length\n");
  printf("                            expressed in bytes.\n");
  printf(" -mbank file:src:banks:dst  Merge contents of binary file into ROM before\n");
  printf("                            updating header and checksum. Offsets and length\n");
  printf("                            expressed in banks.\n");
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

int addMerge(const char *arg, int mult) {

  struct merge mg;
  char *filename, *tmp;

  if (num_merges >= MAX_MERGES) {
    fprintf(stderr, "Too many merges");
    return -1;
  }

  filename = strdup(arg);
  if (!filename) {
    perror("strdup");
    return -1;
  }

  // Find first separator
  tmp = strchr(filename, ':');
  if (!tmp) {
    fprintf(stderr, "Invalid -mbin argument\n");
    return -1;
  }
  *tmp = 0; // replace sparator by NUL to end string
  mg.filename = filename;

  // skip NUL
  tmp++;

  if (3 != sscanf(tmp, "%i:%i:%i", &mg.src, &mg.len, &mg.dst)) {
    fprintf(stderr, "Missing -mbin argument\n");
    free(filename);
    return -1;
  }

  mg.src *= mult;
  mg.len *= mult;
  mg.dst *= mult;

  if (mg.dst + mg.len >= MAX_ROM_SIZE) {
    fprintf(stderr, "Merge exceeds max ROM size\n");
    free(filename);
    return -1;
  }

  memcpy(&merges[num_merges], &mg, sizeof(struct merge));
  num_merges++;

  return 0;
}

void freeMerges(void) {
  int i;

  for (i=0; i<num_merges; i++) {
    free(merges[i].filename);
  }
}

int processMerges(void) {
  int i, j;
  FILE *fptr;

  for (i=0; i<num_merges; i++) {
    fptr = fopen(merges[i].filename, "rb");
    if (!fptr) {
      perror(merges[i].filename);
      return -1;
    }
    if (fseek(fptr, merges[i].src, SEEK_SET) != 0) {
      perror("Could not seek");
      fclose(fptr);
      return -1;
    }
    // Check for non-empty content
    for (j=merges[i].dst; j<merges[i].dst + merges[i].len; j++) {
      if (buf[j] != emptyfill) {
        fprintf(stderr, "Error: Attempt to merge over non-empty (!= 0x%02x) content\n", emptyfill);
        fclose(fptr);
        return -1;
      }
    }
    // addMerge already checked that this fits in the ROM buffer
    if (fread(buf + merges[i].dst, merges[i].len, 1, fptr) != 1) {
      perror("error reading merge source");
      fclose(fptr);
      return -1;
    }
    // grow ROM size (potentially)
    if (merges[i].dst + merges[i].len > size) {
      size = merges[i].dst + merges[i].len;
    }
    // verbose
    printf("Merged %d bytes from %s:0x%06x to ROM:0x%06x\n", merges[i].len, merges[i].filename, merges[i].src, merges[i].dst);
    fclose(fptr);
  }

  return 0;
}

int main(int argc, char const* *argv) {

  unsigned int i,dest_addr;
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
    else if (strcmp(argv[cur_arg], "-mbin")==0) {
      cur_arg++;
      if (cur_arg >=(argc)) {
        fprintf(stderr, "Missing value for -mbin\n");
        usage(1);
      }
      addMerge(argv[cur_arg], 1);
    }
    else if (strcmp(argv[cur_arg], "-mbank")==0) {
      cur_arg++;
      if (cur_arg >=(argc)) {
        fprintf(stderr, "Missing value for -mbank\n");
        usage(1);
      }
      addMerge(argv[cur_arg], BANK_SIZE);
    }
    else if (strcmp(argv[cur_arg], "-emptyfill")==0) {
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

  used_bank[0]=CRT0_END;

  // Initialize buffer with fill value
  memset(buf, emptyfill, MAX_ROM_SIZE);

  while (!feof(fIN)) {
    fscanf(fIN,":%2x%4x%2x%s\n", &count, &addr, &type, data);

    // printf(":%02x-%04x-%02x-%s\n", count, addr, type, data);

    switch (type) {
      case 0: // DATA

        if ((addr>=BANK_ADDR) && (segment!=0)) {
          dest_addr=(segment*16*1024)+(addr & 0x3fff);
        } else {
          dest_addr=addr;
        }

        for (i=0;i<count;i++) {

          strncpy (tmp,&data[i*2],2);
          buf[dest_addr+i]=strtol(tmp,NULL,16);

          // printf("*%02x-%04x\n", buf[dest_addr], dest_addr);

          if (addr>=CRT0_END) {
            if (++used_bank[(dest_addr+i)/BANK_ADDR]>BANK_SIZE) {
              printf("Fatal: Bank %d overflow.\n", (dest_addr+i)/BANK_ADDR);
              return(1);
            }
          }

          if ((dest_addr+i)>=size) {
            size=(dest_addr+i)+1;
          }
        }  // end for

        if (addr>=CRT0_END) {
          used+=count;
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

  if (processMerges()) {
    return(1);
  }

  if (size%BANK_SIZE)
    size=BANK_SIZE*((size/BANK_SIZE)+1);      // make BIN size exact multiple of BANK_SIZE

  printf("Info: %d bytes used/%d total [%0.2f%%] - size of output ROM is %d KB\n",used,size,(float)used/(float)size*100, size/1024);

  if (size>BANK_SIZE) {
    printf("Info: ");
    for (i=0;i<size/BANK_SIZE;i++)
          printf("[bank%d %d] ",i,BANK_SIZE-used_bank[i]);
    printf("bytes free\n");
  }

  /* check/update SDSC header date */
  if (!strncmp("SDSC",(char *)&buf[SDSC_HEADER_ADDR_32K],4)) {
    if (!memcmp("\0\0\0\0",&buf[SDSC_HEADER_ADDR_32K+6],4)) {
      time_t curr_time = time(NULL);
      struct tm *compile_time = localtime(&curr_time);
      buf[SDSC_HEADER_ADDR_32K+6]=BYTE_TO_BCD(compile_time->tm_mday);
      buf[SDSC_HEADER_ADDR_32K+7]=BYTE_TO_BCD(compile_time->tm_mon+1);
      buf[SDSC_HEADER_ADDR_32K+8]=BYTE_TO_BCD((compile_time->tm_year+1900)%100);
      buf[SDSC_HEADER_ADDR_32K+9]=BYTE_TO_BCD((compile_time->tm_year+1900)/100);
      printf("Info: release date in SDSC header updated\n");
    }
  } else if (!strncmp("SDSC",(char *)&buf[SDSC_HEADER_ADDR_16K],4)) {
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
  if (!strncmp("TMR SEGA",(char *)&buf[SEGA_HEADER_ADDR_32K],8)) {
    for (i=0;i<SEGA_HEADER_ADDR_32K;i++)
      checksum+=buf[i];
    buf[SEGA_HEADER_ADDR_32K+10]=checksum&0x00FF;
    buf[SEGA_HEADER_ADDR_32K+11]=checksum>>8;
    printf("Info: SEGA header found, checksum updated\n");
  } else if (!strncmp("TMR SEGA",(char *)&buf[SEGA_HEADER_ADDR_16K],8)) {
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
    freeMerges();
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

  freeMerges();

  return (0);
}
