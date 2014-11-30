#include "chipset.h"
#include "vga.h"
#include "spi.h"


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
	} if (addr == 0x20) {	/* SPI mem counter */
		spi_state.mem_counter = data & 0x3FF;
		return;
	} if (addr == 0x24) {	/* SPI line select */
		spi_state.line_select = data & 0x1F;
		return;
	} if (addr == 0x28) {	/* SPI status register */
		spi_state.reg = data & 0xF;
		return;
	} if (addr == 0x2C) {	/* SPI interrupt acknowledge */
		return chipset_int_set(ChipsetIntNumSPIDone, 0);
	}

	fprintf(stderr, "Wrote invalid IO-port 0x%X\n", addr);

	return;
}


uint32_t chipset_read_io(unsigned int addr) {
	addr &= 0xFC;
	
	if (addr == 0x24) {
		return spi_state.reg & 0x1F;
	} if (addr == 0x28) {
		return spi_state.reg & 0xF;
	}

	fprintf(stderr, "Read invalid IO-port 0x%X\n", addr);
}
