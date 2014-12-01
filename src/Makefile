# Project: m68k-emu
include $(TOPDIR)/config.mk


SRCFILES	=	$(wildcard *.c)
OBJFILES	=	$(SRCFILES:.c=.o)
.PHONY: all clean

all: $(OBJFILES) $(DEPENDS)
	@echo " [MAKE] cpu/"
	@make -C cpu/
	
	@echo " [ LD ] $(BIN)"
	@$(CC) $(CFLAGS) $(OBJFILES) -o $(BIN) cpu/cpu.a $(LDFLAGS)
	@echo "Done."
	@echo
	
clean:
	@echo
	@echo " [CLEAN] cpu/"
	@make -C cpu/ clean
	@echo " [ RM ] $(OBJFILES)"
	@$(RM) $(OBJFILES)
	@echo "Done."
	@echo 

%.o: %.c
	@echo " [ CC ] src/$<"
	@$(CC) $(CFLAGS) $(DBGFLAGS) -c -o $@ $<
	
