#include <stdint.h>
#include <stdlib.h>
#include <string.h>


static int _cur_pos = 0;
static uint8_t _data_buff[9];


struct MemoryBlock {
	uint32_t		addr;
	uint32_t		len;
};


static struct MemoryBlock *_mem_block;
static int _mem_blocks = 0;
static int _check_enabled = 0;

void mdebug_write(uint8_t value) {
	struct MemoryBlock mblk;
	int q, i;
	
	_data_buff[_cur_pos++] = value;
	if (_cur_pos == 9) {
		mblk.addr = (((uint32_t) _data_buff[1]) << 24) | (((uint32_t) _data_buff[2]) << 16) | (((uint32_t) _data_buff[3]) << 8) | ((uint32_t) _data_buff[4]);
		mblk.len = (((uint32_t) _data_buff[5]) << 24) | (((uint32_t) _data_buff[6]) << 16) | (((uint32_t) _data_buff[7]) << 8) | ((uint32_t) _data_buff[8]);
		_cur_pos = 0;
		if (!_data_buff[0]) {
			q = _mem_blocks++;
			_mem_block = realloc(_mem_block, sizeof(*_mem_block) * _mem_blocks);
			_mem_block[q] = mblk;
		} else if (_data_buff[0] == 1) {
			for (i = 0; i < _mem_blocks; i++) {
				if (_mem_block[i].addr == mblk.addr) {
					memmove(&_mem_block[i], &_mem_block[i + 1], (_mem_blocks - 1 - i) * sizeof(*_mem_block));
					_mem_blocks--;
				}
			}
		} else if (_data_buff[0] == 2) {
			_check_enabled = 0;
		} else if (_data_buff[0] == 3) {
			_check_enabled = 1;
		}
	}
}


int mdebug_check_pointer(uint32_t pointer, int size) {
	int i;

	if (!_check_enabled)
		return 1;

	for (i = 0; i < _mem_blocks; i++) {
		if (_mem_block[i].addr + _mem_block[i].len < pointer)
			continue;
		if (pointer + size < _mem_block[i].addr)
			continue;
		if (pointer >= _mem_block[i].addr && pointer + size < _mem_block[i].addr + _mem_block[i].len)
			return 1;
	}

	return 0;
}
