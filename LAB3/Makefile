CC = gcc
CFLAGS = -Wall -pthread
TARGETS = parent child

all: $(TARGETS)

parent: parent.c common.h
	$(CC) $(CFLAGS) -o parent parent.c

child: child.c common.h
	$(CC) $(CFLAGS) -o child child.c

clean:
	rm -f $(TARGETS)

run: all
	./parent

test: all
	echo -e "out1.txt\nout2.txt\nHello World\nTesting" | ./parent
	cat out1.txt
	cat out2.txt

.PHONY: all clean run test
