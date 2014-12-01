# Project: 68k-emu
include config.mk
MAKEFLAGS	+=	--no-print-directory

TOPDIR		=	$(shell pwd)
export TOPDIR

.PHONY: all clean install

all:
	@echo " [INIT] bin/"
	@$(MKDIR) bin/
	@echo " [ CD ] src/"
	+@make -C src/
	
	@echo "Build complete."
	@echo 

install:
	@echo " [INST] m68k-emu"
	@install -s -t $(PREFIX)/bin $(BIN)

clean:
	@echo " [ RM ] bin/"
	+@$(RM) bin/
	@echo " [ CD ] src/"
	+@make -C src/ clean
	@echo
	@echo "Source tree cleaned."
	@echo
