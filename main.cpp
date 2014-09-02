#include <stdio.h>
#include <stdint.h>

extern "C" {
unsigned char *stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp);
extern int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes, int yflip);
}

#define PREFLIP 1
#define YFLIP 1

int main (int argc, const char * argv[]) {
    if (argc < 3) {
		fprintf(stderr, "Usage:\n\t%s infile.png outfile.png\n", argv[0]);
		return 1;
	}
	
	int iwidth, iheight;
	uint32_t *image = (uint32_t *)stbi_load(argv[1], &iwidth, &iheight, NULL, 4);

	if (!image) {
		fprintf(stderr, "Could not open image %s\n", argv[1]);
		return 1;
	}

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

	stbi_write_png(argv[2], iwidth, iheight, 4, &image[0], 4*iwidth, YFLIP);
	
	return 0;
}
