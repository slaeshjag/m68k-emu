# Project: 68kemu
# Makefile configurations

TARGETCC	?=	m68k-elf-gcc
TARGETAS	?=	m68k-elf-as
BIN		=	$(TOPDIR)/bin/emu.elf
BOOTIMG		=	$(TOPDIR)/bin/boot.rom
OSFS		=	$(TOPDIR)/bin/os.rom

DBGFLAGS	=	-O0 -g -D__DEBUG__
#DBGFLAGS	=	-O3 -g
#General flags

CFLAGS		+=	-Wall -I/usr/local/include/musashi
LDFLAGS		+=	-lSDL -lpthread -lm
PREFIX		=	/usr/local

#Extra install targets
INSTARG		=	

#Makefile tools
RM		=	rm -Rf
MKDIR		=	mkdir -p

