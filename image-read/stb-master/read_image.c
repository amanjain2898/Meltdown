#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main() {
    int width, height, bpp;

    uint8_t* rgb_image = stbi_load("image.jpg", &width, &height, &bpp, 3);

    for(int i=0;i<height;i++)
    {
    	for(int j=0;j<width;j++)
    	{
    		printf("%d ",rgb_image[i*100 + j]);
    	}
    	printf("\n");
    }
    stbi_image_free(rgb_image);

    return 0;
}