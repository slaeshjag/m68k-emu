#ifndef __DEBUG_H__
#define __DEBUG_H__
#include <stdint.h>

void debug_init();
void debug_send(uint8_t byte);
uint8_t debug_recv();
void debug_hook(uint32_t pc);
void debug_cpu_step();
void debug_cpu_wait();
void debug_cpu_set_run(int run);
int debug_breakpoint_add(uint32_t pc);
void debug_breakpoint_remove(uint32_t address);


#endif
