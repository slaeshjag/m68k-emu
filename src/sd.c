#include "spi.h"
#include "sd.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mem.h"

enum SdState {
	SD_STATE_NOCARD,
	SD_STATE_UNINIT,
	SD_STATE_IDLE,
	SD_STATE_INIT,
};

enum SdSubState {
	SD_SUBSTATE_IDLE,
	SD_SUBSTATE_CMD,
	SD_SUBSTATE_WAIT_ACK,
	SD_SUBSTATE_INVALID,
	SD_SUBSTATE_PROCESS,
	SD_SUBSTATE_SEND_REG,
	SD_SUBSTATE_SEND_BLOCK,
};

struct {
	enum SdState	state;
	enum SdSubState	substate;
	uint8_t		arg[1024];
	int		args;
	int		command;
	int		acmd_start;

	int		init_wait;

	uint32_t	sdcard_size;
	uint32_t	sdcard_multipl;
	uint32_t	size_blocks;

	FILE		*fp;
} sd_state;


void spi_sd_init(const char *sd_image) {
	int i;

	if (!(sd_state.fp = fopen(sd_image, "r+"))) {
		fprintf(stderr, "No SD card installed\n");
		sd_state.state = SD_STATE_NOCARD;
		return;
	}

	sd_state.state = SD_STATE_UNINIT;
	fseek(sd_state.fp, 0, SEEK_END);
	sd_state.size_blocks = ftell(sd_state.fp);
	rewind(sd_state.fp);
	sd_state.size_blocks /= 512;
	for (i = 0; i < 8; i++)
		if (sd_state.size_blocks / (1 << (i + 2)) < 0x1000)
			break;
	if (i == 8) {
		fprintf(stderr, "SD Card is too big\n");
		fclose(sd_state.fp);
		sd_state.state = SD_STATE_NOCARD;
		return;
	}

	sd_state.sdcard_multipl = i;
	sd_state.sdcard_size = sd_state.size_blocks / (1 << (i + 2)) - 1;

	if ((sd_state.sdcard_size + 1) * (1 << (i + 2)) != sd_state.size_blocks) {
		fprintf(stderr, "Warning: SD-card is of impossible size, using %i kB instead of %i\n", (sd_state.sdcard_size + 1) * (1 << (i + 2)) / 2, sd_state.size_blocks / 2);
	}

	fprintf(stderr, "SD card of size %i kB installed (%i %i)\n", (sd_state.sdcard_size + 1) * (1 << (i + 2)) / 2, sd_state.sdcard_size, sd_state.sdcard_multipl);
}


int spi_sd_check_enabled() {
	if (!mem->new_map) {
		if (!(spi_state.line_select & 0x4))
			return 0;
		if (sd_state.state == SD_STATE_NOCARD)
			return 0;
	}
	return 1;
}


uint8_t sd_idle_handler(uint8_t byte) {
	if ((byte & 0xC0) != 0x40)
		return 0xFF;
	sd_state.command = byte & 0x3F;
	if (sd_state.acmd_start)
		sd_state.command |= 0x40, sd_state.acmd_start = 0;
	sd_state.args = 0;
	sd_state.substate = SD_SUBSTATE_CMD;
	return 0xFF;
}


uint8_t sd_read_command(uint8_t byte) {
	sd_state.arg[sd_state.args++] = byte;
	if (sd_state.args == 5) {
		//fprintf(stderr, "Command %i\n", sd_state.command);
		sd_state.substate = SD_SUBSTATE_PROCESS;
	}
	
	return 0xFF;
}


uint8_t spi_sd_send_recv(uint8_t byte) {
	int n, i;

	if (!spi_sd_check_enabled())
		return 0xFF;
	if (sd_state.substate == SD_SUBSTATE_IDLE) {
		return sd_idle_handler(byte);
	} else if (sd_state.substate == SD_SUBSTATE_CMD) {
		return sd_read_command(byte);
	} else if (sd_state.substate == SD_SUBSTATE_PROCESS) {
		if (sd_state.command == 55) {
			sd_state.acmd_start = 1;
			sd_state.substate = SD_SUBSTATE_IDLE;
			return 0x0;
		}

		/* Process command */
		/* Favorit i repris: //Achtung fulkod */
		if (sd_state.state == SD_STATE_UNINIT) {
			if (sd_state.command == 0) {
				sd_state.state = SD_STATE_IDLE;
				sd_state.substate = SD_SUBSTATE_IDLE;
				sd_state.init_wait = (rand() & 0x1F) + 8;
				return 0x1;
			} else {
				sd_state.substate = SD_SUBSTATE_IDLE;
				return 0x4;
			}
		} else if (sd_state.state == SD_STATE_IDLE) {
			if (sd_state.command == 0) {
				sd_state.substate = SD_SUBSTATE_IDLE;
				return 0x1;
			} else if (sd_state.command == 105) {	/* ACMD41 (init) */
			//	if (--sd_state.init_wait == 0) {
					sd_state.state = SD_STATE_INIT;
					sd_state.substate = SD_SUBSTATE_IDLE;
					return 0x0;
			//	} else {
			//		sd_state.substate = SD_SUBSTATE_IDLE;
			//		return 0x1;
			//	}
			} else {
				sd_state.substate = SD_SUBSTATE_IDLE;
				return 0x5;
			}
		} else if (sd_state.state == SD_STATE_INIT) {
			if (sd_state.command == 0) {
				sd_state.state = SD_STATE_IDLE;
				sd_state.substate = SD_SUBSTATE_IDLE;
				sd_state.init_wait = (rand() & 0x1F) + 8;
				return 0x1;
			} else if (sd_state.command == 58) {
				sd_state.substate = SD_SUBSTATE_SEND_REG;
				sd_state.args = 4;
				sd_state.arg[3] = 0x80;
				sd_state.arg[2] = 0xFF;
				sd_state.arg[1] = sd_state.arg[0] = 0x0;
				return 0x0;
			} else if (sd_state.command == 105) {	/* ACMD41 (init) */
			//	if (--sd_state.init_wait == 0) {
				sd_state.state = SD_STATE_INIT;
				sd_state.substate = SD_SUBSTATE_IDLE;
				return 0x0;
			} else if (sd_state.command == 9) {	/* Read CSD */
				//fprintf(stderr, "READ CSD\n");
				sd_state.substate = SD_SUBSTATE_SEND_REG;
				sd_state.args = 20;
				sd_state.arg[19] = 0xFF, sd_state.arg[18] = 0xFE;
				sd_state.arg[17] = 0x0, sd_state.arg[16] = 0x26;
				sd_state.arg[15] = 0x0, sd_state.arg[14] = 0x5A;
				sd_state.arg[13] = 0x0, sd_state.arg[12] = 0x19;
				sd_state.arg[11] = 0x0 | ((sd_state.sdcard_size >> 10) & 0x3);
				sd_state.arg[10] = (sd_state.sdcard_size >> 2);
				sd_state.arg[9] = ((sd_state.sdcard_size & 0x3) << 6) | 0x3F;
				sd_state.arg[8] = 0xFC | ((sd_state.sdcard_multipl >> 1) & 0x3);
				sd_state.arg[7] = 0x40 | ((sd_state.sdcard_multipl & 0x1) << 7);
				sd_state.arg[6] = 0xFF, sd_state.arg[5] = 0x0A;
				sd_state.arg[4] = 0x40, sd_state.arg[3] = 0x40;
				/* Don't bother with CRC */
				sd_state.arg[2] = 0x1, sd_state.arg[1] = sd_state.arg[0] = 0x0;
				return 0x0;
			} else if (sd_state.command == 16) {	/* Set block size */
				sd_state.substate = SD_SUBSTATE_IDLE;
				if (sd_state.arg[0] == 0x0 && sd_state.arg[1] == 0x0 && sd_state.arg[2] == 0x2 && sd_state.arg[3] == 0x0)
					return 0x0;
				return 0x40;
			} else if (sd_state.command == 17) {
				n = sd_state.arg[0] << 24, n |= sd_state.arg[1] << 16;
				n |= sd_state.arg[2] << 8, n |= sd_state.arg[3];
				if (n & 0x1FF) {
					sd_state.substate = SD_SUBSTATE_IDLE;
					return 0x60;
				}

				n >>= 9;
				if (n >= sd_state.size_blocks) {
					sd_state.substate = SD_SUBSTATE_IDLE;
					return 0x60;
				}

				fseek(sd_state.fp, n * 512, SEEK_SET);
				sd_state.args = 540;
				memset(sd_state.arg, 0xFF, 540);
				sd_state.arg[530] = 0x0;
				sd_state.arg[515] = 0xFE;
				for (i = 0; i < 512; i++)
					sd_state.arg[514 - i] = fgetc(sd_state.fp);
				sd_state.substate = SD_SUBSTATE_SEND_REG;
				return 0xFF;
			} else {
				fprintf(stderr, "Invalid command %i\n", sd_state.command);
				sd_state.substate = SD_SUBSTATE_IDLE;
				return 0x4;
			}

		}

	} else if (sd_state.substate == SD_SUBSTATE_SEND_REG) {
		sd_state.args--;
		if (!sd_state.args)
			sd_state.substate = SD_SUBSTATE_IDLE;
		return sd_state.arg[sd_state.args];
	}
		

	return 0xFF;
}
