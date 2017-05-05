#ifndef SPI_KEYBOARD_H__
#define	SPI_KEYBOARD_H__

#include <stdint.h>
#include <stdbool.h>

uint8_t spi_keyboard_send_receive(uint8_t data);
void spi_keyboard_notify_selected(bool selected);
void spi_keyboard_push_event(bool press, int scancode);
void spi_keyboard_digitizer_set(int x, int y);


enum SPIKeyboardBits {
	SPI_KEYBOARD_KB_BIT,
	SPI_KEYBOARD_MOUSE_BIT,
	SPI_KEYBOARD_POWERBUTTON_BIT,
	SPI_KEYBOARD_SD_BIT,
	SPI_KEYBOARD_HEADPHONE_BIT,
	SPI_KEYBOARD_I2C_BIT,
	SPI_KEYBOARD_DIGITIZER_BIT,
};


enum SPIKeyboardCommand {
	SPI_KEYBOARD_CMD_NONE = -1,
	SPI_KEYBOARD_CMD_STATUS,
	SPI_KEYBOARD_CMD_CONTROL,
	SPI_KEYBOARD_CMD_KEYBOARD_EVENT,
	SPI_KEYBOARD_CMD_MOUSE_EVENT,
	SPI_KEYBOARD_CMD_POWER_OFF,
	SPI_KEYBOARD_CMD_POWER_REBOOT,
	SPI_KEYBOARD_CMD_VOLUME,
	SPI_KEYBOARD_CMD_ADC,
	SPI_KEYBOARD_CMD_I2C,
	SPI_KEYBOARD_CMD_DIGITIZER,
};


#endif
