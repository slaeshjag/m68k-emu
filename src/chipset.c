#include "chipset.h"
#include "mem.h"
#include "vga.h"
#include "spi.h"
#include "uart.h"
#include "debug.h"


static int interrupt[8];
static int bootswitch = 0;


void chipset_set_boot_switch(int sw) {
	bootswitch = sw;
}


int chipset_get_interrupt_level() {
	int i;

	if (!interrupt[0])
		return 0;

	for (i = 0; i < 7; i++)
		if (interrupt[7-i]) {
			return 7 - i;
		}
	return 0;
}


void chipset_int_set(int int_no, int set_unset) {
	int i;

	interrupt[int_no] = set_unset;

	if (!interrupt[0]) {
		//fprintf(stderr, "interrupts not enabled\n");
		//m68k_set_irq(0);
		return;
	} 

	for (i = 0; i < 7; i++) {
		if (interrupt[7-i]) {
			//m68k_set_irq(7 - i);
			return;
		}
	}

	//m68k_set_irq(0);
}


void chipset_new_write_io(unsigned int addr, unsigned int data) {
	if ((addr & 0xF00) == CHIPSET_IO_PORT_NEW_SPI_BASE) {
		//fprintf(stderr, "SPI write to addr 0x%X: 0x%X\n", addr, data);
		spi_new_handle_write(addr, data);
	} else if ((addr & 0xF00) == CHIPSET_IO_PORT_NEW_UART_BASE) {
		spi_new_handle_write(addr, data);
	} else if ((addr & 0xF00) == CHIPSET_IO_PORT_NEW_EXTINT) {
		if ((addr & 0xFC) == 0x4)
			chipset_int_set(0, !!(data & 1));
		else if ((addr & 0xFC) == 0x8) {
			int i;

			for (i = 1; i < 8; i++) {
				if (data & (1 << i))
					chipset_int_set(i, 0);
			}
		}
	}
}


unsigned int chipset_new_read_io(unsigned int addr) {
	unsigned int data;
	if ((addr & 0xF00) == CHIPSET_IO_PORT_NEW_SPI_BASE) {
		data = spi_new_handle_read(addr);
		//fprintf(stderr, "SPI read from addr 0x%X: 0x%X\n", addr, data);
		return data;
	} else if ((addr & 0xF00) == CHIPSET_IO_PORT_NEW_UART_BASE) {
		return uart_handle_read(addr);
	} else if ((addr & 0xF00) == CHIPSET_IO_PORT_NEW_EXTINT) {
		return bootswitch;
	}

	return ~0;
}


void chipset_write_io(unsigned int addr, unsigned int data, bool new_map) {
	unsigned int port = (addr & 0xFFFC)/4;
	//fprintf(stderr, "write_io %u 0x%X %u\n",port, addr, data);
	if (new_map)
		return chipset_new_write_io(addr, data);
	
	switch(port) {
		case CHIPSET_IO_PORT_INTERRUPT_ENABLE:
			chipset_int_set(0, data & 1);
			return;
		case CHIPSET_IO_PORT_IRQ_ACK_SPI:
			chipset_int_set(CHIPSET_INT_NUM_SPI_DONE, 0);
			return;
		case CHIPSET_IO_PORT_IRQ_ACK_VGA:
			chipset_int_set(CHIPSET_INT_NUM_VGA_VSYNC, 0);
			return;
			
		case CHIPSET_IO_PORT_VGA_WINDOW_X:
			vga_state.reg.window_x = data & 0x3FF;
			return;
		case CHIPSET_IO_PORT_VGA_WINDOW_Y:
			vga_state.reg.window_y = data & 0x3FF;
			return;
		case CHIPSET_IO_PORT_VGA_SPRITE_X:
			vga_state.reg.sprite_x = data & 0x3FF;
			return;
		case CHIPSET_IO_PORT_VGA_SPRITE_Y:
			vga_state.reg.sprite_y = data & 0x3FF;
			return;
		
		case CHIPSET_IO_PORT_SPI_MEM_COUNTER:
			spi_state.mem_counter = data & 0x3FF;
			return;
		case CHIPSET_IO_PORT_SPI_LINE_SELECT:
			spi_state.line_select = data & 0x1F;
			return;
		case CHIPSET_IO_PORT_SPI_REG:
			spi_state.reg = data & 0xF;
			return;
		
		case CHIPSET_IO_PORT_DEBUG:
			debug_send(data);
			return;
		
		default:
			fprintf(stderr, "Wrote invalid IO-port 0x%X\n", addr);
	}

	return;
}


uint32_t chipset_read_io(unsigned int addr, bool new_map) {
	unsigned int port = (addr & 0xFFFC)/4;
	
	if (new_map)
		return chipset_new_read_io(addr);
	
	switch(port) {
		/*
		case CHIPSET_IO_PORT_NUMBER_INTERRUPT_ENABLE:
			return 0x0;
		case CHIPSET_IO_PORT_NUMBER_IRQ_ACK_SPI:
			return 0x0;
		case CHIPSET_IO_PORT_NUMBER_IRQ_ACK_VGA:
			return 0x0;
		*/
		
		case CHIPSET_IO_PORT_GET_RAM_SIZE:
			return MEM_SIZE;
			
		case CHIPSET_IO_PORT_VGA_WINDOW_X:
			return vga_state.reg.window_x  & 0x3FF;
		case CHIPSET_IO_PORT_VGA_WINDOW_Y:
			return vga_state.reg.window_y & 0x3FF;
		case CHIPSET_IO_PORT_VGA_SPRITE_X:
			return vga_state.reg.sprite_x & 0x3FF;
		case CHIPSET_IO_PORT_VGA_SPRITE_Y:
			return vga_state.reg.sprite_y & 0x3FF;
		
		case CHIPSET_IO_PORT_SPI_MEM_COUNTER:
			return spi_state.mem_counter & 0x3FF;
		case CHIPSET_IO_PORT_SPI_LINE_SELECT:
			return spi_state.line_select & 0x1F;
		case CHIPSET_IO_PORT_SPI_REG:
			return spi_state.reg & 0xF;
		case CHIPSET_IO_PORT_UART_READ:
			return getchar();
		
		case CHIPSET_IO_PORT_DEBUG:
			return debug_recv();
		
		default:
			fprintf(stderr, "Read invalid IO-port 0x%X\n", addr);	
	}
	return 0xFFFFFFFF;
}
