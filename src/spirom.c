#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

enum SPIRomState {
	SPI_ROM_STATE_IDLE,
	SPI_ROM_STATE_COMMAND,
	SPI_ROM_STATE_COMMAND_READ_ADDR,
	SPI_ROM_STATE_COMMAND_READ_ADDR2,
	SPI_ROM_STATE_COMMAND_READ_ADDR3,
	SPI_ROM_STATE_COMMAND_READ,

	SPI_ROM_STATE_INVALID,
};


static struct {
	bool			selected;
	enum SPIRomState	state;
	FILE			*fp;
} spirom_state;


void spirom_selected_notify(bool selected) {
	spirom_state.selected = selected;
	if (!selected)
		spirom_state.state = SPI_ROM_STATE_IDLE;
}


uint8_t spirom_send_receive(uint8_t data) {
	uint32_t u32;
	
	if (spirom_state.state == SPI_ROM_STATE_IDLE) {
		if (data == 0x3) {
			fprintf(stderr, "SPI ROM: Read command\n");
			spirom_state.state = SPI_ROM_STATE_COMMAND_READ_ADDR;
		} else {
			fprintf(stderr, "SPI ROM: Unhandled command %X\n", data);
			spirom_state.state = SPI_ROM_STATE_INVALID;
			return 0xFF;
		}
	} else if (spirom_state.state == SPI_ROM_STATE_COMMAND_READ_ADDR) {
		u32 = data;
		u32 <<= 16;
		fseek(spirom_state.fp, u32, SEEK_SET);
		spirom_state.state = SPI_ROM_STATE_COMMAND_READ_ADDR2;
	} else if (spirom_state.state == SPI_ROM_STATE_COMMAND_READ_ADDR2) {
		u32 = data;
		u32 <<= 8;
		fseek(spirom_state.fp, u32, SEEK_CUR);
		spirom_state.state = SPI_ROM_STATE_COMMAND_READ_ADDR3;
	} else if (spirom_state.state == SPI_ROM_STATE_COMMAND_READ_ADDR3) {
		u32 = data;
		fseek(spirom_state.fp, u32, SEEK_CUR);
		spirom_state.state = SPI_ROM_STATE_COMMAND_READ;
	} else if (spirom_state.state == SPI_ROM_STATE_COMMAND_READ) {
		fread(&data, 1, 1, spirom_state.fp);
		return data;
	} else
		fprintf(stderr, "SPI ROM: Invalid state %i\n", spirom_state.state);
	return 0xFF;
}


void spi_rom_init(const char *fname) {
	spirom_state.state = SPI_ROM_STATE_IDLE;
	spirom_state.fp = fopen(fname, "r+");
}
