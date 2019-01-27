#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <algorithm>

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

double clamp(double low, double v, double high) {
	return std::min(std::max(v, low), high);
}

uint8_t gray(double v) {
	return clamp(0, v, 1) * 255 + 0.5;
}

double sqr(double x) {
	return x * x;
}

int main (int argc, const char * argv[]) {
    if (argc < 3) {
		fprintf(stderr, "Usage:\n\t%s outfile.png x y scale\n", argv[0]);
		return 1;
	}

	char *_;
	
	int iwidth = atoi(argv[2]), iheight = atoi(argv[3]);
	double scale = argc < 5 ? 1.0 : strtod(argv[4], &_);
	uint32_t *image = (uint32_t *)malloc(iwidth*iheight*sizeof(uint32_t));

	double mx = double(iwidth)/2, my = double(iheight)/2;

	for(int y = 0; y < iheight; y++) {
		for(int x = 0; x < iwidth; x++) {
			int c = x+y*iwidth;
			unsigned char *color = (unsigned char *)&image[c];

			double ox = (x - mx) * scale, oy = (y - my) * scale;
			
			color[0] = gray(std::max(fabs(ox / mx), fabs(oy / my)));
			color[1] = gray(fabs(ox / mx) + fabs(oy / my));
			color[2] = gray(sqrt(sqr(ox / mx) + sqr(oy / my)));

			//color[2] = color[1] = color[0];

			color[3] = 255;
		}
	}

	stbi_write_png(argv[1], iwidth, iheight, 4, &image[0], 4*iwidth, YFLIP);
	
	return 0;
}
