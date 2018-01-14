#ifndef TIMER_H__
#define	TIMER_H__

#include <stdint.h>

struct TimerEntry {
	uint32_t			count;
	uint32_t			compare;
	uint32_t			status;
};

struct Timer {
	uint32_t			interrupt_enable;
	uint32_t			interrupt_flag;

	/* TimerEntry 0 på 0x10, 1 på 0x20 ... */
	struct TimerEntry		timer[4];
};

void timer_run(int cycles);
uint32_t timer_do_read_lword(uint32_t addr);
void timer_do_write_lword(uint32_t addr, uint32_t data);

#endif
