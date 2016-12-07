#include <stdint.h>
#include <stdbool.h>

#ifndef SPIROM_H__
#define	SPIROM_H__

void spirom_selected_notify(bool selected);
uint8_t spirom_send_receive(uint8_t data);
void spi_rom_init(const char *fname);


#endif
