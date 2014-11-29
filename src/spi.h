#ifndef __SPI_H__
#define	__SPI_H__

#include <stdint.h>

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
	** bit 3	- Slow mode (SPI-clock = 11.2896/8 MHz = 352.8 kHz)		*/
	uint8_t			line_select;

	/* bit 0	- Wait for non-0xFF recieved before decreasing mem_counter	*
	** bit 1	- Send bytes (if zero, send from 0x0)				*
	** bit 2	- Recieve bytes (if zero, discard all incoming bytes)		*
	** bit 3	- Wait for non-0xFF recieved before decreasing mem_counter	*/
	uint8_t			reg;
};

extern struct SpiState spi_state;

#endif
