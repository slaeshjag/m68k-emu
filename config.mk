# Project: 68kemu
# Makefile configurations

TARGETCC	?=	m68k-elf-gcc
TARGETAS	?=	m68k-elf-as
PREFIX		?=	/usr/local
BIN		=	$(TOPDIR)/bin/m68k-emu

#DBGFLAGS	=	-O0 -g -D__DEBUG__
DBGFLAGS	=	-g -O3
#General flags

CFLAGS		+=	-Wall -I/usr/local/include/musashi $(DBGFLAGS) -fcommon
LDFLAGS		+=	-lSDL -lpthread -lm -fcommon
PREFIX		=	/usr/local

#Extra install targets
INSTARG		=	

#Makefile tools
RM		=	rm -Rf
MKDIR		=	mkdir -p

