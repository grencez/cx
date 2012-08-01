
CC = gcc

CFLAGS =
CFLAGS += -ansi -pedantic
CFLAGS += -Wall -Wextra -Wstrict-aliasing
CFLAGS += -g
#CFLAGS += -O3

CExes = cx verify

CFiles = bstree.c fileb.c rbtree.c sys-cx.c cx.c verif.c
HFiles = associa.h bittable.h bstree.h cons.h def.h fileb.h rbtree.h synhax.h sys-cx.h table.h

CxDeps = bstree fileb rbtree sys-cx
CxObjs = $(addsuffix .o,$(CxDeps))

CxPpPath = ../cx-pp
BldPath = ../cx-bld

all: $(CExes)

verify: $(addprefix $(BldPath)/,verif.o $(CxObjs))
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

cx: $(addprefix $(BldPath)/,cx.o $(CxObjs))
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

$(CxPpPath)/cx: $(addprefix $(CxPpPath)/,cx.o $(CxObjs))
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

$(CxPpPath)/%.o: $(CxPpPath)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BldPath)/%.c: %.c $(CxPpPath)/cx
	$(CxPpPath)/cx -x $< -o $@

$(BldPath)/%.o: $(BldPath)/%.c
	$(CC) -c $(CFLAGS) -I. $< -o $@

.PHONY: pp
pp:
	$(foreach f,$(addsuffix .c,cx $(CxDeps)), \
		./cx -x $(f) -o $(CxPpPath)/$(f) ;)
	cp *.h $(CxPpPath)

$(addprefix $(BldPath)/,$(CFiles)): | $(BldPath)

$(BldPath):
	mkdir -p $(BldPath)

.PHONY: clean
clean:
	rm -f $(CxPpPath)/*.o $(CExes)
	rm -fr $(BldPath)

