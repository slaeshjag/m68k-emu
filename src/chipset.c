#include "chipset.h"
#include "m68k.h"
#include "vga.h"

static int interrupt[8];


void chipset_int_set(int int_no, int set_unset) {
	int i;
	interrupt[int_no] = set_unset;

	if (!interrupt[0]) {
		fprintf(stderr, "interrupts not enabled\n");
		m68k_set_irq(0);
		return;
	} 

	for (i = 0; i < 7; i++) {
		if (interrupt[7-i]) {
			m68k_set_irq(7 - i);
			return;
		}
	}

	m68k_set_irq(0);
}


void chipset_write_io(unsigned int addr, unsigned int data) {
	addr &= 0xFC;
	fprintf(stderr, "write_io %u %u\n", addr, data);
	if (!addr)		/* IRQ enable register */
		return chipset_int_set(0, data & 1);
	if (addr == 0x4)	/* VGA VSync acknowledge */
		return chipset_int_set(ChipsetIntNumVGAVSync, 0);
	if (addr == 0x8) {	/* VGA window scroll X */
		vga_state.reg.window_x = data & 0x3FF;
		return;
	} if (addr == 0xC) {	/* VGA window scroll Y */
		vga_state.reg.window_y = data & 0x3FF;
		return;
	} if (addr == 0x10) {	/* VGA sprite X */
		vga_state.reg.sprite_x = data & 0x3FF;
		return;
	} if (addr == 0x14) {	/* VGA sprite Y */
		vga_state.reg.sprite_y = data & 0x3FF;
		return;
	}

	return;
}
