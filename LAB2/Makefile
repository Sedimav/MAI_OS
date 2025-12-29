CC = clang
CFLAGS = -Wall -Wextra -O2 -D_POSIX_C_SOURCE=200809L
TARGET = quicksort

all: $(TARGET)

quicksort: quicksort.c
	$(CC) $(CFLAGS) -o $@ $< -pthread

clean:
	rm -f $(TARGET)

.PHONY: all clean
