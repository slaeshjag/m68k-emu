#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "vga.h"
#include "spi_keyboard.h"
#include "scancodes.h"
#include "audio.h"

static int ready = 0;
static enum SPIKeyboardCommand cmd = SPI_KEYBOARD_CMD_NONE;
static int counter = 0;


static struct {
	uint8_t code[16];
	int cur;
	int next;
	uint8_t flags;

	int mouse_x;
	int mouse_y;
	int new_mouse_x;
	int new_mouse_y;
	int byte_count;
} keyboard_buffer = { { 0 }, 0, 0 };


#define	MAKE_KBD_CODE(col, row) (((col) * 4 + (row)) | 0x100)

static const int sequence_lookup[17] = { 5, 1, 0, 2, 6, 7, 3, 4, 8, 14, 10, 13, 9, 15, 11, 12, 16 };


static const uint16_t scancode_lookup[256] = {
	[SCANCODE_1] = MAKE_KBD_CODE(0, 0),
	[SCANCODE_2] = MAKE_KBD_CODE(1, 0),
	[SCANCODE_3] = MAKE_KBD_CODE(2, 0),
	[SCANCODE_4] = MAKE_KBD_CODE(3, 0),
	[SCANCODE_5] = MAKE_KBD_CODE(4, 0),
	[SCANCODE_6] = MAKE_KBD_CODE(5, 0),
	[SCANCODE_7] = MAKE_KBD_CODE(6, 0),
	[SCANCODE_8] = MAKE_KBD_CODE(7, 0),
	[SCANCODE_9] = MAKE_KBD_CODE(8, 0),
	[SCANCODE_0] = MAKE_KBD_CODE(9, 0),
	[SCANCODE_PLUS] = MAKE_KBD_CODE(10, 0),
	[SCANCODE_ACUTE] = MAKE_KBD_CODE(11, 0),
	[SCANCODE_BACKSPACE] = MAKE_KBD_CODE(12, 0),
	[SCANCODE_CONTEXT] = MAKE_KBD_CODE(13, 0),
	[SCANCODE_ALT] = MAKE_KBD_CODE(14, 0),
	[SCANCODE_UP] = MAKE_KBD_CODE(15, 0),
	[SCANCODE_LCTRL] = MAKE_KBD_CODE(16, 0),
	[SCANCODE_Q] = MAKE_KBD_CODE(0, 1),
	[SCANCODE_W] = MAKE_KBD_CODE(1, 1),
	[SCANCODE_E] = MAKE_KBD_CODE(2, 1),
	[SCANCODE_R] = MAKE_KBD_CODE(3, 1),
	[SCANCODE_T] = MAKE_KBD_CODE(4, 1),
	[SCANCODE_Y] = MAKE_KBD_CODE(5, 1),
	[SCANCODE_U] = MAKE_KBD_CODE(6, 1),
	[SCANCODE_I] = MAKE_KBD_CODE(7, 1),
	[SCANCODE_O] = MAKE_KBD_CODE(8, 1),
	[SCANCODE_P] = MAKE_KBD_CODE(9, 1),
	[SCANCODE_ARING] = MAKE_KBD_CODE(10, 1),
	[SCANCODE_DEADUML] = MAKE_KBD_CODE(11, 1),
	[SCANCODE_ENTER] = MAKE_KBD_CODE(12, 1),
	[SCANCODE_ALTGR] = MAKE_KBD_CODE(13, 1),
	[SCANCODE_LESSTHAN] = MAKE_KBD_CODE(14, 1),
	[SCANCODE_RIGHT] = MAKE_KBD_CODE(15, 1),
	[SCANCODE_RMB] = MAKE_KBD_CODE(16, 1),
	[SCANCODE_A] = MAKE_KBD_CODE(0, 2),
	[SCANCODE_S] = MAKE_KBD_CODE(1, 2),
	[SCANCODE_D] = MAKE_KBD_CODE(2, 2),
	[SCANCODE_F] = MAKE_KBD_CODE(3, 2),
	[SCANCODE_G] = MAKE_KBD_CODE(4, 2),
	[SCANCODE_H] = MAKE_KBD_CODE(5, 2),
	[SCANCODE_J] = MAKE_KBD_CODE(6, 2),
	[SCANCODE_K] = MAKE_KBD_CODE(7, 2),
	[SCANCODE_L] = MAKE_KBD_CODE(8, 2),
	[SCANCODE_OUML] = MAKE_KBD_CODE(9, 2),
	[SCANCODE_AUML] = MAKE_KBD_CODE(10, 2),
	[SCANCODE_APOS] = MAKE_KBD_CODE(11, 2),
	[SCANCODE_ESC] = MAKE_KBD_CODE(12, 2),
	[SCANCODE_TAB] = MAKE_KBD_CODE(13, 2),
	[SCANCODE_PARAGRAPH] = MAKE_KBD_CODE(14, 2),
	[SCANCODE_DOWN] = MAKE_KBD_CODE(15, 2),
	[SCANCODE_MMB] = MAKE_KBD_CODE(16, 2),

	[SCANCODE_LSHIFT] = MAKE_KBD_CODE(0, 3),
	[SCANCODE_Z] = MAKE_KBD_CODE(1, 3),
	[SCANCODE_X] = MAKE_KBD_CODE(2, 3),
	[SCANCODE_C] = MAKE_KBD_CODE(3, 3),
	[SCANCODE_V] = MAKE_KBD_CODE(4, 3),
	[SCANCODE_B] = MAKE_KBD_CODE(5, 3),
	[SCANCODE_N] = MAKE_KBD_CODE(6, 3),
	[SCANCODE_M] = MAKE_KBD_CODE(7, 3),
	[SCANCODE_COMMA] = MAKE_KBD_CODE(8, 3),
	[SCANCODE_PERIOD] = MAKE_KBD_CODE(9, 3),
	[SCANCODE_DASH] = MAKE_KBD_CODE(10, 3),
	[SCANCODE_RSHIFT] = MAKE_KBD_CODE(11, 3),
	[SCANCODE_CAPSLOCK] = MAKE_KBD_CODE(12, 3),
	[SCANCODE_SPACE] = MAKE_KBD_CODE(13, 3),
	//[SCANCODE_LSHIFT] = MAKE_KBD_CODE(14, 3), // Left FN not mapped //
	[SCANCODE_LEFT] = MAKE_KBD_CODE(15, 3),
	[SCANCODE_LMB] = MAKE_KBD_CODE(16, 3),
};


void spi_keyboard_push_event(bool press, int scancode) {
	//fprintf(stderr, "scancode is %i\n", scancode);
	if (scancode > 255)
		return;
	if (!(scancode_lookup[scancode] & 0x100))
		return;
	keyboard_buffer.code[keyboard_buffer.next++] = (sequence_lookup[(scancode_lookup[scancode] & 0xFF) >> 2] << 2) | (scancode_lookup[scancode] & 0x3) | (press?0:0x80);
	if (keyboard_buffer.next == 16)
		keyboard_buffer.next = 0;
	keyboard_buffer.flags |= (1 << SPI_KEYBOARD_KB_BIT);
}


uint8_t spi_keyboard_pop_event() {
	if (keyboard_buffer.cur == 16)
		keyboard_buffer.cur = 0;
	if (keyboard_buffer.cur == keyboard_buffer.next) {
		keyboard_buffer.flags &= ~(1 << SPI_KEYBOARD_KB_BIT);
		return 0xFF;
	}
	return keyboard_buffer.code[keyboard_buffer.cur++];
}


void spi_keyboard_digitizer_set(int x, int y) {
	keyboard_buffer.new_mouse_x = x;
	keyboard_buffer.new_mouse_y = y;
	if (!(keyboard_buffer.flags & (1 << SPI_KEYBOARD_DIGITIZER_BIT)))
		keyboard_buffer.mouse_x = x, keyboard_buffer.mouse_y = y;
	keyboard_buffer.flags |= (1 << SPI_KEYBOARD_DIGITIZER_BIT);
}


uint8_t spi_keyboard_control() {
	fprintf(stderr, "[KBD] control\n");
	return 0x0;
}


uint8_t spi_keyboard_send_receive(uint8_t data) {
	if (!ready)
		return ~0;
	counter++;
	
	switch (cmd) {
		case SPI_KEYBOARD_CMD_NONE:
			cmd = data;
			keyboard_buffer.byte_count = 0;
			return 0xFF;
		case SPI_KEYBOARD_CMD_STATUS:
			// TODO: Release IRQ
			cmd = SPI_KEYBOARD_CMD_NONE;
			return keyboard_buffer.flags;
		case SPI_KEYBOARD_CMD_CONTROL:
			cmd = SPI_KEYBOARD_CMD_NONE;
			return spi_keyboard_control();
		case SPI_KEYBOARD_CMD_KEYBOARD_EVENT:
			data = spi_keyboard_pop_event();
			if (data == 0xFF)
				cmd = SPI_KEYBOARD_CMD_NONE;
			return data;
		case SPI_KEYBOARD_CMD_VOLUME:
			keyboard_buffer.byte_count++;
			if (keyboard_buffer.byte_count == 1) {
				audio_set_volume_l(data);
				return 0xFF;
			} else if (keyboard_buffer.byte_count == 2) {
				audio_set_volume_r(data);
				cmd = SPI_KEYBOARD_CMD_NONE;
				return 0xFF;
			}
		case SPI_KEYBOARD_CMD_POWER_OFF:
			cmd = SPI_KEYBOARD_CMD_NONE;
			// TODO: Power off
			return 0xFF;
		case SPI_KEYBOARD_CMD_DIGITIZER:
			keyboard_buffer.byte_count++;
			if (keyboard_buffer.byte_count == 1)
				return keyboard_buffer.mouse_x >> 8;
			if (keyboard_buffer.byte_count == 2)
				return keyboard_buffer.mouse_x & 0xFF;
			if (keyboard_buffer.byte_count == 3)
				return keyboard_buffer.mouse_y >> 8;
			if (keyboard_buffer.byte_count == 4) {
				data = keyboard_buffer.mouse_y & 0xFF;
				if ((keyboard_buffer.new_mouse_x == keyboard_buffer.mouse_x) && (keyboard_buffer.new_mouse_y == keyboard_buffer.mouse_y))
					keyboard_buffer.flags &= ~(1 << SPI_KEYBOARD_DIGITIZER_BIT);
				else
					keyboard_buffer.mouse_x = keyboard_buffer.new_mouse_x, keyboard_buffer.mouse_y = keyboard_buffer.new_mouse_y;
				cmd = SPI_KEYBOARD_CMD_NONE;
				
				return data;
			}
			
			return 0xFF;
			
		default:
			fprintf(stderr, "[KBD] Unhandled command byte 0x%X\n", data);
			cmd = SPI_KEYBOARD_CMD_NONE;
			return 0xFF;
	}

	return ~vga_state.keyboard;
}


void spi_keyboard_notify_selected(bool selected) {
	if (selected) {
		ready = 1;
		counter = 0;
	} else
		ready = 0, cmd = SPI_KEYBOARD_CMD_NONE;
}
