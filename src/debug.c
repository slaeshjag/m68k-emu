#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "debug.h"

int listensock, sock;


void debug_init() {
	struct sockaddr_in addr = {};
	
	listensock = socket(AF_INET, SOCK_STREAM, 0);
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(1234);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	
	if(bind(listensock, &addr, sizeof(addr)) < 0) {
		sock = -1;
		return;
	}
	
	printf("Attach gdb by using target remote :1234\n");
	
	listen(listensock, 1);
	sock = accept(listensock, NULL, NULL);
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
