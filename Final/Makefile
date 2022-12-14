C = gcc
CFLAGS = -c -g -Wall -Wextra
LFLAGS = -Wall -Wextra -pthread

.PHONY: all clean

all: matrixMul2

matrixMul: matrixMul2.o
	$(CC) $(LFLAGS) $^ -o $@

clean:
	rm -f matrixMul2
	rm -f *.o
	rm -f *~
	rm -f results.txt
