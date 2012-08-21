
# Required:
#  CxPath
#  PfxBldPath
#  BldPath
#  BinPath
#  Objs
#  ExeList
# Suggested:
#  CONFIG

# Defines:
#  CxExe
#  CxBldPath

CxExe ?= $(BinPath)/cx

CxBldPath ?= $(PfxBldPath)/cx

CxDeps = bstree fileb ospc rbtree sxpn syscx

CxHFiles = \
	$(CxDeps) \
	associa \
	alphatab \
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
ifneq (,$(filter ultradebug,$(CONFIG)))
	CFLAGS += -g3
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
CFLAGS += -I$(PfxBldPath)
ExecCx = $(CxExe)

ifdef CxPpPath
	CxPpExe = $(CxPpPath)/cx
	ExecCxPp = $(CxPpExe)
endif

ifneq (,$(filter wine,$(CONFIG)))
	CC = wine "$(HOME)/.wine/drive_c/Program Files (x86)/CodeBlocks/MinGW/bin/mingw32-gcc-4.4.1.exe"
	#CC = wine "$(HOME)/.wine/drive_c/Program Files/CodeBlocks/MinGW/bin/mingw32-gcc-4.4.1.exe"
	CFLAGS := $(filter-out -ansi,$(CFLAGS))
	ifdef CxPpPath
		CxPpExe := $(CxPpExe).exe
		ExecCxPp = wine $(CxPpExe)
	endif
	CxExe := $(CxExe).exe
	ExecCx = wine $(CxExe)
	TestExe := $(TestExe).exe
	ExecTest = wine $(TestExe)
endif
ifneq (,$(filter winegcc,$(CONFIG)))
	CC = winegcc -mno-cygwin
	CFLAGS := $(filter-out -ansi,$(CFLAGS))
	ifdef CxPpPath
		CxPpExe := $(CxPpExe).exe
		ExecCxPp = wine $(CxPpExe)
	endif
	CxExe := $(CxExe).exe
	ExecCx = $(CxExe)
	TestExe := $(TestExe).exe
	ExecTest := $(ExecTest)
endif

ifdef CxPpPath

$(CxPpPath)/%.o: $(CxPpPath)/%.c
	$(CC) -c $(CFLAGS) -I$(CxPpPath) $< -o $@
	
$(CxPpExe): $(CxPpPath)/cx.o \
   	$(patsubst $(CxBldPath)/%,$(CxPpPath)/%,$(CxObjs))
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

$(CxBldPath)/%.c: $(CxPath)/%.c $(CxPpExe)
	$(ExecCxPp) -x $< -o $@

$(CxBldPath)/%.h: $(CxPath)/%.h $(CxPpExe)
	$(ExecCxPp) -x $< -o $@

$(CxBldPath)/cx.c: | $(CxBldPath)

$(CxExe): $(CxBldPath)/cx.o $(CxObjs)
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)
else

$(CxBldPath)/%.c: $(CxPath)/%.c $(CxExe)
	$(ExecCx) -x $< -o $@

$(CxBldPath)/%.h: $(CxPath)/%.h $(CxExe)
	$(ExecCx) -x $< -o $@
endif

$(eval $(shell \
	sed \
	-e 's/\(.*\): *\(.*\)/$$(eval $$(CxBldPath)\/\1: $$(CxBldPath)\/\2)/' \
	$(CxPath)/deps.mk))

$(BldPath)/%.c: %.c $(CxExe) $(addprefix $(CxBldPath)/,$(CxHFiles))
	$(ExecCx) -x $< -o $@

$(BldPath)/%.h: %.h $(CxExe) $(addprefix $(CxBldPath)/,$(CxHFiles))
	$(ExecCx) -x $< -o $@

$(CxBldPath)/%.o: $(CxBldPath)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BldPath)/%.o: $(BldPath)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(patsubst %.o,%.c,$(CxObjs)): | $(CxBldPath)
$(addprefix $(CxBldPath)/,$(CxHFiles)): | $(CxBldPath)
$(ExeList): | $(BinPath)
$(patsubst %.o,%.c,$(Objs)): | $(BldPath)


$(CxBldPath):
	mkdir -p $@
$(BinPath):
	mkdir -p $@
$(BldPath):
	mkdir -p $@

.PHONY: killcmake
killcmake:
	rm -fr CMakeFiles
	rm -f cmake_install.cmake CMakeCache.txt Makefile
	ln -s Makefile.raw Makefile

