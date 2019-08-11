#include "libkdump.h"
#include <stdio.h>
#include <stdlib.h>

// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "./stb-master/stb_image_write.h"

int main(int argc, char *argv[]) {
  size_t phys;
  if (argc < 3) {
    printf("Usage: %s <physical address> [<direct physical map>]\n", argv[0]);
    return 0;
  }

  phys = strtoull(argv[1], NULL, 0);

  libkdump_config_t config;
  config = libkdump_get_autoconfig();
  if (argc > 2) {
    config.physical_offset = strtoull(argv[2], NULL, 0);
  }

  libkdump_init(config);

  size_t vaddr = libkdump_phys_to_virt(phys);

  printf("\x1b[32;1m[+]\x1b[0m Physical address       : \x1b[33;1m0x%zx\x1b[0m\n", phys);
  printf("\x1b[32;1m[+]\x1b[0m Physical offset        : \x1b[33;1m0x%zx\x1b[0m\n", config.physical_offset);
  printf("\x1b[32;1m[+]\x1b[0m Reading virtual address: \x1b[33;1m0x%zx\x1b[0m\n\n", vaddr);

  // int height = atoi(argv[3]);
  // int width = atoi(argv[4]);
  // int img[height][width];
  // int i = 0, j = 0;

  while (1) {
    // img[i][j] = (uint8_t)libkdump_read(vaddr);
    int value = libkdump_read(vaddr);
    // img[i][j] = value;
    // printf("%d ", img[i][j]);
    printf("%x ", value);
    fflush(stdout);
    vaddr++;
    // j = (j + 1)%width;
    // if(j == 0){
    //   printf("%d \n",i);
    //   i++;
    // }

    // if(i >= height)
    //   break;
  }

  // stbi_write_png("image.png", width, height, 1, img, width);

  libkdump_cleanup();

  return 0;
}
