#ifndef MDEBUG_H_
#define	MDEBUG_H_


#include <stdint.h>

void mdebug_write(uint8_t value);
int mdebug_check_pointer(uint32_t pointer, int size);


#endif
