#ifndef SPI_KEYBOARD_H__
#define	SPI_KEYBOARD_H__

#include <stdint.h>
#include <stdbool.h>

uint8_t spi_keyboard_send_receive(uint8_t data);
void spi_keyboard_notify_selected(bool selected);


#endif
