#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "mem.h"
#include "cpu/m68000.h"

int get_interrupt_level(void) {
	return 0;
}

uint32_t get_long(uint32_t addr) {
	uint32_t ret = m68k_read_memory_32(addr);
	//fprintf(stderr, "read 32bit 0x%X from 0x%X\n", ret, addr);
	return ret;
}
uint32_t get_word(uint32_t addr) {
	uint32_t ret = m68k_read_memory_16(addr);
	//fprintf(stderr, "read 16bit 0x%X from 0x%X\n", ret, addr);
	return ret;
}
uint32_t get_byte(uint32_t addr) {
	uint32_t ret = m68k_read_memory_8(addr);
	//fprintf(stderr, "read 8bit 0x%X from 0x%X\n", ret, addr);
	return ret;
}
void put_long(uint32_t addr, uint32_t l) {
	m68k_write_memory_32(addr, l);
}
void put_word(uint32_t addr, uint32_t w) {
	if(addr > 0x400 && addr < 0x420) fprintf(stderr, "palott!!1 0x%X wrote 16bit value 0x%X\n", addr, w);
	m68k_write_memory_16(addr, w);
}
void put_byte(uint32_t addr, uint32_t b) {
	m68k_write_memory_8(addr, b);
}

uint8_t *get_real_address(uint32_t addr) {
	int write;
	return mem_decode_addr(addr, &write);
}
int valid_address(uint32_t addr, uint32_t size) {
	int write;
	return mem_decode_addr(addr, &write) == NULL ? 0 : 1;
}
