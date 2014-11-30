#ifndef __SD_H__
#define	__SD_H__


uint8_t spi_sd_send_recv(uint8_t byte);
void spi_sd_init(const char *sd_image);

#endif
