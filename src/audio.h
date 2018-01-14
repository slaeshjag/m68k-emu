#ifndef AUDIO_H__
#define	AUDIO_H__

#include <stdint.h>

void audio_io_write(uint32_t addr, uint32_t data);
uint32_t audio_io_read(uint32_t addr);
void audio_set_volume_l(uint8_t vol);
void audio_set_volume_r(uint8_t vol);

#endif
