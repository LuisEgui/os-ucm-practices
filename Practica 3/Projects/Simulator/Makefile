CFLAGS  = -Wall -g -pthread 
SRC = simulator.c
PROG   = simulator

all :
	gcc $(CFLAGS) -o $(PROG) $(SRC)

clean :
	-rm -f $(PROG)

debug : $(PROG)
	@echo "# Debugging $@ "
	gdb -tui ./$(PROG) -s $<
