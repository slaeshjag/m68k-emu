#ifndef __VGA_H__
#define	__VGA_H__

#include <stdint.h>
#include <stdbool.h>
#include <SDL/SDL.h>

#define	VGA_WINDOW_SCROLL

void vga_init(bool new_mode);
void vga_render_line();

struct VgaRegister {
	unsigned short		window_x;
	unsigned short		window_y;
	unsigned short		sprite_x;
	unsigned short		sprite_y;
};

struct VgaState {
	int			vga_width;
	uint8_t		*buff;
	uint8_t		*pal;
	int			line;

	struct VgaRegister	reg;

	SDL_Surface		*screen;
	SDL_Surface		*pixbuf;
	int			ticks;

	uint8_t			keyboard;
};


extern struct VgaState vga_state;

#endif
