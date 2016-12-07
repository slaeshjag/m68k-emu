#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>

static struct {
	int		fd;
} uart_state;


void uart_init() {
	unlink("/tmp/trollbookemu.fifo_in");
	mkfifo("/tmp/trollbookemu.fifo_in", 0700);
	uart_state.fd = open("/tmp/trollbookemu.fifo_in", O_RDWR);
}


void uart_handle_write(unsigned int addr, unsigned int data) {
	if ((addr & 0xFFC) == 0x0) {
		fputc(data & 0xFF, stdout);
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


uint32_t uart_handle_read(unsigned int addr) {
	uint8_t data = 0xFF;
	if ((addr & 0xFC) == 0x0) {
		if (_data_available() >= 0)
			read(uart_state.fd, &data, 1);
		return data;
	} else if ((addr & 0xFC) == 0x4) {
		return (_data_available() >= 0) ? 0x3 : 0x1;
	}

	return data;
}
