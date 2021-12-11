CFLAGS  = -Wall -g -pthread
SRC1 = partial_sum1.c
SRC2 = partial_sum2.c
SRC3 = partial_sum3.c
PROG1   = partial_sum1
PROG2   = partial_sum2
PROG3   = partial_sum3

all :
	gcc $(CFLAGS) -o $(PROG1) $(SRC1)
	gcc $(CFLAGS) -o $(PROG2) $(SRC2)
	gcc $(CFLAGS) -o $(PROG3) $(SRC3)

clean :
	-rm -f $(PROG1) $(PROG2) $(PROG3)
