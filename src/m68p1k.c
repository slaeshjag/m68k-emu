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

void die(int arne) {
	SDL_Quit();
	exit(0);
}

static void *run_cpu(void *data) {
	m68krash_start();
	
	pthread_exit(NULL);
}


int main(int argc, char **argv) {
	pthread_attr_t attr;
	pthread_t thread;
	
	if(argc < 2) {
		fprintf(stderr, "Usage: m68k <bootimg.img>\n");
		return 1;
	}

	signal(SIGINT, die);
	debug_init();
	if (argc >= 3)
		spi_sd_init(argv[2]);
	mem_init(argv[1]);
	vga_init();
	m68krash_init(M68K_CPU_030);
	m68krash_reset(true);
	
	pthread_attr_init(&attr);
	pthread_create(&thread, &attr, run_cpu, NULL);
	
	for (;;) {
		vga_render_line();
		spi_loop(800);
	}


	return 0;
}
