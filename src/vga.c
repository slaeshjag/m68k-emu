#include <stdint.h>
#include <stdbool.h>
#include "vga.h"
#include "mem.h"
#include "chipset.h"
#include "palette.h"
#include "timer.h"
#include "uart.h"
#include "interrupt.h"
#include <SDL/SDL.h>
#include <arpa/inet.h>

#include "spi_keyboard.h"

struct VgaState vga_state = {
	.vga_reg = {
		.cursor_x = 800,
		.cursor_y = 480,
		.cursor_data =  {
			3,0,0,0,0,0,0,0,
			3,3,0,0,0,0,0,0,
			3,2,3,0,0,0,0,0,
			3,2,2,3,0,0,0,0,
			3,2,2,2,3,0,0,0,
			3,2,2,2,2,3,0,0,
			3,2,2,2,2,2,3,0,
			3,2,2,2,2,2,2,3,
			3,2,2,2,2,3,3,3,
			3,2,3,2,2,3,0,0,
			3,3,0,3,2,2,3,0,
			0,0,0,3,2,2,3,0,
			0,0,0,0,3,2,2,3,
			0,0,0,0,3,2,2,3,
			0,0,0,0,0,3,3,0,
			0,0,0,0,0,0,0,0
		},
	},
};

void vga_init(bool new_mode) {
	int i;
	uint16_t *pal_data;

	vga_state.keyboard = 0;
	vga_state.vga_reg.mode = 1;

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

	//for (i = 0; i < 800*525; i++)
	//	vga_state.buff[i] = 0;

	vga_state.line = 0;
	vga_state.ticks = SDL_GetTicks();
	vga_state.reg.window_x = vga_state.reg.window_y = 0;
	SDL_ShowCursor(false);

	return;
}


void vga_render_line() {
	uint8_t *ptr, data;
	int i, ram_pos;
	uint16_t pix, white = 0xFFFF, black = 0x0000;
	uint16_t *next_pb = vga_state.pixbuf->pixels;

	timer_run(1000);
	if (vga_state.line < 13 || vga_state.line >= (480+13))
		goto end;
	
	#ifdef VGA_WINDOW_SCROLL
	ram_pos = (vga_state.line - 13 + vga_state.reg.window_y);
	if (ram_pos >= 525) ram_pos %= 525;
	ram_pos *= 800;
	ram_pos += vga_state.reg.window_x;
	#else
	if (vga_state.vga_reg.mode & 2)
		ram_pos = (vga_state.line - 13)/2 * vga_state.vga_width/2 + !!(vga_state.vga_reg.mode & 4) * (1 << 17);
	else
		ram_pos = (vga_state.line - 13) * vga_state.vga_width;
	#endif
	next_pb += (vga_state.line - 13) * vga_state.vga_width;
	
	for (i = 0; i < vga_state.vga_width; i++) {
		if (!(vga_state.vga_reg.mode & 1)) {
			pix = 0;
			goto got_pix;
		}

		data = vga_state.buff[ram_pos];
		ptr = &vga_state.pal[data * 2];
		if (vga_state.vga_reg.cursor_x <= i && vga_state.vga_reg.cursor_x + VGA_CURSOR_W > i && vga_state.vga_reg.cursor_y <= vga_state.line - 13 && vga_state.vga_reg.cursor_y + VGA_CURSOR_H > vga_state.line - 13) {
			int x, y;
			x = i - vga_state.vga_reg.cursor_x;
			y = vga_state.line - 13 - vga_state.vga_reg.cursor_y;
			if (!(vga_state.vga_reg.cursor_data[VGA_CURSOR_W * y + x] & 02))
				goto no_cursor;
			if (vga_state.vga_reg.cursor_data[VGA_CURSOR_W * y + x] & 01)
				ptr = (void *) &white;
			else
				ptr = (void *) &black;
		}

	no_cursor:
		if (!mem->new_map) {
			pix = ((*ptr++) << 8);
			pix |= *ptr;
		} else {
			pix = *ptr;
			pix |= (*(ptr + 1) << 8);
		}
	got_pix:
		*next_pb = pix;
		if (vga_state.vga_reg.mode & 2) {
			if (i & 1)
				ram_pos++;
		} else
			ram_pos++;
		next_pb++;
		#ifdef VGA_WINDOW_SCROLL
		if (ram_pos >= 800*525)
			ram_pos = 0;
		#endif
	}

	end:

	vga_state.line++;
	if (vga_state.line == 525) {
		SDL_Event event_sdl;
		//chipset_int_set(CHIPSET_INT_NUM_VGA_VSYNC, 1);
		//~ static int count = 10;
		//~ if(count-- == 0) {
			interrupt_trig(CHIPSET_INT_NUM_VGA_VSYNC);
			//~ count = 10;
		//~ }
		
		vga_state.line = 0;
		SDL_BlitSurface(vga_state.pixbuf, NULL, vga_state.screen, NULL);
		SDL_Flip(vga_state.screen);
		
		if (SDL_GetTicks() - vga_state.ticks <= 20)
			SDL_Delay(20 - (SDL_GetTicks() - vga_state.ticks));
		//else
			//fprintf(stderr, "Running below framerate (ft=%i ms)\n", SDL_GetTicks() - vga_state.ticks);
		vga_state.ticks = SDL_GetTicks();
		
		while (SDL_PollEvent(&event_sdl)) {
			switch (event_sdl.type) {
				case SDL_KEYDOWN:
					spi_keyboard_push_event(true, event_sdl.key.keysym.scancode);
					break;
				case SDL_KEYUP:
					spi_keyboard_push_event(false, event_sdl.key.keysym.scancode);
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event_sdl.button.button == SDL_BUTTON_LEFT)
						spi_keyboard_push_event(true, 192);
					if (event_sdl.button.button == SDL_BUTTON_MIDDLE)
						spi_keyboard_push_event(true, 193);
					if (event_sdl.button.button == SDL_BUTTON_RIGHT)
						spi_keyboard_push_event(true, 194);
					break;
				case SDL_MOUSEBUTTONUP:
					if (event_sdl.button.button == SDL_BUTTON_LEFT)
						spi_keyboard_push_event(false, 192);
					if (event_sdl.button.button == SDL_BUTTON_MIDDLE)
						spi_keyboard_push_event(false, 193);
					if (event_sdl.button.button == SDL_BUTTON_RIGHT)
						spi_keyboard_push_event(false, 194);
					break;
				case SDL_MOUSEMOTION:
					spi_keyboard_digitizer_set(event_sdl.motion.x, event_sdl.motion.y);
					break;
				case SDL_QUIT:
					exit(0);
					break;
				default:
					break;
			}

		}
	}

	vga_state.keyboard |= 0xC0;
	uart_irq_handle();

	return;
}


void vga_io_write(uint32_t addr, uint32_t data) {
	addr &= 0xFC;
	if (addr == 0) {
		if ((data & 1) && !(vga_state.vga_reg.mode & 1))
			fprintf(stderr, "[VGA] MAIN SCREEN TURN ON");
		vga_state.vga_reg.mode = data;
	} if (addr == 4) {
		fprintf(stderr, "[VGA] Brightness is now %i\n", data);
	} if (addr == 0x10) {
		vga_state.vga_reg.pal_pos = data & 0xFF;
	} if (addr == 0x14) {
		uint16_t *pal_addr = (void *) vga_state.pal;
		pal_addr[vga_state.vga_reg.pal_pos & 0xFF] = data;
		//vga_state.vga_reg.pal_pos++;
	} if (addr == 0x20) {
		vga_state.vga_reg.cursor_pos = data;
	} if (addr == 0x24) {
		vga_state.vga_reg.cursor_data[vga_state.vga_reg.cursor_pos % (VGA_CURSOR_W * VGA_CURSOR_H)] = data;
		vga_state.vga_reg.cursor_pos++;
	} if (addr == 0x28) {
		vga_state.vga_reg.cursor_x = data & 0x3FF;
	} if (addr == 0x2C) {
		vga_state.vga_reg.cursor_y = data & 0x1FF;
	}
}
