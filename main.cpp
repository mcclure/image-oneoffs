#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
unsigned char *stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp);
extern int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes, int yflip);
}

// This program recreates a botched attempt to write a PNG exporter that flipped the Y axis as it encoded.
// The attempt failed because the PNG encoder was using a form of compression where colors were stored
// with values relative to their neighbors, but the flipping was confusing the notion of "neighbors".
// To produce the effect, we take three steps:

// 1. Pad the image with one row of black pixels, since the glitched encoder commits a memory access violation.
#define SAFE 0

// 2. Manually reverse the order of the pixel rows, so the flipped image looks right side up.
#define PREFLIP 0

// 3. Instruct the encoder to run its broken y-flip routine.
#define YFLIP 0

int main (int argc, const char * argv[]) {
    if (argc < 4) {
		fprintf(stderr, "Usage:\n\t%s outfile.png x y randmask blackmask\n", argv[0]);
		return 1;
	}
	
	int iwidth = atoi(argv[2]), iheight = atoi(argv[3]), randmask = argc<5?0:atoi(argv[4]), blackmask = argc<6?0:atoi(argv[5]);
	uint32_t *image = (uint32_t *)malloc(iwidth*iheight*sizeof(uint32_t));

#if SAFE
	uint32_t *oldimage = image;
	image = (uint32_t *)malloc(4*iwidth*(iheight+1));
	memset(image, 0, 4*iwidth);
	image += iwidth;
	memcpy(image, oldimage, 4*iwidth*iheight);
	free(oldimage);
#endif

#if PREFLIP
	for(int y = 0; y < iheight/2; y++) {
		for(int x = 0; x < iwidth; x++) {
			int c = x+y*iwidth;
			int c2 = x+(iheight-y-1)*iwidth;
			uint32_t temp = image[c];
			image[c] = image[c2];
			image[c2] = temp;
		}
	}
#endif

	for(int y = 0; y < iheight; y++) {
		for(int x = 0; x < iwidth; x++) {
			int c = x+y*iwidth;
			unsigned char *color = (unsigned char *)&image[c];
			unsigned char basic = arc4random_uniform(256);

			for (int ch = 0; ch < 3; ch++) {
				if (randmask&(1<<ch))
					color[ch] = arc4random_uniform(256);
				else if (blackmask&(1<<ch))
					color[ch] = basic;
				else
					color[ch] = 0;
			}
			color[3] = 255;
		}
	}

	stbi_write_png(argv[1], iwidth, iheight, 4, &image[0], 4*iwidth, YFLIP);
	
	return 0;
}
