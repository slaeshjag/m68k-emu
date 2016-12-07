#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "cpu/m68krash.h"
#include "mem.h"
#include "vga.h"
#include "signal.h"
#include "spi.h"
#include "sd.h"
#include "debug.h"
#include "uart.h"
#include "spirom.h"

void die(int arne) {
	SDL_Quit();
	exit(0);
}

static void *run_cpu(void *data) {
	m68krash_start();
	
	pthread_exit(NULL);
}


int main(int argc, char **argv) {
	bool new_map, debug;
	pthread_attr_t attr;
	pthread_t thread;
	
	if(argc < 4) {
		fprintf(stderr, "Usage: m68k <new | old> <debug | nodebug> <bootimg.img> [spi rom] [sd card]\n");
		return 1;
	}

	signal(SIGINT, die);
	if (argc >= 6)
		spi_sd_init(argv[5]);
	if (argc >= 5)
		spi_rom_init(argv[4]);
	new_map = !strcasecmp(argv[1], "new");
	debug = !strcasecmp(argv[2], "debug");
	if (debug)
		debug_init();
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
