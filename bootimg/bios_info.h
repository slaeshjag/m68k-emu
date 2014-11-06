#ifndef __BIOS_INFO_H__
#define	__BIOS_INFO_H__

#define	BIOS_INFO_ADDR		((void *) 0x7FF00)


struct BiosInfo {
	unsigned char		*font;
	int			term_x;
	int			term_y;
	int			def_fg;
	int			def_bg;
};


#endif
