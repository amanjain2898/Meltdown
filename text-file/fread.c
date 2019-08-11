#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


int main(int argc, char *argv[]) {
  // const char *secret = strings[rand() % (sizeof(strings) / sizeof(strings[0]))];

  FILE *file;
  unsigned char *buffer;
  unsigned long fileLen;

  //Open file
  file = fopen(argv[1], "rb");
  if (!file)
  {
          fprintf(stderr, "Unable to open file %s", argv[1]);
          return 0;
  }

  //Get file length
  fseek(file, 0, SEEK_END);
  fileLen=ftell(file);
  fseek(file, 0, SEEK_SET);

  //Allocate memory
  buffer=(char *)malloc(fileLen);
  if (!buffer)
  {
          fprintf(stderr, "Memory error!");
                          fclose(file);
          return 0;
  }

 fread(buffer,fileLen,sizeof(unsigned char),file);
 fclose(file);

 int i=0;

 while (i < fileLen){
     printf("%02X ",((unsigned char)buffer[i]));
     i++;
     if( ! (i % 16) ) printf( "\n" );
 }

  return 0;
}
