/* Memory map								*
** 0x00000000 - 0x0007FFFF	LLRAM					*
** 0x00000000 - 0x0007FFFF	ROM					*
** 0x10000000 - 0x13FFFFFF	Main RAM				*
** 0x20000000 - 0x2FFFFFFF	Chipset control registers		*/

/* LLRAM Map								*
** 0x00000000 - 0x000003FF	Reserved for CPU vectors		*
** 0x00000400 - 0x000007FF	Palette RAM				*
** 0x00000800 - 0x0006709F	VGA framebuffer	(800x525)		*
** 0x00067800 - 0x000678FF	Hardware sprite	(16x16)			*
** 0x00068000 - 0x000687FF	Audio buffer 1 (20 ms)			*
** 0x00068800 - 0x00068FFF	Audio buffer 2 (20 ms)			*
** 0x00069000 - 0x000693FF	SPI 0 send buffer			*
** 0x00069400 - 0x000697FF	SPI 0 recv buffer			*
** 0x00069800 - 0x00069BFF	SPI 1 send buffer			*
** 0x00069C00 - 0x00069FFF	SPI 1 recv buffer			*
** 0x0006A000 - 0x0006A3FF	SPI 2 send buffer			*
** 0x0006A400 - 0x0006A7FF	SPI 2 recv buffer			*
** 0x0006A800 - 0x0006ABFF	SPI 3 send buffer			*
** 0x0006AC00 - 0x0006AFFF	SPI 3 recv buffer			*
** 0x0006B000 - 0x0007FFFF	Available				*/

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

	ptr = mem_decode_addr(address, &write);
	return *ptr;
}

unsigned int m68k_read_memory_16(unsigned int address) {
	uint8_t *ptr;
	int write;
	uint16_t data;
	
	ptr = mem_decode_addr(address, &write);
	data = ((*ptr) << 8) | ((*(ptr + 1)) << 0);
	return data;
}

unsigned int m68k_read_memory_32(unsigned int address) {
	uint8_t *ptr;
	int write;
	uint32_t data;
	ptr = mem_decode_addr(address, &write);
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
