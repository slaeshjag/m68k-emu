#ifndef __CHIPSET_H__
#define	__CHIPSET_H__

void chipset_int_set(int int_no, int set_unset);
void chipset_write_io(unsigned int addr, unsigned int data);

enum ChipsetIntNum {
	CHIPSET_INT_NUM_SPI_DONE = 3,
	CHIPSET_INT_NUM_EXTERNAL,
	CHIPSET_INT_NUM_VGA_VSYNC,
	CHIPSET_INT_NUM_AUDIO,
	#if 0
	ChipsetIntNumSPIDone	= 3,	/* SPI transaction done */
	ChipsetIntNumSPI3	= 4,	/* External SPI-port interrupt pin */
	ChipsetIntNumVGAVSync	= 5,
	ChipsetIntNumAudio	= 6,
	#endif
};

#endif
