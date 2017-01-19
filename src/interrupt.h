#ifndef INTERRUPT_H__
#define	INTERRUPT_H__

#include <stdint.h>
#include <stdbool.h>

void interrupt_set_boot_switch(int sw);
void interrupt_trig(int i);
int interrupt_get_ipl();
uint32_t interrupt_do_read_lword(uint32_t addr);
void interrupt_do_write_lword(uint32_t addr, uint32_t data);

#endif
