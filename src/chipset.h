#ifndef __CHIPSET_H__
#define	__CHIPSET_H__

void chipset_int_set(int int_no, int set_unset);
void chipset_write_io(unsigned int addr, unsigned int data);

enum ChipsetIntNum {
	ChipsetIntNumVGAVSync	= 5,
	ChipsetIntNumAudio	= 6,
};

#endif
