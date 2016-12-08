#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "vga.h"

static int ready = 0;

uint8_t spi_keyboard_send_receive(uint8_t data) {
	if (!ready)
		return ~0;
	ready = 0;
	return ~vga_state.keyboard;
}


void spi_keyboard_notify_selected(bool selected) {
	if (selected)
		ready = 1;
	else ready = 0;
}
