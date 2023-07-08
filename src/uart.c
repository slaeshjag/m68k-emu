#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>

#include "interrupt.h"

#define	UART_TXE		1
#define	UART_RXF		2
#define	UART_FLAGS		3

static struct {
	int		fd;
	uint32_t	status;
	uint32_t	control;
} uart_state;


void uart_init() {
	unlink("/tmp/trollbookemu.fifo_in");
	mkfifo("/tmp/trollbookemu.fifo_in", 0700);
	uart_state.fd = open("/tmp/trollbookemu.fifo_in", O_RDWR);
}


void uart_handle_write(unsigned int addr, unsigned int data) {
	if ((addr & 0xFFC) == 0x0) {
		fputc(data & 0xFF, stdout);
	} else if ((addr & 0xFFC) == 0x8) {
		uart_state.control = data;
		printf("UART: Control data: 0x%X\n", data);
	}
}


static int _data_available() {
	fd_set set;
	struct timeval timeout;

	FD_ZERO(&set);
	FD_SET(0, &set);
	FD_SET(uart_state.fd, &set);
	timeout.tv_sec = timeout.tv_usec = 0;
	select(uart_state.fd + 1, &set, NULL, NULL, &timeout);
	if (FD_ISSET(uart_state.fd, &set))
		return uart_state.fd;
	if (FD_ISSET(0, &set))
		return 0;
	return -1;
}


void uart_irq_handle() {
	int iflag = 0;


	if ((!(uart_state.status & UART_TXE)) && (uart_state.control & UART_TXE))
		iflag = 1;
	uart_state.status |= UART_TXE;
	if (_data_available() >= 0) {
		uart_state.status |= UART_RXF;
		if (uart_state.control & UART_RXF)
			iflag = 1;
	} else
		uart_state.status = 0x1;

	if (iflag)
		interrupt_trig(9);
}


uint32_t uart_handle_read(unsigned int addr) {
	uint8_t data = 0xFF;
	if ((addr & 0xFC) == 0x0) {
		if (_data_available() >= 0) {
			uart_state.status &= UART_TXE;
			read(uart_state.fd, &data, 1);
		}
		return data;
	} else if ((addr & 0xFC) == 0x4) {
		return uart_state.status;
		//return (_data_available() >= 0) ? 0x3 : 0x1;
	}

	return data;
}
