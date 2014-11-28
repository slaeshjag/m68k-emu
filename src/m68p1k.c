#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "cpu/m68000.h"
#include "mem.h"
#include "vga.h"
#include "signal.h"

void die(int arne) {
	SDL_Quit();
	exit(0);
}

static void *run_cpu(void *data) {
	m68030_start();
	
	pthread_exit(NULL);
}


int main(int argc, char **argv) {
	pthread_attr_t attr;
	pthread_t thread;
	
	signal(SIGINT, die);
	mem_init();
	vga_init();
	m68030_init();
	m68030_reset(true);
	
	pthread_attr_init(&attr);
	pthread_create(&thread, &attr, run_cpu, NULL);
	
	for (;;) {
		vga_render_line();
	}


	return 0;
}
