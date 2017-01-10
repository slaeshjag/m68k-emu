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
	SPI_ROM_STATE_COMMAND_WRITE_ADDR,
	SPI_ROM_STATE_COMMAND_WRITE_ADDR2,
	SPI_ROM_STATE_COMMAND_WRITE_ADDR3,
	SPI_ROM_STATE_COMMAND_WRITE,

	SPI_ROM_STATE_LIMBO,
	SPI_ROM_STATE_INVALID,
};


static struct {
	bool			selected;
	enum SPIRomState	state;
	FILE			*fp;
	bool			write_enable;
} spirom_state;


void spirom_selected_notify(bool selected) {
	spirom_state.selected = selected;
	if (!selected)
		spirom_state.state = SPI_ROM_STATE_IDLE;
}


uint8_t spirom_send_receive(uint8_t data) {
	static uint32_t u32;
	int i;
	
	if (spirom_state.state == SPI_ROM_STATE_IDLE) {
		if (data == 0x2) {
			if (spirom_state.write_enable) {
				fprintf(stderr, "SPI ROM: Write command\n");
				spirom_state.state = SPI_ROM_STATE_COMMAND_WRITE_ADDR;
			} else {
				fprintf(stderr, "SPI ROM: Write command: Write latch is off\n");
				spirom_state.state = SPI_ROM_STATE_LIMBO;
			}
		} else if (data == 0x3) {
			fprintf(stderr, "SPI ROM: Read command\n");
			spirom_state.state = SPI_ROM_STATE_COMMAND_READ_ADDR;
		} else if (data == 0x5) {
			spirom_state.state = SPI_ROM_STATE_LIMBO;
			return (spirom_state.write_enable?0x2:0);
		} else if (data == 0x6) {
			fprintf(stderr, "SPI ROM: Write Enable\n");
			spirom_state.write_enable = true;
		} else if (data == 0x4) {
			fprintf(stderr, "SPI ROM; Write Disable\n");
			spirom_state.write_enable = false;
		} else if (data == 0xC7) {
			spirom_state.write_enable = false;
			fseek(spirom_state.fp, 0, SEEK_SET);
			u32 = 0xFFFFFFFF;
			for (i = 0; i < 512*1024; i++) {
				fwrite(&u32, 1, 1, spirom_state.fp);
			}
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
	} else if (spirom_state.state == SPI_ROM_STATE_COMMAND_WRITE_ADDR) {
		u32 = data;
		u32 <<= 16;
		spirom_state.state = SPI_ROM_STATE_COMMAND_WRITE_ADDR2;
	} else if (spirom_state.state == SPI_ROM_STATE_COMMAND_WRITE_ADDR2) {
		u32 |= (data << 8);
		spirom_state.state = SPI_ROM_STATE_COMMAND_WRITE_ADDR3;
	} else if (spirom_state.state == SPI_ROM_STATE_COMMAND_WRITE_ADDR3) {
		u32 |= data;
		spirom_state.state = SPI_ROM_STATE_COMMAND_WRITE;
	} else if (spirom_state.state == SPI_ROM_STATE_COMMAND_WRITE) {
		spirom_state.write_enable = false;
		fseek(spirom_state.fp, u32, SEEK_SET);
		fwrite(&data, 1, 1, spirom_state.fp);
		if ((u32 & 0xFF) == 0xFF)
			u32 &= 0xFFFF00;
		else
			u32++;
	} else if (spirom_state.state == SPI_ROM_STATE_LIMBO) {
		return 0;
	} else
		fprintf(stderr, "SPI ROM: Invalid state %i\n", spirom_state.state);
	return 0xFF;
}


void spi_rom_init(const char *fname) {
	spirom_state.state = SPI_ROM_STATE_IDLE;
	if (!(spirom_state.fp = fopen(fname, "r+")))
		spirom_state.fp = fopen(fname, "w+");
}
