#ifndef GDBSERVER_H_
#define GDBSERVER_H_

#include <semaphore.h>

#define GDB_SERVER_HEADER '$'
#define GDB_SERVER_FOOTER '#'
#define GDB_SERVER_ESCAPE '}'
#define GDB_SERVER_BREAK 0x3
#define GDB_SERVER_REPLY_ACK '+'
#define GDB_SERVER_REPLY_NACK '-'

#define GDB_SERVER_REPLY_OK "OK"
#define GDB_SERVER_REPLY_ERROR "E"

#define GDB_SERVER_ESCAPE_FLAG 0x20

typedef enum GdbServerCommand GdbServerCommand;
enum GdbServerCommand {
	GDB_SERVER_COMMAND_QUERY,
	GDB_SERVER_COMMAND_CONTINUE,
	GDB_SERVER_COMMAND_STEP,
	GDB_SERVER_COMMAND_MEM_READ,
	GDB_SERVER_COMMAND_MEM_WRITE,
	GDB_SERVER_COMMAND_REG_READ,
	GDB_SERVER_COMMAND_REG_WRITE,
	GDB_SERVER_COMMAND_REG_SPEC_READ,
	GDB_SERVER_COMMAND_REG_SPEC_WRITE,
	GDB_SERVER_COMMAND_QOFFSETS,
	GDB_SERVER_COMMAND_QSUPPORTED,
	GDB_SERVER_COMMAND_BREAK_INSERT,
	GDB_SERVER_COMMAND_BREAK_REMOVE,

	GDB_SERVER_COMMANDS,

	/* Disabled commands */
	GDB_SERVER_COMMAND_EXTENDED,
	GDB_SERVER_COMMAND_QSYMBOL,
	GDB_SERVER_COMMAND_VCONT,
	GDB_SERVER_COMMAND_LOAD_BINARY,
	GDB_SERVER_COMMAND_CONTINUE_SIGNAL,
	GDB_SERVER_COMMAND_STEP_SIGNAL,
};

typedef struct GdbServer GdbServer;
struct GdbServer {
	void (*send_byte)(uint8_t c);
	uint8_t (*recv_byte)();

	pthread_t thread;
	sem_t recv_sem;
	sem_t recv_ack_sem;
	uint8_t c;
	int state;
};

GdbServer *gdbserver_init(uint8_t (*recv_byte)(), void (*send_byte)(uint8_t));
void gdbserver_run(GdbServer *server);

#endif
