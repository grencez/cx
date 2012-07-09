
CC = gcc

CFLAGS =
CFLAGS += -ansi -pedantic
CFLAGS += -Wall -Wextra -Wstrict-aliasing
CFLAGS += -g
#CFLAGS += -O3

CExes = cx verify

all: $(CExes)

verify: verif.c bstree.o fileb.o rbtree.o sys-cx.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

cx: cx.c bstree.o fileb.o rbtree.o sys-cx.o
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f *.o $(CExes)

