/*
     ihx2sms - 'broken' ihx converter for multibank SEGA Master System ROMS
     
     with 'broken' I mean that the program it's actually assuming that each new declaration of data
     at address 0x8000 informs that we have to allocate a new ROM bank in the final SMS file
     
     sverx\2015
               
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

FILE *fIN;
FILE *fOUT;

unsigned char buf[1024*1024];
unsigned int size=0;
int use_additional_banks=0;
unsigned int add_banks=0;
unsigned int count, addr, type;
char data[256];


int main(int argc, char const* *argv) {
	
  int i,dest_addr;
  char tmp[3];
  
  printf("*** sverx's IHX to SMS converter ***\n");
	
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
      
        if (addr==0x8000) {
          if (use_additional_banks)
            add_banks++;
          else
            use_additional_banks=1;
        }
      
        for (i=0;i<count;i++) {

          dest_addr=addr+i+add_banks*0x4000;
          
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

        break;
        
      case 1: // END (just ignore)
  	break;  
  	
    }
  	
  }
  fclose (fIN);

  if (size%16384)
    size=16384*((size/16384)+1);
    
  printf("Info: size of output ROM is %d KB\n",size/1024);
  
  fOUT=fopen(argv[2],"wb");
  if (!fOUT) {
    printf("Fatal: can't open output SMS file\n");
    return(1);
  }
  
  fwrite (&buf, 1, size, fOUT); 
  fclose (fOUT);	
  
  return (0);
}
