#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

#include "debug.h"
#include "gdbserver.h"

#include "cpu/m68000.h"

typedef enum ParseState ParseState;
enum ParseState {
	PARSE_STATE_HEADER,
	PARSE_STATE_COMMAND,
	PARSE_STATE_FOOTER,
	PARSE_STATE_CHECKSUM,
	PARSE_STATE_ESCAPE,
};

static void _cmd_extended(GdbServer *server, char *arg);
static void _cmd_query(GdbServer *server, char *arg);
static void _cmd_continue(GdbServer *server, char *arg);
static void _cmd_continue_signal(GdbServer *server, char *arg);
static void _cmd_step(GdbServer *server, char *arg);
static void _cmd_step_signal(GdbServer *server, char *arg);
static void _cmd_mem_read(GdbServer *server, char *arg);
static void _cmd_mem_write(GdbServer *server, char *arg);
static void _cmd_reg_read(GdbServer *server, char *arg);
static void _cmd_reg_write(GdbServer *server, char *arg);
static void _cmd_reg_spec_read(GdbServer *server, char *arg);
static void _cmd_reg_spec_write(GdbServer *server, char *arg);
static void _cmd_qoffsets(GdbServer *server, char *arg);
static void _cmd_qsupported(GdbServer *server, char *arg);
static void _cmd_qsymbol(GdbServer *server, char *arg);
static void _cmd_vcont(GdbServer *server, char *arg);
static void _cmd_load_binary(GdbServer *server, char *arg);
static void _cmd_break_insert(GdbServer *server, char *arg);
static void _cmd_break_remove(GdbServer *server, char *arg);

static void _reply_simple(GdbServer *server, const char *s);
static void _reply_ack(GdbServer *server, bool ack);
static void _reply(GdbServer *server, const uint8_t *s, size_t len);
static void _run_command(GdbServer *server, uint8_t *buf, size_t len);

static const char *_command_str[] = {
	[GDB_SERVER_COMMAND_EXTENDED] = "!",
	[GDB_SERVER_COMMAND_QUERY] = "?",
	[GDB_SERVER_COMMAND_CONTINUE] = "c",
	[GDB_SERVER_COMMAND_CONTINUE_SIGNAL] = "C",
	[GDB_SERVER_COMMAND_STEP] = "s",
	[GDB_SERVER_COMMAND_STEP_SIGNAL] = "S",
	[GDB_SERVER_COMMAND_MEM_READ] = "m",
	[GDB_SERVER_COMMAND_MEM_WRITE] = "M",
	[GDB_SERVER_COMMAND_REG_READ] = "g",
	[GDB_SERVER_COMMAND_REG_WRITE] = "G",
	[GDB_SERVER_COMMAND_REG_SPEC_READ] = "p",
	[GDB_SERVER_COMMAND_REG_SPEC_WRITE] = "P",
	[GDB_SERVER_COMMAND_QOFFSETS] = "qOffsets",
	[GDB_SERVER_COMMAND_QSUPPORTED] = "qSupported",
	[GDB_SERVER_COMMAND_QSYMBOL] = "qSymbol",
	[GDB_SERVER_COMMAND_VCONT] = "vCont?",
	[GDB_SERVER_COMMAND_LOAD_BINARY] = "X",
	[GDB_SERVER_COMMAND_BREAK_INSERT] = "Z",
	[GDB_SERVER_COMMAND_BREAK_REMOVE] = "z",
};

static void (*_command_func[])(GdbServer *server, char *arg) = {
	[GDB_SERVER_COMMAND_EXTENDED] = _cmd_extended,
	[GDB_SERVER_COMMAND_QUERY] = _cmd_query,
	[GDB_SERVER_COMMAND_CONTINUE] = _cmd_continue,
	[GDB_SERVER_COMMAND_CONTINUE_SIGNAL] = _cmd_continue_signal,
	[GDB_SERVER_COMMAND_STEP] = _cmd_step,
	[GDB_SERVER_COMMAND_STEP_SIGNAL] = _cmd_step_signal,
	[GDB_SERVER_COMMAND_MEM_READ] = _cmd_mem_read,
	[GDB_SERVER_COMMAND_MEM_WRITE] = _cmd_mem_write,
	[GDB_SERVER_COMMAND_REG_READ] = _cmd_reg_read,
	[GDB_SERVER_COMMAND_REG_WRITE] = _cmd_reg_write,
	[GDB_SERVER_COMMAND_REG_SPEC_READ] = _cmd_reg_spec_read,
	[GDB_SERVER_COMMAND_REG_SPEC_WRITE] = _cmd_reg_spec_write,
	[GDB_SERVER_COMMAND_QOFFSETS] = _cmd_qoffsets,
	[GDB_SERVER_COMMAND_QSUPPORTED] = _cmd_qsupported,
	[GDB_SERVER_COMMAND_QSYMBOL] = _cmd_qsymbol,
	[GDB_SERVER_COMMAND_VCONT] = _cmd_vcont,
	[GDB_SERVER_COMMAND_LOAD_BINARY] = _cmd_load_binary,
	[GDB_SERVER_COMMAND_BREAK_INSERT] = _cmd_break_insert,
	[GDB_SERVER_COMMAND_BREAK_REMOVE] = _cmd_break_remove,
	
};


static void _cmd_extended(GdbServer *server, char *arg){
}

static void _cmd_query(GdbServer *server, char *arg){
	_reply_simple(server, "S05");
}

static void _cmd_continue(GdbServer *server, char *arg){
	debug_cpu_set_run(1);
	debug_cpu_wait();
	_reply_simple(server, "S05");
}

static void _cmd_continue_signal(GdbServer *server, char *arg){
}

static void _cmd_step(GdbServer *server, char *arg){
	debug_cpu_step();
	debug_cpu_wait();
	_reply_simple(server, "S05");
}

static void _cmd_step_signal(GdbServer *server, char *arg){
}

static void _cmd_mem_read(GdbServer *server, char *arg){
	uint32_t addr = 0, len = 0;
	char buff[257] = { 0 }, byte[3];
	int i;

	sscanf(arg, "%X,%X", &addr, &len);

	if (len > 128)
		len = 128;
	for (i = 0; i < len; i++)
		sprintf(byte, "%.2X", get_byte(addr + i)), strcat(buff, byte);
	_reply_simple(server, buff);
}

static void _cmd_mem_write(GdbServer *server, char *arg){
	uint32_t addr = 0, len = 0;
	char byte[3] = {0};
	int i, off;

	sscanf(arg, "%X,%X:%n", &addr, &len, &off);

	for (i = 0; i < len; i++) {
		byte[0] = arg[off + i*2];
		byte[1] = arg[off + i*2 + 1];

		put_byte(addr + i, strtoul(byte, NULL, 16));
	}

	_reply_simple(server, GDB_SERVER_REPLY_OK);
}

static void _cmd_reg_read(GdbServer *server, char *arg){
	char reg[9], buff[8*18+1] = { 0 };
	int i;

	/* d0 .. d7, a0 .. a7, status register, program counter*/
	for (i = 0; i < 8; i++)
		sprintf(reg, "%.8X", m68k_dreg(regs, i)), strcat(buff, reg);
	for (i = 0; i < 8; i++)
		sprintf(reg, "%.8X", m68k_areg(regs, i)), strcat(buff, reg);
	
	sprintf(reg, "%.8X", regs.sr), strcat(buff, reg);
	sprintf(reg, "%.8X", m68k_getpc()), strcat(buff, reg);

	_reply_simple(server, buff);
}

static void _cmd_reg_write(GdbServer *server, char *arg){
	char buf[9] = { 0 };
	uint32_t val[18];
	int i;

	if (strlen(arg) != 8*18) {
		_reply_simple(server, GDB_SERVER_REPLY_ERROR "01");
		return;
	}

	for (i = 0; i < 18; i++) {
		memcpy(buf, arg, 8);
		arg += 8;
		val[i] = strtoul(buf, NULL, 16);
	}

	/* d0 .. d7, a0 .. a7, status register, program counter*/
	for (i = 0; i < 8; i++) {
		m68k_dreg(regs, i) = val[i];
	}

	for (i = 0; i < 8; i++) {
		m68k_areg(regs, i) = val[i - 8];
	}
	
	regs.sr = val[16];
	m68k_setpc(val[17]);

	_reply_simple(server, GDB_SERVER_REPLY_OK);
}

static void _cmd_reg_spec_read(GdbServer *server, char *arg){
	char buf[8 + 1] = {0};
	uint32_t val = 0;
	uint8_t reg = 0;

	sscanf(arg, "%hhx", &reg);

	if (reg < 8)
		val = m68k_dreg(regs, reg);
	else if (reg < 16)
		val = m68k_areg(regs, reg - 8);
	else if (reg == 16)
		val = regs.sr;
	else if (reg == 17)
		val = m68k_getpc();
	else {
		_reply_simple(server, GDB_SERVER_REPLY_ERROR "01");
		return;
	}

	sprintf(buf, "%.8X", val);
	_reply_simple(server, buf);
}

static void _cmd_reg_spec_write(GdbServer *server, char *arg){
	uint32_t val = 0;
	uint8_t reg = 0;
	sscanf(arg, "%hhx=%x", &reg, &val);

	if (reg < 8)
		m68k_dreg(regs, reg) = val;
	else if (reg < 16)
		m68k_areg(regs, reg - 8) = val;
	else if (reg == 16)
		regs.sr = val;
	else if (reg == 17)
		m68k_setpc(val);
	else {
		_reply_simple(server, GDB_SERVER_REPLY_ERROR "01");
		return;
	}

	_reply_simple(server, GDB_SERVER_REPLY_OK);
}

static void _cmd_qoffsets(GdbServer *server, char *arg){
	_reply_simple(server, "Text=0;Data=0;Bss=0;");
}

static void _cmd_qsupported(GdbServer *server, char *arg){
	_reply_simple(server, "PacketSize=280");
}

static void _cmd_qsymbol(GdbServer *server, char *arg){
}

static void _cmd_vcont(GdbServer *server, char *arg){
}

static void _cmd_load_binary(GdbServer *server, char *arg){
}

static void _cmd_break_insert(GdbServer *server, char *arg){
	int type, kind;
	uint32_t address;

	sscanf(arg, "%X,%X,%X", &type, &address, &kind);

	switch (type) {
		case GDB_SERVER_BREAKPOINT_TYPE_HARDWARE:
			debug_breakpoint_add(address);
			_reply_simple(server, GDB_SERVER_REPLY_OK);
			break;
		default:
			_reply_simple(server, "");
	}
}

static void _cmd_break_remove(GdbServer *server, char *arg){
	int type, kind;
	uint32_t address;

	sscanf(arg, "%X,%X,%X", &type, &address, &kind);

	switch (type) {
		case GDB_SERVER_BREAKPOINT_TYPE_HARDWARE:
			debug_breakpoint_remove(address);
			_reply_simple(server, GDB_SERVER_REPLY_OK);
			break;
		default:
			_reply_simple(server, "");
	}
}

static void _reply_simple(GdbServer *server, const char *s) {
	_reply(server, (uint8_t *) s, strlen(s));
}

static void _reply_ack(GdbServer *server, bool ack) {
	server->send_byte(ack ? GDB_SERVER_REPLY_ACK : GDB_SERVER_REPLY_NACK);
}

static void _reply(GdbServer *server, const uint8_t *s, size_t len) {
	uint8_t checksum_str[3];
	uint8_t checksum = 0;

	server->send_byte(GDB_SERVER_HEADER);

	for (int i = 0; i < len; i++) {
		uint8_t c;

		switch (s[i]) {
			case GDB_SERVER_HEADER:
			case GDB_SERVER_FOOTER:
			case GDB_SERVER_ESCAPE:
				server->send_byte(GDB_SERVER_ESCAPE);
				checksum += GDB_SERVER_ESCAPE;
				c = s[i] ^ GDB_SERVER_ESCAPE_FLAG;
				break;

			default:
				c = s[i];
				break;
		}

		server->send_byte(c);
		checksum += c;
	}

	sprintf((char *) checksum_str, "%02hhX", checksum);

	server->send_byte(GDB_SERVER_FOOTER);
	server->send_byte(checksum_str[0]);
	server->send_byte(checksum_str[1]);
}

static void _run_command(GdbServer *server, uint8_t *buf, size_t len) {
	int i;

	for (i = 0; i < GDB_SERVER_COMMANDS; i++) {
		size_t cmdlen = strlen(_command_str[i]);
		if (!strncmp(_command_str[i], (char *) buf, cmdlen)) {
			fprintf(stderr, "command %s\n", buf);
			_command_func[i](server, (char *) (buf + cmdlen));
			break;
		}
	}

	if (i == GDB_SERVER_COMMANDS) {
		/* Unknown commands get empty reply */
		fprintf(stderr, "unknown command %s\n", buf);
		_reply_simple(server, "");
	}
}

void *_recv_thread(void *data) {
	GdbServer *server = data;
	ParseState state = PARSE_STATE_FOOTER;
	uint8_t c;

	for (;;) {
		c = server->recv_byte();
		if (c == GDB_SERVER_BREAK) {
 			if (state == PARSE_STATE_FOOTER) {
				debug_cpu_set_run(0);
				continue;
			} else {
				fprintf(stderr, "got break but state is %i\n", state);
			}
		}
		
		sem_wait(&server->recv_ack_sem);
		state = server->state;
		server->c = c;
		sem_post(&server->recv_sem);
	}

}

static uint8_t _recv_internal(GdbServer *server, ParseState state) {
	uint8_t c;

	sem_wait(&server->recv_sem);
	c = server->c;
	server->state = state;
	sem_post(&server->recv_ack_sem);

	return c;
}

GdbServer *gdbserver_init(uint8_t (*recv_byte)(), void (*send_byte)(uint8_t)) {
	GdbServer *server = NULL;

	if (!(server = malloc(sizeof (GdbServer)))) {
		return NULL;
	}

	server->recv_byte = recv_byte;
	server->send_byte = send_byte;

	sem_init(&server->recv_sem, 0, 0);
	sem_init(&server->recv_ack_sem, 0, 1);

	server->state = PARSE_STATE_HEADER;

	return server;
}


void gdbserver_run(GdbServer *server) {
	uint8_t buf[300], c;
	ParseState state = server->state, state_next;

	pthread_create(&server->thread, NULL, _recv_thread, server);

	int i = 0;
	for (;;) {

		c = _recv_internal(server, state);
		state_next = state;

		switch (state) {
			case PARSE_STATE_HEADER:
				switch (c) {
					case GDB_SERVER_HEADER:
						state_next = PARSE_STATE_COMMAND;
						break;

					case GDB_SERVER_BREAK:
						fprintf(stderr, "break\n");
						break;

					case GDB_SERVER_REPLY_ACK:
						break;

					case GDB_SERVER_REPLY_NACK:
						break;

				}

				break;

			case PARSE_STATE_COMMAND:
				switch (c) {
					case GDB_SERVER_FOOTER:
						state_next = PARSE_STATE_FOOTER;
						break;

					case GDB_SERVER_ESCAPE:
						state_next = PARSE_STATE_ESCAPE;
						break;

					default:
						buf[i++] = c;
						break;
				}

				break;

			case PARSE_STATE_FOOTER:
				state_next = PARSE_STATE_CHECKSUM;
				buf[i] = 0;
				break;

			case PARSE_STATE_CHECKSUM:
				_reply_ack(server, true);
				_run_command(server, buf, i);

				i = 0;
				state_next = PARSE_STATE_HEADER;
				break;

			case PARSE_STATE_ESCAPE:
				buf[i++] = c ^ GDB_SERVER_ESCAPE_FLAG;

		}


		state = state_next;
	}

}

