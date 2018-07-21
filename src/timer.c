#include <stdio.h>
#include "timer.h"
#include "interrupt.h"
#include "chipset.h"

static struct Timer timer;

static void timer_trigger(int timer_no) {
	timer.interrupt_flag |= (1 << timer_no);
	if (timer.interrupt_enable & (1 << timer_no)) {
		interrupt_trig(CHIPSET_INT_NUM_TIMER);
	}
}

void timer_run(int cycles) {
	int i, add;
	
	add = (timer.prescaler_count + cycles) >> 16;
	timer.prescaler_count += cycles;

	for (i = 0; i < 4; i++) {
		if (!(timer.timer[i].status & 1))
			continue;
		timer.timer[i].count += add;
		if (timer.timer[i].count >= timer.timer[i].compare) {
			timer_trigger(i);
			if (timer.timer[i].status & 2) {
				if (timer.timer[i].compare)
					timer.timer[i].count %= timer.timer[i].compare;
			} else {
				timer.timer[i].status &= 0x2;
				timer.timer[i].count = timer.timer[i].compare;
			}
		}
	}
}


uint32_t timer_do_read_lword(uint32_t addr) {
	addr &= 0xFF;
	if ((addr % 0xFC) == 0) {
		return timer.interrupt_enable | (timer.prescaler << 16);
	} else if ((addr & 0xFC) == 0x4) {
		return timer.interrupt_flag;
	} else if ((addr & 0xFC) == 0x10) {
		return timer.timer[0].status;
	} else if ((addr & 0xFC) == 0x14) {
		return timer.timer[0].count;
	} else if ((addr & 0xFC) == 0x18) {
		return timer.timer[0].compare;
	} else if ((addr & 0xFC) == 0x20) {
		return timer.timer[1].status;
	} else if ((addr & 0xFC) == 0x24) {
		return timer.timer[1].count;
	} else if ((addr & 0xFC) == 0x28) {
		return timer.timer[1].compare;
	} else if ((addr & 0xFC) == 0x30) {
		return timer.timer[2].status;
	} else if ((addr & 0xFC) == 0x34) {
		return timer.timer[2].count;
	} else if ((addr & 0xFC) == 0x38) {
		return timer.timer[2].compare;
	} else if ((addr & 0xFC) == 0x40) {
		return timer.timer[3].status;
	} else if ((addr & 0xFC) == 0x44) {
		return timer.timer[3].count;
	} else if ((addr & 0xFC) == 0x48) {
		return timer.timer[3].compare;
	} else
		return ~0;
}


void timer_do_write_lword(uint32_t addr, uint32_t data) {
	addr &= 0xFC;

	if ((addr & 0xFC) == 0x0) {
		timer.interrupt_enable = data & 0xF;
		timer.prescaler = data >> 16;
	} else if ((addr & 0xFC) == 0x4) {
		timer.interrupt_flag &= ~(data & 0xF);
	} else if (addr >= 0x10) {
		if ((addr & 0xC) == 0x4)
			timer.timer[((addr & 0x70) >> 4) - 1].count = data;
		else if ((addr & 0xC) == 0x8)
			timer.timer[((addr & 0x70) >> 4) - 1].compare = data;
		else if (!(addr & 0xC)) {
			uint32_t oldstat = timer.timer[((addr & 0x70) >> 4) - 1].status;
			if ((!(oldstat & 1)) && (data & 0x1)) {
				timer.timer[((addr & 0x70) >> 4) - 1].count = 0;
			}

			timer.timer[((addr & 0x70) >> 4) - 1].status = data & 3;
		}
	}
}
