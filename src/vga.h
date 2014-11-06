#ifndef __VGA_H__
#define	__VGA_H__

#include <SDL/SDL.h>

#define	VGA_WINDOW_SCROLL

void vga_init();
void vga_render_line();

struct VgaRegister {
	unsigned short		window_x;
	unsigned short		window_y;
	unsigned short		sprite_x;
	unsigned short		sprite_y;
};

struct {
	unsigned char		*buff;
	unsigned short		*pal;
	int			line;

	struct VgaRegister	reg;

	SDL_Surface		*screen;
	SDL_Surface		*pixbuf;
	int			ticks;
} vga_state;


#endif
