#include "libkdump.h"
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


const char *strings[] = {
    "If you can read this, this is really bad",
    "Burn after reading this string, it is a secret string",
    "Congratulations, you just spied on an application",
    "Wow, you broke the security boundary between user space and kernel",
    "Welcome to the wonderful world of microarchitectural attacks",
    "Please wait while we steal your secrets...",
    "Don't panic... But your CPU is broken and your data is not safe",
    "How can you read this? You should not read this!"};

int main(int argc, char *argv[]) {
  libkdump_config_t config;
  config = libkdump_get_autoconfig();
  libkdump_init(config);

  srand(time(NULL));
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

  // printf("\x1b[32;1m[+]\x1b[0m Secret: \x1b[33;1m%s\x1b[0m\n", secret);

  size_t paddr = libkdump_virt_to_phys((size_t)buffer);
  if (!paddr) {
    printf("\x1b[31;1m[!]\x1b[0m Program requires root privileges (or read access to /proc/<pid>/pagemap)!\n");
    libkdump_cleanup();
    exit(1);
  }

  printf("\x1b[32;1m[+]\x1b[0m Physical address of buffer: \x1b[32;1m0x%zx\x1b[0m\n", paddr);
  printf("\x1b[32;1m[+]\x1b[0m Exit with \x1b[37;1mCtrl+C\x1b[0m if you are done reading the secret\n");
  while (1) {
    // keep string cached for better results
    volatile size_t dummy = 0, i;
    for (i = 0; i < fileLen; i++) {
      dummy += buffer[i];
    }
    sched_yield();
  }

  libkdump_cleanup();

  return 0;
}
