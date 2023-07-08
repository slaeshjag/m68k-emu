#ifndef UART_H__
#define	UART_H__


void uart_init();
uint32_t uart_handle_read(unsigned int addr);
void uart_handle_write(unsigned int addr, unsigned int data);
void uart_irq_handle();


#endif
