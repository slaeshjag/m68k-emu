#include <stdint.h>
#include "vga.h"
#include "mem.h"
#include <SDL/SDL.h>
#include <arpa/inet.h>


void vga_init() {
	int i, j;

	vga_state.buff = mem_decode_addr(0x00080800, &i, &j);
	vga_state.pal = mem_decode_addr(0x00080400, &i, &j);
	
	vga_state.screen = SDL_SetVideoMode(640, 480, 16, SDL_SWSURFACE);
	vga_state.pixbuf = SDL_CreateRGBSurface(0, 640, 480, 16, 0xF800, 0x7E0, 0x1F, 0x0);

	for (i = 0; i < 640*480; i++)
		vga_state.buff[i] = 0;

	/* TODO: Load palette from file */

	vga_state.line = 0;
	vga_state.ticks = SDL_GetTicks();

	return;
}


void vga_render_line() {
	int i, ram_pos;
	uint16_t *next_pb = vga_state.pixbuf->pixels;

	if (vga_state.line < 13 || vga_state.line > (480+13))
		goto end;
	
	ram_pos = (vga_state.line - 13) * 640;
	next_pb += ram_pos;
	
	for (i = 0; i < 640; i++) {
		*next_pb = ntohs(vga_state.pal[vga_state.buff[ram_pos]]);
		ram_pos++, next_pb++;
	}

	end:

	vga_state.line++;
	if (vga_state.line == 525) {
		/* TODO: Interrupt for vsync? */
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
