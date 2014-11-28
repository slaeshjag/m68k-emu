#include "spi.h"
#include "sd.h"
#include <stdint.h>

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
	SD_SUBSTATE_SEND_REG;
	SD_SUBSTATE_SEND_BLOCK;
};

struct {
	int		init_count;
	enum SdState	state;
	enum SdSubState	substate;
	uint8_t		arg[32];
	int		args;
	int		command;
	int		acmd_start;
	int		wait_cycles;
	int		init_wait;

	int		send_block;
	int		send_count;

	int		sdcard_size;
	int		sdcard_multipl;
	int		size_blocks;
} sd_state;


uint8_t spi_sd_send_recv(uint8_t byte) {
	int n;

	if (sd_state.state == SD_STATE_NOCARD)
		return 0xFF;
	
	/* Make sure that we're not selected and that the clock is slow */
	if ((spi_state.line_select & 0x8) && (spi_state.line_select & 0x2)) {
		sd_state.init_count++;
	}

	if (!(spi_state.line_select & 0x8))
		return 0xFF;

	if (sd_state.init_count < 74) {
		sd_state.init_count = 0;
		return 0xFF;
	}
	/* End of MMC mimicing */

	if (sd_state.substate == SD_SUBSTATE_IDLE) {
		if ((byte & 0xC0) != 0x40)
			return 0xFF;
		sd_state.command = byte & 0x3F;
		if (sd_state.acmd_start)
			sd_state.command |= 0x40, sd_state.acmd_start = 0;
		sd_state.substate = SD_SUBSTATE_CMD;
		return 0xFF;
	} else if (sd_state.substate == SD_SUBSTATE_CMD) {
		sd_state.arg[sd_state.args++] = byte;
		if (sd_state.args == 5) {
			sd_state.wait_cycles = rand() & 0x7;
			sd_state.substate = SD_SUBSTATE_WAIT;
		}
		
		return 0xFF;
	} else if (sd_state.substate == SD_SUBSTATE_WAIT) {
		if (sd_state.wait_cycles > 0) {
			sd_state.wait_cycles--;
			return 0xFF;
		}
		
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
				if (--sd_state.init_wait == 0) {
					sd_state.state = SD_STATE_INIT;
					sd_state.substate = SD_SUBSTATE_IDLE;
					return 0x0;
				} else {
					sd_state.substate = SD_SUBSTATE_IDLE;
					return 0x1;
				}
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
			} else if (sd_state.command == 8) {	/* Read CSD */
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
			} else if (sd_state.command == 17 || sd_state.command == 18) {
				n = sd_state.arg[0] << 24, n |= sd_state.arg[1] << 16;
				n |= sd_state.arg[2] << 8, n |= sd_state.arg[3];
				if (n & 0x1FF) {
					sd_state.substate = SD_SUBSTATE_IDLE;
					return 0x60;
				}

				n >> 9;
				if (n >= sd_state.size_blocks) {
					sd_state.substate = SD_SUBSTATE_IDLE;
					return 0x60;
				}

				sd_state.send_block = n;
				sd_state.send_count = 0;
				sd_state.substate = SD_SUBSTATE_SEND_BLOCK;
				return 0x0;
			} else {
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
