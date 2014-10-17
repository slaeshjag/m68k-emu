#ifndef __VGA_H__
#define	__VGA_H__

#include <SDL/SDL.h>

void vga_init();
void vga_render_line();

struct {
	unsigned char		*buff;
	unsigned short		*pal;
	int			line;

	SDL_Surface		*screen;
	SDL_Surface		*pixbuf;
	int			ticks;
} vga_state;


#endif
