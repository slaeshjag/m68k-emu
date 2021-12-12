#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include "debug.h"

#define	MAX_BREAKPOINTS	32

int listensock, sock;


static uint32_t _breakpoint[MAX_BREAKPOINTS];
static sem_t _cpu_run_sem;
static int _cpu_run;
static volatile int _cpu_stopped;


static int _check_breakpoint(uint32_t pc) {
	int i;

	for (i = 0; i < MAX_BREAKPOINTS; i++)
		if (_breakpoint[i] == pc)
			return i;
	return -1;
}


static int _add_breakpoint(uint32_t pc) {
	int i;

	for (i = 0; i < MAX_BREAKPOINTS; i++)
		if (!_breakpoint[i])
			return _breakpoint[i] = pc, i;
	return -1;
}


static uint32_t _get_breakpoint(int id) {
	return _breakpoint[id];
}


static void _remove_breakpoint(int id) {
	_breakpoint[id] = 0;
}


// Read memory with x_get_byte


static void _set_cpu_run(int run) {
	_cpu_run = run;
	if (run)
		sem_post(&_cpu_run_sem);
	else
		while (!_cpu_stopped)
			usleep(10);
}





void debug_init() {
	struct sockaddr_in addr = {};

	sem_init(&_cpu_run_sem, 0, 0);
	listensock = socket(AF_INET, SOCK_STREAM, 0);
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(1234);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	
	if(bind(listensock, &addr, sizeof(addr)) < 0) {
		sock = -1;
		return;
	}
	listen(listensock, 1);
	sock = accept(listensock, NULL, NULL);
}


void debug_hook(uint32_t pc) {
	int brk;

	if (!_cpu_run) {
		_cpu_stopped = 1;
		sem_wait(&_cpu_run_sem);
		_cpu_stopped = 0;
	}

	if ((brk = _check_breakpoint(pc)) < 0)
		return;
}


void debug_send(uint8_t byte) {
	if(sock < 0)
		return;
	send(sock, &byte, 1, 0);
}

uint8_t debug_recv() {
	uint8_t ret;
	if(sock < 0)
		return 0;
	recv(sock, &ret, 1, 0);
	return ret;
}
