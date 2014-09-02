#include <stdio.h>
#include <stdint.h>

extern "C" {
unsigned char *stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp);
extern int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes, int yflip);
}

#define PREFLIP 0
#define YFLIP 1

int main (int argc, const char * argv[]) {
    if (argc < 3) {
		fprintf(stderr, "Usage:\n\t%s inimage.png outimage.png\n", argv[0]);
		return 1;
	}
	
	int iwidth, iheight;
	uint32_t *image = (uint32_t *)stbi_load(argv[1], &iwidth, &iheight, NULL, 4);

#if PREFLIP
	
#endif

	stbi_write_png(argv[2], iwidth, iheight, 4, &image[0], 4*iwidth, YFLIP);
	
	return 0;
}
