#include <stdint.h>
#include "libkdump.h"
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define STB_IMAGE_IMPLEMENTATION
#include "./stb-master/stb_image.h"


int main() {
  int width, height, bpp;
  libkdump_config_t config;
  config = libkdump_get_autoconfig();
  libkdump_init(config);

  srand(time(NULL));
  // const char *secret = strings[rand() % (sizeof(strings) / sizeof(strings[0]))];

  uint8_t* buffer = stbi_load("img1.jpg", &width, &height, &bpp, 3);

  printf("height = %d, width = %d \n", height, width);
  for(int i=0;i<width;i++){
    for(int j=0;j<height;j++){
        printf("%x ",buffer[width*i+j]);
    }
    printf("\n");
  }

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
    for (i = 0; i < width*height*3; i++) {
      dummy += buffer[i];
    }
    sched_yield();
  }

  stbi_image_free(buffer);
  libkdump_cleanup();
  return 0;
}