/* Memory map								*
** 0x00000000 - 0x0007FFFF	LLRAM					*
** 0x00080000 - 0x000FFFFF	ROM					*
** 0x10000000 - 0x13FFFFFF	Main RAM				*
** 0x20000000 - 0x2FFFFFFF	Chipset control registers		*/

/* LLRAM Map								*
** 0x00000000 - 0x000003FF	Reserved for CPU vectors		*
** 0x00001400 - 0x000017FF	Palette RAM				*
** 0x00001800 - 0x0006809F	VGA framebuffer	(800x525)		*
** 0x00068800 - 0x000688FF	Hardware sprite	(16x16)			*
** 0x00069000 - 0x000697FF	Audio buffer 1 (20 ms)			*
** 0x00069800 - 0x00069FFF	Audio buffer 2 (20 ms)			*
** 0x0006A000 - 0x00077FFF	Available				*
** 0x00078000 - 0x00079FFF	SPI send buffer 0			*
** 0x0007A000 - 0x0007BFFF	SPI send buffer 1			*
** 0x0007C000 - 0x0007DFFF	SPI recv buffer 0			*
** 0x0007E000 - 0x0007FFFF	SPI recv buffer 1			*/

#include "mem.h"
#include "chipset.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

struct Mem *mem;

uint32_t junk;

void *mem_decode_addr(unsigned int address, int *write) {
	*write = 1;

	if (address < 0x8) {
		*write = 0;
		return mem->rom + address;
	} else if (address < 0x80000) {
		return mem->llram + address;
	}

	if (address < 0x100000) {
		*write = 0;
		return mem->rom + (address - 0x80000);
	} if (address >= 0x10000000 && address <= 0x13FFFFFF)
		return mem->mram + (address - 0x10000000);
	if ((address & 0xF0000000) == 0x20000000) {
		/* Dummy value */
		*write = 0;
		return mem->llram;
	}
	fprintf(stderr, "ERROR: Invalid address %X\n", address);
	exit(-1);
}


unsigned int m68k_read_memory_8(unsigned int address) {
	uint8_t *ptr;
	int write;
	uint32_t tmp;

	ptr = mem_decode_addr(address, &write);
	if (!write && (address & 0xF0000000) == 0x20000000)
		tmp = chipset_read_io(address), ptr = (void *) &tmp;
	return *ptr;
}

unsigned int m68k_read_memory_16(unsigned int address) {
	uint8_t *ptr;
	int write;
	uint16_t data, tmp;
	
	ptr = mem_decode_addr(address, &write);
	if (!write && (address & 0xF0000000) == 0x20000000)
		tmp = chipset_read_io(address), ptr = (void *) &tmp;
	data = ((*ptr) << 8) | ((*(ptr + 1)) << 0);
	return data;
}

unsigned int m68k_read_memory_32(unsigned int address) {
	uint8_t *ptr;
	int write;
	uint32_t data, tmp;
	ptr = mem_decode_addr(address, &write);
	if (!write && (address & 0xF0000000) == 0x20000000)
		tmp = chipset_read_io(address), ptr = (void *) &tmp;
	data = ((*ptr) << 24) | ((*(ptr + 1)) << 16) | ((*(ptr + 2)) << 8) | ((*(ptr + 3)) << 0);
	return data;
}

void m68k_write_memory_8(unsigned int address, unsigned int value) {
	uint8_t *ptr;
	int write;
	ptr = mem_decode_addr(address, &write);
	
	if (!write) {
		if ((address & 0xF0000000) == 0x20000000)
			return chipset_write_io(address, value);
		if (address == 0xFFFFFFFF)
			fprintf(stdout, "%c", (char) value);
		else
			fprintf(stderr, "Invalid write to %X\n", address);
		return;
	}

	*ptr++ = value;
	return;
}

void m68k_write_memory_16(unsigned int address, unsigned int value) {
	uint8_t *ptr;
	int write;

	ptr = mem_decode_addr(address, &write);
	if (!write) {
		if ((address & 0xF0000000) == 0x20000000)
			return chipset_write_io(address, value);
		fprintf(stderr, "Invalid write to %X\n", address);
		return;
	}
	
	*ptr++ = value >> 8;
	*ptr++ = value >> 0;
	return;
}

void m68k_write_memory_32(unsigned int address, unsigned int value) {
	uint8_t *ptr;
	int write;

	ptr = mem_decode_addr(address, &write);
	if (!write) {
		if ((address & 0xF0000000) == 0x20000000)
			return chipset_write_io(address, value);
		fprintf(stderr, "Invalid write to %X\n", address);
		return;
	}

	*ptr++ = value >> 24;
	*ptr++ = value >> 16;
	*ptr++ = value >> 8;
	*ptr++ = value >> 0;
}


void mem_init() {
	FILE *fp;
	size_t sz;

	mem = calloc(sizeof(*mem), 1);

	mem->llram = malloc(1024*512);
	mem->rom = malloc(1024*512);
	mem->mram = malloc(1024*1024*64);
	mem->rom_active = 1;

	fp = fopen("bin/boot.rom", "r");
	fseek(fp, 0, SEEK_END);
	sz = ftell(fp);
	rewind(fp);
	fread(mem->rom, sz, 1, fp);
	fclose(fp);

	return;
}
