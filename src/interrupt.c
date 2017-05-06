#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

struct InterruptState {
	uint32_t		int_flag[32];
	uint32_t		priority[32];
	int			bootswitch;
} is;


void interrupt_set_boot_switch(int sw) {
	is.bootswitch = sw << 16;
}


void interrupt_trig(int i) {
	is.int_flag[i] |= 1;
}


int interrupt_get_ipl() {
	int i, ipl;

	for (i = 1, ipl = 0; i < 32; i++) {
		if ((is.priority[i] & 0x7) < ipl)
			continue;
		if (is.int_flag[i])
			ipl = is.priority[i] & 0x7;
	}
	
	return ipl;	
}


uint32_t interrupt_do_read_lword(uint32_t addr) {
	addr &= 0xFC;
//	fprintf(stderr, "Chipset interrupt read 0x%X: 0x%X\n", addr, is.bootswitch);
	if (!addr)
		return 0xFFFFFFFF;
	if (addr == 0x80)
		return is.bootswitch;
	if (addr > 0x80)
		return is.int_flag[(addr & 0x7C) >> 2];
	else
		return is.priority[addr >> 2];
}


void interrupt_do_write_lword(uint32_t addr, uint32_t data) {
	//printf("write to interrupt controller addr = 0x%X\n", addr);
	addr &= 0xFC;
	if (!addr)
		return;
	if (addr == 0x80)
		return;
	if (addr > 0x80) {
		//printf("cleared interrupt flag %i\n", (addr & ~0x80) >> 2);
		is.int_flag[(addr & 0x7C) >> 2] = data;
	} else {
		//printf("enabled interrupt %i\n", addr >> 2);
		is.priority[addr >> 2] = data;
	}
}
