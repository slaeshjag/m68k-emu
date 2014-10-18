# Project: 68k-emu
include config.mk
MAKEFLAGS	+=	--no-print-directory

TOPDIR		=	$(shell pwd)
export TOPDIR

.PHONY: all clean

all:
	@echo " [INIT] bin/"
	@$(MKDIR) bin/
	@echo " [ CD ] src/"
	+@make -C src/
	@echo " [ CD ] bootimg/"
	+@make -C bootimg/
	@echo " [ CD ] osloader/"
	+@make -C osloader/
	@cat $(OSFS) >> $(BOOTIMG)
	
	@echo "Build complete."
	@echo 

clean:
	@echo " [ RM ] bin/"
	+@$(RM) bin/
	@echo " [ CD ] src/"
	+@make -C src/ clean
	@echo " [ CD ] bootimg/"
	+@make -C bootimg/ clean
	@echo " [ CD ] osloader/"
	+@make -C osloader/ clean
	@echo
	@echo "Source tree cleaned."
	@echo
