# Project: 68kemu
# Makefile configurations

TARGETCC	?=	m68k-elf-gcc
BIN		=	$(TOPDIR)/bin/emu.elf
BOOTIMG		=	$(TOPDIR)/bin/boot.rom

DBGFLAGS	=	-O0 -g -D__DEBUG__
#DBGFLAGS	=	-O3 -g
#General flags

CFLAGS		+=	-Wall -I/usr/local/include/musashi
LDFLAGS		+=	-lSDL -lmusashi
PREFIX		=	/usr/local

#Extra install targets
INSTARG		=	

#Makefile tools
RM		=	rm -Rf
MKDIR		=	mkdir -p

