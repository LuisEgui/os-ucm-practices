CC = gcc
CFLAGS = -Wall -g

PROG = badsort-ptr
OBJECTS = $(PROG).o

all : $(PROG) 

.PHONY : clean

clean : clear
	rm -f core $(PROG) *.log

clear :
	rm -f *.o

$(PROG) : $(OBJECTS)
	@echo "# Generando $@ "
	$(CC) $(CFLAGS) -o $@ $@.o
	@rm -f *.o
	@echo "#------------------\n"
	
%.o : %.c 
	@echo "# Generando $@ "
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "#------------------\n"

valgrind-check: $(PROG) clear
	@echo "Ejecutando $@"
	@valgrind --leak-check=full ./$(PROG)

debug : $(PROG)
	@echo "# Debugging $@ "
	gdb -tui ./$(PROG) -s $< 
	@echo "#------------------\n"
