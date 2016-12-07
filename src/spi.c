#include "spi.h"
#include "mem.h"
#include "sd.h"
#include "chipset.h"
#include <stdint.h>
#include <stdio.h>


struct SpiState spi_state;
struct SpiStateNew spi_state_new;

int spi_get_divider() {
	if (spi_state.reg & 020)	/* Slow mode */
		return 512;
	return 8;
}


void spi_new_transfer_one(uint8_t send_byte) {
	if (spi_state_new.slave_select == 0x1) {
		spi_state_new.reg = spirom_send_receive(send_byte);
	} else if (spi_state_new.slave_select == 0x4) {
		fprintf(stderr, "Transferring data %X to SPI\n", send_byte);
		spi_state_new.reg = spi_sd_send_recv(send_byte);
	}
}


uint32_t spi_new_handle_read(unsigned int addr) {
	if ((addr & 0xFC) == 0) {
		return spi_state_new.reg;
	} else if ((addr & 0xFC) == 0x4) {
		return spi_state_new.clockdiv_reg | 0x1;
	} else if ((addr & 0xFC) == 0x8) {
		return spi_state_new.slave_select;
	}
	
	return ~0;
}


void spi_new_handle_write(unsigned int addr, unsigned int data) {
	if ((addr & 0xFC) == 0) {
		spi_new_transfer_one(data);
	} else if ((addr & 0xFC) == 0x8) {
		spi_state_new.slave_select = data;
		spirom_selected_notify((data & 0xF) == 0x1);
	} else if ((addr & 0xFC) == 0x4) {
		spi_state_new.clockdiv_reg = data & 0xFFFF0000;
	} else
		fprintf(stderr, "Unimplemented write to SPI register 0x%X\n", addr);
	return;
}


void spi_loop_one() {
	uint8_t *send = mem->llram + 0x78000;
	uint8_t *recv = mem->llram + 0x7C000;
	uint8_t send_byte, recv_byte, line;

	line = spi_state.line_select & 03;
	if (spi_state.line_select & 040) {
		send += 0x2000;
		recv += 0x2000;
		fprintf(stderr, "Bank 2\n");
	}
	
	if (!(spi_state.reg & 06)) {
		/* Neither send nor revieve.. */
		return;
	}

	if (spi_state.reg & 02)
		send += spi_state.mem_counter;
	if (spi_state.reg & 04)
		recv += spi_state.mem_counter;

	send_byte = *send;
	recv_byte = 0xFF;
	if (!line) {
		recv_byte = spi_sd_send_recv(send_byte);
	} else if (line == 1) {
	} else if (line == 2) {
	} else if (line == 3) {
		recv_byte = spi_sd_send_recv(send_byte);
	}

	if ((spi_state.reg & 01) && recv_byte == 0xFF)
		return;
	if ((spi_state.reg & 010) && recv_byte == 0x0)
		return;
	spi_state.reg &= (~011);
	
	if (spi_state.reg & 04)
		*recv = recv_byte;
	if (!spi_state.mem_counter) {
		spi_state.reg = 0;
		if (spi_state.line_select & 020)
			chipset_int_set(CHIPSET_INT_NUM_SPI_DONE, 1);
	} else
		spi_state.mem_counter--;
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
