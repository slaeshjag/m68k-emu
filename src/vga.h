#ifndef __VGA_H__
#define	__VGA_H__

#include <stdint.h>
#include <stdbool.h>
#include <SDL/SDL.h>

//#define	VGA_WINDOW_SCROLL

// h4xxel can't make up his mind whether or not the cursor is 8 pixels wide
// I like to think that hardware is fixed, and won't be changed when it has been implemented,
// but I think someone have become spoiled with FPGAs
#define	VGA_CURSOR_W	8
#define	VGA_CURSOR_H	16

void vga_init(bool new_mode);
void vga_render_line();
void vga_io_write(uint32_t addr, uint32_t data);

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
	struct {
		uint32_t	pal_pos;
		uint32_t	cursor_pos;
		uint32_t	cursor_x;
		uint32_t	cursor_y;
		uint32_t	mode;
		uint8_t		cursor_data[VGA_CURSOR_W * VGA_CURSOR_H];
	} vga_reg;
};


extern struct VgaState vga_state;

#endif
