#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <getopt.h>

#include "cpu/m68krash.h"
#include "cpu/m68000.h"
#include "mem.h"
#include "vga.h"
#include "signal.h"
#include "spi.h"
#include "sd.h"
#include "debug.h"
#include "uart.h"
#include "spirom.h"
#include "chipset.h"

#define CONFIGDIR ".m68kemu"

static bool _debug;

void die(int arne) {
	SDL_Quit();
	exit(0);
}


static void *run_cpu(void *data) {
	for (;;) {
		if (_debug) {
			debug_hook(m68k_getpc());
			regs.spcflags |= SPCFLAG_MODE_SINGLESTEP;
		}

		m68krash_start();
	}
	
	pthread_exit(NULL);
}


bool file_exists(const char *fname) {
	FILE *f;
	f = fopen(fname, "r");

	if (!f)
		return false;
	
	fclose(f);
	return true;
}

char *find_config_file(const char *fname) {
	char *path = NULL;
	char *home = getenv("HOME");

	if (!home)
		return NULL;

	int len = snprintf(NULL, 0, "%s/%s/%s", home, CONFIGDIR, fname);
	
	if (!(path = malloc(len + 1)))
		return NULL;

	sprintf(path, "%s/%s/%s", home, CONFIGDIR, fname);

	if (!file_exists(path)) {
		free(path);
		return NULL;
	}

	return path;
}


void usage() {
	fprintf(stderr, "Usage: m68k-emu [-h] [-s] [-d] [-b <boot rom>] [-r <spi rom image>] [-c <sd card image>]\n\n");
	fprintf(stderr, "  Option Name          Argument             Description\n\n");
	fprintf(stderr, "  -h | --help                               Show this help\n");
	fprintf(stderr, "  -s | --serial-boot                        Boot from serial port instead of SPI rom\n");
	fprintf(stderr, "  -d | --debug                              Run in debug mode\n");
	fprintf(stderr, "  -b | --boot-rom      <boot rom file>      Specify stage 1 boot rom file\n");
	fprintf(stderr, "  -r | --spi-rom       <spi rom image>      Specify spi rom file\n");
	fprintf(stderr, "  -c | --sd-card       <boot rom file>      Specify sd card file\n");
	fprintf(stderr, "\n");

}
int main(int argc, char **argv) {
	static struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"serial-boot", no_argument, 0, 's'},
		{"debug", no_argument, 0, 'd'},
		{"boot-rom", required_argument, 0, 'b'},
		{"spi-rom", required_argument, 0, 'r'},
		{"sd-card", required_argument, 0, 'c'},
		{NULL, 0, 0, 0},
	};

	pthread_attr_t attr;
	pthread_t thread;
	bool boot_switch = true;
	bool new_map = true; 

	char *boot_rom = NULL;
	char *spi_rom = NULL;
	char *sd_card = NULL;

	boot_rom = find_config_file("boot.bin");
	spi_rom = find_config_file("spi.img");
	sd_card = find_config_file("sd.img");

	for (;;) {
		int c;
		int option_index = 0;

		c = getopt_long(argc, argv, "hsdb:r:c:", long_options, &option_index);

		if (c == -1)
			break;

		switch (c) {
			case 'h':
				usage();
				return 0;

			case 's':
				boot_switch = false;
				break;

			case 'd':
				_debug = true;
				break;

			case 'b':
				boot_rom = strdup(optarg);
				break;

			case 'r':
				spi_rom = strdup(optarg);
				break;

			case 'c':
				sd_card = strdup(optarg);
				break;

			default:
				fprintf(stderr, "Unknown command line option '%c'\n", c);
				return 1;
		}
	}

	if (boot_rom == NULL) {
		fprintf(stderr, "Boot rom missing, either put a boot rom in the config directory or specify one on the command line\n");
		return 1;
	}

	if (spi_rom)
		spi_rom_init(spi_rom);
	
	if (sd_card)
		spi_sd_init(sd_card);
	
	signal(SIGINT, die);

	chipset_set_boot_switch(boot_switch);

	if (_debug) {
		debug_init();
	}

	mem_init(boot_rom, new_map);
	vga_init(new_map);
	uart_init();
	m68krash_init(new_map?M68K_CPU_040:M68K_CPU_030);
	m68krash_reset(true);
	
	pthread_attr_init(&attr);
	pthread_create(&thread, &attr, run_cpu, NULL);
	
	for (;;) {
		vga_render_line();
		if (!new_map)
			spi_loop(800);
	}


	return 0;
}
