#include <stdio.h>
#include <stdlib.h>
#include "m68k.h"
#include "mem.h"
#include "vga.h"
#include "signal.h"

void die(int arne) {
	SDL_Quit();
	exit(0);
}


int main(int argc, char **argv) {
	signal(SIGINT, die);
	mem_init();
	vga_init();
	m68k_init();
	m68k_set_cpu_type(M68K_CPU_TYPE_68020);
	m68k_pulse_reset();

	for (;;) {
		m68k_execute(800);
		vga_render_line();
	}


	return 0;
}
