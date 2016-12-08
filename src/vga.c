#include <stdint.h>
#include "vga.h"
#include "mem.h"
#include "chipset.h"
#include "palette.h"
#include <SDL/SDL.h>
#include <arpa/inet.h>

struct VgaState vga_state;

void vga_init(bool new_mode) {
	int i;
	uint16_t *pal_data;

	if (!new_mode) {
		vga_state.buff = mem_decode_addr(0x00081800, &i);
		vga_state.pal = mem_decode_addr(0x00081400, &i);
		vga_state.vga_width = 640;
	} else {
		vga_state.buff = mem_decode_addr(0x00080000, &i);
		vga_state.pal = malloc(256*2);
		vga_state.vga_width = 800;
		pal_data = (void *) vga_state.pal;
		for (i = 0; i < 256; i++)
			pal_data[i] = palette_data[i];
	}


	vga_state.screen = SDL_SetVideoMode(vga_state.vga_width, 480, 16, SDL_SWSURFACE);
	vga_state.pixbuf = SDL_CreateRGBSurface(0, vga_state.vga_width, 480, 16, 0xF800, 0x7E0, 0x1F, 0x0);

	for (i = 0; i < 800*525; i++)
		vga_state.buff[i] = 0;

	vga_state.line = 0;
	vga_state.ticks = SDL_GetTicks();
	vga_state.reg.window_x = vga_state.reg.window_y = 0;

	return;
}


void vga_render_line() {
	uint8_t *ptr;
	int i, ram_pos;
	uint16_t pix;
	uint16_t *next_pb = vga_state.pixbuf->pixels;

	if (vga_state.line < 13 || vga_state.line >= (480+13))
		goto end;
	
	#ifdef VGA_WINDOW_SCROLL
	ram_pos = (vga_state.line - 13 + vga_state.reg.window_y);
	if (ram_pos >= 525) ram_pos %= 525;
	ram_pos *= 800;
	ram_pos += vga_state.reg.window_x;
	#else
	ram_pos = (vga_state.line - 13) * vga_state.vga_width;
	#endif
	next_pb += (vga_state.line - 13) * vga_state.vga_width;
	
	for (i = 0; i < vga_state.vga_width; i++) {
		ptr = &vga_state.pal[vga_state.buff[ram_pos] * 2];
		if (!mem->new_map) {
			pix = ((*ptr++) << 8);
			pix |= *ptr;
		} else {
			pix = *ptr;
			pix |= (*(ptr + 1) << 8);
		}
		*next_pb = pix;
		ram_pos++, next_pb++;
		#ifdef VGA_WINDOW_SCROLL
		if (ram_pos >= 800*525)
			ram_pos = 0;
		#endif
	}

	end:

	vga_state.line++;
	if (vga_state.line == 525) {
		chipset_int_set(CHIPSET_INT_NUM_VGA_VSYNC, 1);
		
		vga_state.line = 0;
		SDL_BlitSurface(vga_state.pixbuf, NULL, vga_state.screen, NULL);
		SDL_Flip(vga_state.screen);
		
		if (SDL_GetTicks() - vga_state.ticks <= 20)
			SDL_Delay(20 - (SDL_GetTicks() - vga_state.ticks));
		else
			fprintf(stderr, "Running below framerate (ft=%i ms)\n", SDL_GetTicks() - vga_state.ticks);
		vga_state.ticks = SDL_GetTicks();
	}

	return;
}
