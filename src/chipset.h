#ifndef __CHIPSET_H__
#define	__CHIPSET_H__

#include <stdint.h>
#include <stdbool.h>

void chipset_int_set(int int_no, int set_unset);
void chipset_write_io(unsigned int addr, unsigned int data, bool new_map);
uint32_t chipset_read_io(unsigned int addr, bool new_map);
void chipset_set_boot_switch(int sw);

enum ChipsetIntNum {
	CHIPSET_INT_NUM_SPI_DONE = 3,
	CHIPSET_INT_NUM_EXTERNAL,
	CHIPSET_INT_NUM_VGA_VSYNC,
	CHIPSET_INT_NUM_AUDIO,
};


enum ChipsetIoPortNew {
	CHIPSET_IO_PORT_NEW_INTERRUPT = 0x0,
	CHIPSET_IO_PORT_NEW_SPI_BASE = 0x800,
	CHIPSET_IO_PORT_NEW_UART_BASE = 0x900,
	CHIPSET_IO_PORT_NEW_VGA_BASE = 0xA00,
	CHIPSET_IO_PORT_NEW_SOUND_BASE = 0xB00,
	CHIPSET_IO_PORT_NEW_EXTINT = 0xC00,
};


typedef enum ChipsetIoPort ChipsetIoPort;
enum ChipsetIoPort {
	CHIPSET_IO_PORT_INTERRUPT_ENABLE = 0x0,
	
	CHIPSET_IO_PORT_IRQ_ACK_SPI = 0x3,
	CHIPSET_IO_PORT_IRQ_ACK_EXTERNAL,
	CHIPSET_IO_PORT_IRQ_ACK_VGA,
	CHIPSET_IO_PORT_IRQ_ACK_AUDIO,
	
	CHIPSET_IO_PORT_GET_RAM_SIZE = 0x10,
	
	CHIPSET_IO_PORT_SPI_MEM_COUNTER = 0x30,
	CHIPSET_IO_PORT_SPI_LINE_SELECT,
	CHIPSET_IO_PORT_SPI_REG,
	
	CHIPSET_IO_PORT_VGA_WINDOW_X = 0x50,
	CHIPSET_IO_PORT_VGA_WINDOW_Y,
	CHIPSET_IO_PORT_VGA_SPRITE_X,
	CHIPSET_IO_PORT_VGA_SPRITE_Y,

	CHIPSET_IO_PORT_DEBUG = 0x60,

	CHIPSET_IO_PORT_UART_READ = 0x80,
};

#endif
