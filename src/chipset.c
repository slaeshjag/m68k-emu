#include "chipset.h"
#include "vga.h"


static int interrupt[8];

int chipset_get_interrupt_level() {
	int i;

	if (!interrupt[0])
		return 0;

	for (i = 0; i < 7; i++)
		if (interrupt[7-i])
			return 7 - 1;
	return 0;
}


void chipset_int_set(int int_no, int set_unset) {
	int i;

	interrupt[int_no] = set_unset;

	if (!interrupt[0]) {
		fprintf(stderr, "interrupts not enabled\n");
		//m68k_set_irq(0);
		return;
	} 

	for (i = 0; i < 7; i++) {
		if (interrupt[7-i]) {
			//m68k_set_irq(7 - i);
			return;
		}
	}

	//m68k_set_irq(0);
}


void chipset_write_io(unsigned int addr, unsigned int data) {
	addr &= 0xFC;
	//fprintf(stderr, "write_io %u %u\n", addr, data);
	
	switch(addr) {
		case 0x0:
			chipset_int_set(0, data & 1);
			return;
		case 0x4:
			chipset_int_set(CHIPSET_INT_NUM_VGA_VSYNC, 0);
			return;
		case 0x8:
			vga_state.reg.window_x = data & 0x3FF;
			return;
		case 0xC:
			vga_state.reg.window_y = data & 0x3FF;
			return;
		case 0x10:
			vga_state.reg.sprite_x = data & 0x3FF;
			return;
		case 0x14:
			vga_state.reg.sprite_y = data & 0x3FF;
			return;
	}
}
