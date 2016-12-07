#ifndef __SPI_H__
#define	__SPI_H__

#include <stdint.h>

/* Den lökigaste SPI-hårdvaran i landet! */
struct SpiStateNew {
	uint8_t			reg;
	unsigned int		slave_select;
	unsigned int		clockdiv_reg;
};


/* Den blingigaste SPI-hårdvaran i stan! */
struct SpiState {
	/* Internal state only, not needed in hardware */
	unsigned int		spare_cycles;
	
	/* Next byte offset. Decremented for every transaction. */
	unsigned int		mem_counter;
	/* Select active SPI slave (0..3,) bit 0-1					*
	** Slave 0	- Internal SD card						*
	** Slave 1	- HID/RTC/LED controller (requires slow mode)			*
	** Slave 2	- Not assigned							*
	** Slave 3	- External SPI port						*
	** bit 2	- Assert ~CS (if zero, all CS lines remain high)		*
	** bit 3	- Slow mode (SPI-clock = 11.2896/8 MHz = 352.8 kHz)		*
	** bit 4	- Interrupt when done						*
	** bit 5	- Memory bank select (bank 0/1)					*/
	uint8_t			line_select;

	/* bit 0	- Wait for non-0xFF recieved before decreasing mem_counter	*
	** bit 1	- Send bytes (if zero, send from 0x0)				*
	** bit 2	- Recieve bytes (if zero, discard all incoming bytes)		*
	** bit 3	- Wait for non-0x00 recieved before decreasing mem_counter	*/
	uint8_t			reg;
};

void spi_loop(int cycles);
void spi_new_handle_write(unsigned int addr, unsigned int data);
unsigned int spi_new_handle_read(unsigned int addr);

extern struct SpiState spi_state;

#endif
