
CC = gcc

CFLAGS =
CFLAGS += -ansi -pedantic
CFLAGS += -Wall -Wextra
CFLAGS += -g

all: cx verify

verify: verif.c bstree.o fileb.o rbtree.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

cx: cx.c bstree.o fileb.o rbtree.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f *.o cx verify

