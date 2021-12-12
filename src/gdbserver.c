#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "gdbserver.h"

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
}

static void _cmd_continue_signal(GdbServer *server, char *arg){
}

static void _cmd_step(GdbServer *server, char *arg){
}

static void _cmd_step_signal(GdbServer *server, char *arg){
}

static void _cmd_mem_read(GdbServer *server, char *arg){
}

static void _cmd_mem_write(GdbServer *server, char *arg){
}

static void _cmd_reg_read(GdbServer *server, char *arg){
}

static void _cmd_reg_write(GdbServer *server, char *arg){
}

static void _cmd_reg_spec_read(GdbServer *server, char *arg){
}

static void _cmd_reg_spec_write(GdbServer *server, char *arg){
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
}

static void _cmd_break_remove(GdbServer *server, char *arg){
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
	server->send_byte(GDB_SERVER_FOOTER);

	for (int i = 0; i < len; i++) {
		server->send_byte(s[i]);
		checksum += i;
	}

	sprintf((char *) checksum_str, "%hhX", checksum);

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
		_reply_simple(server, "");
	}
}


void gdbserver_run(GdbServer *server) {
	uint8_t buf[300], c;
	ParseState state = PARSE_STATE_HEADER, state_next;

	int i = 0;
	for (;;) {

		c = server->recv_byte();
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
						fprintf(stderr, "ack\n");
						break;

					case GDB_SERVER_REPLY_NACK:
						fprintf(stderr, "nack\n");
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
				buf[i++] = c;

		}


	state = state_next;
	}

}
