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

#include "cpu/m68000.h"
#include "mem.h"
#include "mdebug.h"
#include "chipset.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

struct Mem *mem;

uint32_t junk;

void *mem_decode_addr(unsigned int address, int *write) {
	*write = 1;

	if (address < 0x80000UL) {
		*write = 0;
		return mem->rom + address;
	} else if (address >= 0x80000UL && address < 0x100000UL) {
		return mem->llram + (address - 0x80000UL);
	}

	if (mem->new_map) {
		if (address >= 0x80000000UL && address < 0x84000000UL)
			return mem->mram + (address - 0x80000000UL);
	} else {
		if (address >= 0x1000000UL && address < 0x5000000UL)
			return mem->mram + (address - 0x1000000UL);
	}

	if (((address & 0xFFF00000UL) == 0x100000UL) && mem->new_map) {
		*write = 0;
		return mem->llram;
	}

	if ((address & 0xFFF00000UL) == 0x200000UL) {
		/* Dummy value */
		*write = 0;
		return mem->llram;
	}
	fprintf(stderr, "Invalid address 0x%X (PC=0x%X)\n", address, m68k_getpc());
	return NULL;
}


unsigned int m68k_read_memory_8(unsigned int address) {
	uint8_t *ptr;
	int write;
	uint32_t tmp;

	if (!(ptr = mem_decode_addr(address, &write))) {
		M68000_BusError(address, BUS_ERROR_READ);
		return 0;
	}

	if (mem->new_map) {
		if (!write && (address & 0xFFF00000UL) == 0x100000UL)
			tmp = chipset_read_io(address, mem->new_map), ptr = (void *) &tmp;
	} else {
		if (!write && (address & 0xFFF00000UL) == 0x200000UL)
			tmp = chipset_read_io(address, mem->new_map), ptr = (void *) &tmp;
	}

	return *ptr;
}

unsigned int m68k_read_memory_16(unsigned int address) {
	uint8_t *ptr;
	int write;
	uint16_t data, tmp;
	
	if (!(ptr = mem_decode_addr(address, &write))) {
		M68000_BusError(address, BUS_ERROR_READ);
		return 0;
	}

	if (mem->new_map) {
		if (!write && (address & 0xFFF00000UL) == 0x200000UL)
			data = chipset_read_io(address, mem->new_map), ptr = (void *) &tmp;
		else
			data = ((*ptr) << 8) | ((*(ptr + 1)) << 0);
	} else {
		if (!write && (address & 0xFFF00000UL) == 0x100000UL)
			data = chipset_read_io(address, mem->new_map), ptr = (void *) &tmp;
		else
			data = ((*ptr) << 8) | ((*(ptr + 1)) << 0);
	}
	return data;
}

unsigned int m68k_read_memory_32(unsigned int address) {
	uint8_t *ptr;
	int write;
	uint32_t data, tmp;
	if (!(ptr = mem_decode_addr(address, &write))) {
		M68000_BusError(address, BUS_ERROR_READ);
		return 0;
	}
	if (mem->new_map) {
		if (!write && (address & 0xFFF00000UL) == 0x100000UL)
			data = chipset_read_io(address, mem->new_map), ptr = (void *) &tmp;
		else
			data = ((*ptr) << 24) | ((*(ptr + 1)) << 16) | ((*(ptr + 2)) << 8) | ((*(ptr + 3)) << 0);
	} else {
		if (!write && (address & 0xFFF00000UL) == 0x200000UL)
			data = chipset_read_io(address, mem->new_map), ptr = (void *) &tmp;
		else
			data = ((*ptr) << 24) | ((*(ptr + 1)) << 16) | ((*(ptr + 2)) << 8) | ((*(ptr + 3)) << 0);
	}
	return data;
}

void m68k_write_memory_8(unsigned int address, unsigned int value) {
	uint8_t *ptr;
	int write;

	if (address == 0xFFFFFFFE) {
		mdebug_write(value);
		return;
	}

	if (address == 0xFFFFFFFF)
		return (void) fwrite(&value, 1, 1, stdout);

	if (!(ptr = mem_decode_addr(address, &write))) {
		M68000_BusError(address, BUS_ERROR_WRITE);
		return;
	}
	
	if (!write) {
		if (mem->new_map) {
			if ((address & 0xFFF00000UL) == 0x100000UL)
				return chipset_write_io(address, value, mem->new_map);
		} else {
			if ((address & 0xFFF00000UL) == 0x200000UL)
				return chipset_write_io(address, value, mem->new_map);
		}

		if (address == 0xFFFFFFFF)
			fprintf(stdout, "%c", (char) value);
		else
			fprintf(stderr, "Invalid write to 0x%X (PC=0x%X\n", address, m68k_getpc());
		return;
	}

	*ptr++ = value;
	return;
}

void m68k_write_memory_16(unsigned int address, unsigned int value) {
	uint8_t *ptr;
	int write;

	if (!(ptr = mem_decode_addr(address, &write))) {
		M68000_BusError(address, BUS_ERROR_WRITE);
		return;
	}
	if (!write) {
		if (mem->new_map) {
			if ((address & 0xFFF00000UL) == 0x100000UL)
				return chipset_write_io(address, value, mem->new_map);
		} else {
			if ((address & 0xFFF00000UL) == 0x200000UL)
				return chipset_write_io(address, value, mem->new_map);
		}

		fprintf(stderr, "Invalid write to 0x%X (PC=0x%X\n", address, m68k_getpc());
		return;
	}
	
	*ptr++ = value >> 8;
	*ptr++ = value >> 0;
	return;
}

void m68k_write_memory_32(unsigned int address, unsigned int value) {
	uint8_t *ptr;
	int write;

	if (!(ptr = mem_decode_addr(address, &write))) {
		M68000_BusError(address, BUS_ERROR_WRITE);
		return;
	}
	if (!write) {
		if (mem->new_map) {
			if ((address & 0xFFF00000UL) == 0x100000UL)
				return chipset_write_io(address, value, mem->new_map);
		} else {
			if ((address & 0xFFF00000UL) == 0x200000UL)
				return chipset_write_io(address, value, mem->new_map);
		}
		fprintf(stderr, "Invalid write to 0x%X (PC=0x%X\n", address, m68k_getpc());
		return;
	}

	*ptr++ = value >> 24;
	*ptr++ = value >> 16;
	*ptr++ = value >> 8;
	*ptr++ = value >> 0;
}


void mem_init(const char *filename, bool new_map) {
	FILE *fp;
	size_t sz;
	int i;

	fprintf(stderr, "using %s memory layout\n", new_map?"new":"old");

	mem = calloc(sizeof(*mem), 1);

	mem->llram = malloc(1024*512);
	mem->rom = malloc(1024*512);
	mem->mram = malloc(MEM_SIZE);
	mem->rom_active = 1;
	mem->new_map = new_map;

	fp = fopen(filename, "r");
	if(!fp) {
		fprintf(stderr, "Error: could not open rom image\n");
		exit(1);
	}
	fseek(fp, 0, SEEK_END);
	sz = ftell(fp);
	rewind(fp);
	fread(mem->rom, sz, 1, fp);
	fclose(fp);

	srand(time(NULL));
	for (i = 0; i < 512*512; i++)
		((uint16_t *) mem->llram)[i] = rand();
	for (i = 0; i < MEM_SIZE/2; i++)
		((uint16_t *) mem->mram)[i] = rand();

	return;
}
