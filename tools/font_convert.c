#include		<stdlib.h>
#include		<stdio.h>

#include		<SDL/SDL.h>
#include		<SDL/SDL_image.h>

int main(int argc, char **argv) {
	FILE *out;
	int i, j, k, data, slot, t, w;
	unsigned char *ptr;
	SDL_Surface *tex;
	
	if (argc <3) {
		fprintf(stderr, "Invalid amount of arguments!\n");
		printf("Usage: font_convert <list> <destination>\n");
		return -1;
	}
	
	
	if ((out = fopen(argv[2], "w")) == NULL) {
		fprintf(stderr, "Failed to open file %s for writing+trunc!\n", argv[2]);
		return -1;
	}
	
	if ((tex = IMG_Load(argv[1])) == NULL) {
		printf("Failed to load %s\n", argv[1]);
		return 0;
	}

	fprintf(stderr, "%i\n", tex->format->BytesPerPixel);

	slot = tex->w / 9;
	ptr = tex->pixels;
	w = tex->w + (tex->w % 4);
	fprintf(out, "#ifndef __VGA_FONT_H__\n#define __VGA_FONT_H__\n");
	fprintf(out, "static unsigned char vgafont_data[%i] = {", slot * 16);
	for (i = 0; i < slot; i++) {
		fprintf(out, "\n\t");
		for (j = 0; j < 16; j++) {
			data = 0;
			for (k = 0; k < 8; k++) {
				t = i * 9 + j * w + k;
				if (ptr[t])
					data |= (1 << k);
			//	fprintf(stderr, "%i,%i,%i;; %i, %i, %i\n", i, j, k, t, ptr[t], data);
			}
			if (j > 0)
				fprintf(out, ", ");
			fprintf(out, "%i", data);
			if (j == 15 && i + 1 < slot)
				fprintf(out, ",");
		}
	}
	fprintf(out, "\n};");
	fprintf(out, "\n#endif\n");
					

	fclose(out);
	
	return 0;
}
