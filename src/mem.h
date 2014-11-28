#ifndef __MEM_H__
#define	__MEM_H__

struct Mem {
	void 			*llram;
	void			*mram;
	void			*rom;

	int			rom_active;
};


void mem_init();
void *mem_decode_addr(unsigned int address, int *write);

extern struct Mem *mem;

#endif
