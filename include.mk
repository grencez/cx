
# Required:
#  CxPath
#  PfxBldPath
#  BldPath
#  BinPath
#  Objs
#  ExeList
# Suggested:
#  CONFIG

CxBldPath = $(PfxBldPath)/cx

CxDeps = bstree fileb ospc rbtree sxpn syscx

CxHFiles = \
	$(CxDeps) \
	associa \
	bittable \
	def \
	lgtable \
	synhax \
	table

CxHFiles := $(addsuffix .h,$(CxHFiles))

CxObjs = $(addprefix $(CxBldPath)/,$(addsuffix .o,$(CxDeps)))

ifneq (,$(filter debug,$(CONFIG)))
	CFLAGS += -g
endif
ifneq (,$(filter fast,$(CONFIG)))
	CFLAGS += -O3
endif
ifneq (,$(filter noassert,$(CONFIG)))
	CFLAGS += -DNDEBUG
endif
ifneq (,$(filter ansi,$(CONFIG)))
	CFLAGS += -ansi -pedantic
endif

CFLAGS += -Wall -Wextra -Wstrict-aliasing


$(CxBldPath)/%.o: $(CxPath)/%.c
	$(CC) -c $(CFLAGS) -I. $< -o $@

$(BldPath)/%.o: %.c
	$(CC) -c $(CFLAGS) -I. $< -o $@


$(CxObjs): | $(CxBldPath)
$(ExeList): | $(BinPath)
$(Objs): | $(BldPath)


$(CxBldPath):
	mkdir -p $@
$(BinPath):
	mkdir -p $@
$(BldPath):
	mkdir -p $@

