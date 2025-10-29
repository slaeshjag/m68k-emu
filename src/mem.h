#ifndef __MEM_H__
#define	__MEM_H__

#include <stdbool.h>

#define MEM_SIZE (1024*1024*64)

struct Mem {
	bool			tb1;		// If true, use trollbook 1 map
	void 			*llram;
	void			*mram;
	void			*rom;

	int			rom_active;
};


void mem_init(const char *filename, bool tb1);
void *mem_decode_addr(unsigned int address, int *write);

extern struct Mem *mem;

unsigned int m68k_read_memory_8(unsigned int address);
unsigned int m68k_read_memory_16(unsigned int address);
unsigned int m68k_read_memory_32(unsigned int address);
void m68k_write_memory_8(unsigned int address, unsigned int value);
void m68k_write_memory_16(unsigned int address, unsigned int value);
void m68k_write_memory_32(unsigned int address, unsigned int value);

#endif
