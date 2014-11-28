#include "spi.h"
#include "mem.h"
#include "sd.h"
#include "chipset.h"
#include <stdint.h>


struct SpiState spi_state;

int spi_get_divider() {
	if (spi_state.reg & 020)	/* Slow mode */
		return 512;
	return 8;
}

void spi_loop_one() {
	uint8_t *send = mem->llram + 0x6A000;
	uint8_t *recv = mem->llram + 0x6A400;
	uint8_t send_byte, recv_byte, line;

	line = spi_state.line_select & 03,
	send += line * 0x800;
	recv += line * 0x800;
	
	if (!(spi_state.reg & 06)) {
		/* Neither send nor revieve.. */
		return;
	}

	if (spi_state.reg & 02)
		send += spi_state.mem_counter;
	if (spi_state.reg & 04)
		recv += spi_state.mem_counter;

	send_byte = *send;
	if (!line) {

	} else if (line == 1) {
	} else if (line == 2) {
	} else if (line == 3) {
		//recv_byte = spi_sd_send_recv(send_byte);
	}

	if ((spi_state.reg & 01) && recv_byte == 0xFF)
		return;
	if ((spi_state.reg & 010) && recv_byte == 0x0)
		return;
	spi_state.reg &= (~011);
	if (!spi_state.mem_counter)
		spi_state.reg = 0, chipset_int_set(ChipsetIntNumSPIDone, 1);
	else
		spi_state.mem_counter--;
	if (spi_state.reg & 04)
		*recv = recv_byte;
	return;
}


void spi_loop(int cycles) {
	unsigned int bytes;
	int i;

	bytes = (cycles + spi_state.spare_cycles) / spi_get_divider();
	spi_state.spare_cycles = (cycles + spi_state.spare_cycles) / spi_get_divider();
	
	for (i = 0; i < bytes; i++)
		spi_loop_one();
	return;
}
