#include "audio.h"
#include "mem.h"
#include "interrupt.h"
#include "chipset.h"
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <SDL/SDL.h>

static bool enabled = false;
static uint32_t sample_rate = 512;
static uint8_t buffer_size = 3;
static uint32_t buffer_pos = 0x0;
static int8_t buffer = 0;
static volatile int8_t thread_ack = 0;

static void _copy_audio(void *junk, uint8_t *data, int bytes) {
	int16_t *buff = (void *) data;
	int8_t *read_buff = mem->llram + (buffer * bytes) + buffer_pos;
	int i;
		
	while (thread_ack != 1) {
		usleep(10);
	}

	for (i = 0; i < (bytes >> 2); i++) {
		buff[i<<1] = 0x100 * (read_buff[i<<1] - 128);
		buff[(i<<1)+1] = 0x100 * (read_buff[(i<<1) + 1] - 128);
	}

	thread_ack = 0;
	interrupt_trig(CHIPSET_INT_NUM_AUDIO);
}


uint32_t audio_io_read(uint32_t addr) {
	addr &= 0xFC;
	if (addr == 0x4) {
		thread_ack = 1;
		return buffer;
	} else
		return fprintf(stderr, "[AUDIO] Unhandled read from port 0x%X\n", addr), ~0;
}


void audio_io_write(uint32_t addr, uint32_t data) {
	addr &= 0xFC;
	if (addr == 0x0) {
		SDL_AudioSpec spec;
		if (enabled)
			SDL_CloseAudio();
		enabled = !!data;
		if (enabled) {
			spec.freq = 24576000 / sample_rate;
			spec.format = AUDIO_S16SYS;
			spec.channels = 2;
			spec.samples = 32 << buffer_size;
			spec.callback = _copy_audio;
			spec.userdata = NULL;
			fprintf(stderr, "[AUDIO] Enabled audio, fs=%i Hz, bs=%i\n", spec.freq, spec.samples * 2);
			SDL_OpenAudio(&spec, NULL);
			SDL_PauseAudio(0);
			thread_ack = 1;
		}
	} else if (addr == 0x8) {
		buffer_pos = data & 0x7FFFF;
		buffer_pos &= ~(0x7FF >> (4 - buffer_size));
	} else if (addr == 0xC) {
		buffer_size = data & 0x3;
	} else if (addr == 0x10) {
		data &= 0x3FF;
		if (!data)
			data = 512;
		sample_rate = data;
	} else {
		fprintf(stderr, "[AUDIO] Unhandled write to port 0x%X\n", addr);
	}
}
