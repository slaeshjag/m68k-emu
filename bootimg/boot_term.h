#ifndef __BOOT_TERM_H__
#define	__BOOT_TERM_H__

void term_init();
void term_putc(int c, int color);
void term_puts(char *str, int color);

#endif
