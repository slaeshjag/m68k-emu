#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
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


int main(int argc, char **argv) {
	bool new_map; 
	pthread_attr_t attr;
	pthread_t thread;
	
	if(argc < 4) {
		fprintf(stderr, "Usage: m68k <new | newspiboot | old> <debug | nodebug> <bootimg.img> [spi rom] [sd card]\n");
		return 1;
	}

	signal(SIGINT, die);
	if (argc >= 6)
		spi_sd_init(argv[5]);
	if (argc >= 5)
		spi_rom_init(argv[4]);
	if (!(new_map = !strcasecmp(argv[1], "new"))) {
		new_map = !strcasecmp(argv[1], "newspiboot");
		chipset_set_boot_switch(1);
	} 
	_debug = !strcasecmp(argv[2], "debug");
	if (_debug) {
		debug_init();
	}
	mem_init(argv[3], new_map);
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
