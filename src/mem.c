/* Memory map								*
** 0x00000000 - 0x0007FFFF	ROM (before bank-switch)		*
** 0x00000000 - 0x0007FFFF	LLRAM mirror (after bank-switch)	*
** 0x00080000 - 0x000FFFFF	LLRAM					*
** 0x10000000 - 0x13FFFFFF	Main RAM				*
** 0x20000000 - 0x2FFFFFFF	Chipset control registers		*/

/* LLRAM Map								*
** 0x00000000 - 0x000003FF	Reserved for CPU vectors		*
** 0x00000400 - 0x000007FF	Palette RAM				*
** 0x00000800 - 0x0004B800	VGA framebuffer				*
** 0x0004B800 - 0x0004BFFF	Audio buffer 1 (20 ms)			*
** 0x0004CFFF - 0x0004CFFF	Audio buffer 2 (20 ms)			*
** 0x0004C800 - 0x0004CBFF	SPI 0 send buffer			*
** 0x0004CC00 - 0x0004CFFF	SPI 0 recv buffer			*
** 0x0004D000 - 0x0004D3FF	SPI 1 send buffer			*
** 0x0004D400 - 0x0004D7FF	SPI 1 recv buffer			*
** 0x0004D800 - 0x0004DBFF	SPI 2 send buffer			*
** 0x0004DC00 - 0x0004DFFF	SPI 2 recv buffer			*
** 0x0004E000 - 0x0004E3FF	SPI 3 send buffer			*
** 0x0004E400 - 0x0004E7FF	SPI 3 recv buffer			*
** 0x0004E800 - 0x0007FFFF	Available				*/

#include "mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>

struct Mem *mem;

void *mem_decode_addr(unsigned int address, int *write, int *endian) {
	*endian = 0;
	if (address < 0x80000) {
		if (mem->rom_active) {
			*write = 0;
			*endian = 1;
			return mem->rom + address;
		} else {
			*write = 1;
			return mem->llram + address;
		}
	}

	*write = 1;
	if (address < 0x100000)
		return mem->llram + (address - 0x80000);
	if (address >= 0x10000000 && address <= 0x13FFFFFF)
		return mem->mram + (address - 0x10000000);
	fprintf(stderr, "ERROR: Invalid address %X\n", address);
	exit(-1);
}


unsigned int m68k_read_memory_8(unsigned int address) {
	void *ptr;
	int write, endian;

	ptr = mem_decode_addr(address, &write, &endian);
	return *((uint8_t *) ptr);
}

unsigned int m68k_read_memory_16(unsigned int address) {
	void *ptr;
	int write, endian;
	uint16_t data;
	
	ptr = mem_decode_addr(address, &write, &endian);
	data = *((uint16_t *) ptr);
	//if (endian)
		data = htons(data);
	return data;
}

unsigned int m68k_read_memory_32(unsigned int address) {
	void *ptr;
	int write, endian;
	uint32_t data;
	ptr = mem_decode_addr(address, &write, &endian);
	data = *((uint32_t *) ptr);
	//if (endian)
		data = htonl(data);
	return data;
}

void m68k_write_memory_8(unsigned int address, unsigned int value) {
	void *ptr;
	int write, endian;
	ptr = mem_decode_addr(address, &write, &endian);
	
	if (!write) {
		if (address == 0xFFFFFFFF)
			fprintf(stdout, "%c", (char) value);
		else
			fprintf(stderr, "Invalid write to %X\n", address);
		return;
	}

	*((uint8_t *) ptr) = value;
	return;
}

void m68k_write_memory_16(unsigned int address, unsigned int value) {
	void *ptr;
	int write, endian;

	ptr = mem_decode_addr(address, &write, &endian);
	//if (endian)
		value = ntohs(value);
	if (!write) {
		fprintf(stderr, "Invalid write to %X\n", address);
		return;
	}

	*((uint16_t *) ptr) = value;
	return;
}

void m68k_write_memory_32(unsigned int address, unsigned int value) {
	void *ptr;
	int write, endian;

	ptr = mem_decode_addr(address, &write, &endian);
	//if (endian)
		value = ntohl(value);
	if (!write) {
		fprintf(stderr, "Invalid write to %X\n", address);
		return;
	}

	*((uint32_t *) ptr) = value;
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
