TARGET = mytar
CC = gcc
CFLAGS = -g -Wall 
OBJS = mytar.o mytar_routines.o
SOURCES = $(addsuffix .c, $(basename $(OBJS)))
HEADERS = mytar.h

all: $(TARGET)

.PHONY : clean

clean : clear
	rm -f core $(TARGET) *.log *.mtar

clear :
	rm -f *.o

$(TARGET): $(OBJS) 
	@echo "# Generando $@ "
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)
	@rm -f *.o
	@echo "#------------------\n"

#.c.o: 
#	$(CC) $(CFLAGS) -c  $< -o $@

$(OBJS): $(HEADERS)

valgrind-check-create: $(TARGET) clear
	@echo "Ejecutando $@"
	@valgrind --leak-check=full ./$(TARGET) -c -f test.mtar a.txt b.txt c.txt

valgrind-check-extract: $(TARGET) clear
	@echo "Ejecutando $@"
	@valgrind --leak-check=full ../$(TARGET) -x -f test.mtar

debug-create : $(TARGET)
	@echo "# Debugging $@ "
	gdb -tui ./$(TARGET) -s $< 
	@echo "#------------------\n"

debug-extract : $(TARGET)
	@echo "# Debugging $@ "
	gdb -tui ./$(TARGET) -s $< 
	@echo "#------------------\n"