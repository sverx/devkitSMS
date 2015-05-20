/*
    folder2c - creates a .c and a .h file with the contents of
               every binary file in a folder

    sverx\2015

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

FILE *fIN;
FILE *fc;
FILE *fh;
DIR *dp;
struct dirent *entry;

#define BUFSIZE     16
unsigned char buf[BUFSIZE];
#define SUBST_NUM   4
char *subst[SUBST_NUM]={" ",".","(",")"};

void cleanstr(char *str) {
  int i;
  for (i=0;i<SUBST_NUM;i++)
    while (strstr(str,subst[i]))
      strncpy (strstr(str,subst[i]),"_",1);
}


int main(int argc, char const* *argv) {

  char *cname, *hname, *iname, *clean;
  int i,cnt,size;
  int total_size=0;
  
  printf("*** sverx's folder2c converter ***\n");
	
  if (argc!=3) {
    printf("Usage: folder2c folder outfilename\n");
    printf("creates outfilename.c and outfilename.h with contents of every\nfile found in folder\n");    
    return(1);
  }
  
  dp = opendir(argv[1]);
  if (dp == NULL) {
    printf("Fatal: can't open %s folder\n",argv[1]);
    return (1);
  }
  
  cname=malloc(strlen(argv[2])+3);
  hname=malloc(strlen(argv[2])+3);
  
  sprintf(cname,"%s.c",argv[2]);
  sprintf(hname,"%s.h",argv[2]);
  
  fc=fopen(cname,"w");
  if (!fc) {
    printf("Fatal: can't open output %s.c file\n",argv[2]);
    return(1);
  }
  
  fh=fopen(hname,"w");
  if (!fh) {
    printf("Fatal: can't open output %s.h file\n",argv[2]);
    return(1);
  }
  
  while ((entry=readdir(dp))) {
    if (entry->d_type==DT_REG) {
    
      clean=malloc(strlen(entry->d_name)+1);
      strcpy(clean,entry->d_name);
      cleanstr(clean);
      
      iname=malloc(strlen(argv[1])+strlen(entry->d_name)+2);
      sprintf(iname,"%s/%s",argv[1],entry->d_name);
      
      printf("Info: converting %s ...\n",entry->d_name);
      
      fIN=fopen(iname,"rb");
      if (!fc) {
        printf("Fatal: can't open %s\n",entry->d_name);
        return(1);
      }
      
      fprintf (fc,"const unsigned char %s[] = {\n",clean);
      
      size=0;
      while (!feof(fIN)) {
        cnt=fread (buf, 1, BUFSIZE, fIN);
        if ((size!=0) && (cnt!=0))
          fprintf (fc,",\n");
        size+=cnt;
        for (i=0;i<cnt;i++) {
          fprintf (fc,"0x%02x",buf[i]);
          if (i+1!=cnt)
            fprintf (fc,",");
        }
      }
      
      fprintf (fc,"};\n\n");
      
      fprintf (fh,"extern const unsigned char\t%s[];\n",clean);
      fprintf (fh,"#define\t\t\t\t%s_size %d\n\n",clean,size);
      
      fclose (fIN);
      free(clean);
      free(iname);
      
      total_size+=size;
    }
  }
  
  fclose (fc);
  fclose (fh);
  closedir(dp);
  
  free(cname);
  free(hname);

  printf("Info: conversion completed. File \"%s.c\" defines %d total bytes.\n",argv[2],total_size);
  return (0);
}
