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
static uint8_t audio_buffer[1024];
static int audio_buffer_pos = 512;


static void _copy_audio(void *junk, uint8_t *data, int bytes) {
	int i, j;
	int16_t *buff = (void *) data;
	//fprintf(stderr, "bytes=%i\n", bytes);
	
	for (i = 0; i < (bytes >> 2);) {
		for (; audio_buffer_pos < (64 << buffer_size) && i < (bytes >> 2); audio_buffer_pos++, i++) {
			if (i == bytes)
				return;
			buff[i<<1] = 0x100 * (audio_buffer[audio_buffer_pos<<1] - 128);
			buff[(i<<1)+1] = 0x100 * (audio_buffer[(audio_buffer_pos<<1) + 1] - 128);
		}
		
		if (audio_buffer_pos < (64 << buffer_size))
			return;

		int8_t *read_buff = mem->llram + (buffer * (128 << buffer_size)) + buffer_pos;
	
		audio_buffer_pos = 0;
	
		while (thread_ack != 1) {
			usleep(10);
		}

		buffer = !buffer;
		memcpy(audio_buffer, read_buff, (128 << buffer_size));
	
		thread_ack = 0;
		interrupt_trig(CHIPSET_INT_NUM_AUDIO);
	}
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
			spec.samples = 64 << buffer_size;
			spec.callback = _copy_audio;
			spec.userdata = NULL;
			fprintf(stderr, "[AUDIO] Enabled audio, fs=%i Hz, bs=%i\n", spec.freq, spec.samples * 2);
			SDL_OpenAudio(&spec, NULL);
			SDL_PauseAudio(0);
			thread_ack = 1;
		}
	} else if (addr == 0x8) {
		buffer_pos = data & 0x7FFFF;
		buffer_pos &= ~(0x3FF >> (3 - buffer_size));
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
