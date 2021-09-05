#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
unsigned char *stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp);
extern int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes, int yflip);
}

// Set 1 to glitch
#define BADPNG 0

template <class T>
T divideCeiling(T x, T y) {
	return x / y + (x%y ? 1 : 0);
}

int main (int argc, const char * argv[]) {
    if (argc < 3) {
		fprintf(stderr, "Usage:\n\t%s infile.png outfile.png [outrows] [incols] [framewidth] [fillcolor]\n", argv[0]);
		return 1;
	}
	
	int iwidth, iheight;
	uint32_t *image = (uint32_t *)stbi_load(argv[1], &iwidth, &iheight, NULL, 4);
	if (!image) {
		fprintf(stderr, "Error: Could not open image %s\n", argv[1]);
		return 1;
	}

	// Frame size arguments
	int outrows = argc > 3 ? atoi(argv[3]) : 0;
	int incols = argc > 4 ? atoi(argv[4]) : 0;
	int framewidth = argc > 5 ? atoi(argv[5]) : 0;
	if (!outrows) outrows = 2;
	if (!framewidth) framewidth = incols ? iwidth / incols : 1;
	if (!incols) incols = iwidth/framewidth;

	if (incols * framewidth > iwidth) {
		fprintf(stderr, "Error: in-columns (%d) * frame-width (%d) was wider than the image (%d)\n", incols, framewidth, iwidth);
		return 1;
	}

	// Empty-space color argument
	uint32_t fillcolor = 0;
	if (argc > 6) {
		char *endptr = 0;
		fillcolor = strtoul(argv[6], &endptr, 16);
		if (*endptr) {
			fprintf(stderr, "Error: Could not interpret fillcolor (%s) as a hex number (confused at char %d)\n", argv[6], (int)(endptr-argv[6]));
			return 1;
		}
	}

	// Do actual work
	int outcols = divideCeiling(incols, outrows);
	int owidth = framewidth*outcols;
	int oheight = outrows*iheight;

	uint32_t *outimage = (uint32_t *)malloc(owidth*oheight*sizeof(uint32_t));

	if (framewidth == 1)
		printf("split across %d rows\nfinal image %d x %d\n", outrows, owidth, oheight);
	else
		printf("%d frames split across %d rows\n%d frames per row\neach frame %d x %d\nfinal image %d x %d\n", incols, outrows, outcols, framewidth, iheight, owidth, oheight);

	for(int col = 0; col < outcols; col++) { // Iterate over each cel
		for(int row = 0; row < outrows; row++) {
			for(int y = 0; y < iheight; y++) { // Iterate over cel contents
				for(int x = 0; x < framewidth; x++) {
					int outidx = (row*iheight + y)*owidth + col*framewidth + x;
					unsigned char *outcolor = (unsigned char *)&outimage[outidx];
					int incol = row*outcols+col;
					if (row*outcols+col < incols) { // For odd # of frames, may be extra space
						//printf("%d %d %d %d\n", color[0], color[1], color[2], color[3]);
						int inidx = y*iwidth + incol*framewidth + x;
						unsigned char *incolor = (unsigned char *)&image[inidx];
						for (int ch = 0; ch < 4; ch++)
							outcolor[ch] = incolor[ch];
					} else {
						outcolor[3] = fillcolor & 0xFF;
						outcolor[2] = (fillcolor >> 8) & 0xFF;
						outcolor[1] = (fillcolor >> 16) & 0xFF;
						outcolor[0] = (fillcolor >> 24) & 0xFF;
					}
				}
			}
		}
	}

	stbi_write_png(argv[2], owidth, oheight, 4, &outimage[0], 4*owidth, BADPNG);
	
	return 0;
}
