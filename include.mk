
# Required:
#  CxPath
#  PfxBldPath
#  BldPath
#  BinPath
#  ExeList
# Suggested:
#  CONFIG

# Defines:
#  CxExe
#  CxBldPath
#  CxHFiles

.SUFFIXES:
SUFFIXES :=

PfxBldPath ?= bld
BldPathCXX := $(PfxBldPath)/$(BldPath)-cxx
BldPath := $(PfxBldPath)/$(BldPath)

CxExe ?= $(BinPath)/cx

CxBldPath ?= $(PfxBldPath)/cx

CxDeps = alphatab bstree fileb lsp ospc rbtree sxpn syscx

CxHFiles = \
	$(CxDeps) \
	associa \
	bittable \
	def \
	lgtable \
	synhax \
	table

CxHHFiles = synhax set table


CxHFileDeps := $(addsuffix .h,$(CxHFiles))
CxCFileDeps := $(addsuffix .c,$(CxDeps))
CxHHFileDeps := $(addsuffix .hh,$(CxHHFiles))

CxHFiles := $(addprefix $(CxBldPath)/,$(CxHFileDeps))
CxHHFiles := $(addprefix $(CxBldPath)/,$(CxHHFileDeps))

CxObjs = $(addprefix $(CxBldPath)/,$(addsuffix .o,$(CxDeps)))

## Serious debugging is about to happen.
ifneq (,$(filter ultradebug,$(CONFIG)))
	CONFIG := $(filter-out snappy fast debug,$(CONFIG))
	CFLAGS += -g3
endif
## Add debugging symbols.
ifneq (,$(filter debug,$(CONFIG)))
	CFLAGS += -g
endif
ifneq (,$(filter ultradebug,$(CONFIG)))
	CFLAGS += -g3
endif
## Go really fast.
ifneq (,$(filter fast,$(CONFIG)))
	CFLAGS += -O3
endif
## Go pretty fast.
ifneq (,$(filter snappy,$(CONFIG)))
	CFLAGS += -O2
endif
## Enable profiling.
ifneq (,$(filter profile,$(CONFIG)))
	CFLAGS += -pg
	LFLAGS += -pg
endif
## Disable assertions.
ifneq (,$(filter noassert,$(CONFIG)))
	CFLAGS += -DNDEBUG
endif
## Stick to the ANSI standard.
ifneq (,$(filter ansi,$(CONFIG)))
	CFLAGS += -ansi -pedantic
endif
ifneq (,$(filter errwarn,$(CONFIG)))
	CFLAGS += -Werror
endif

CFLAGS += -Wall -Wextra -Wstrict-aliasing
IFLAGS += -I$(PfxBldPath)
IFLAGS += -I$(BldPath)
CFLAGS += $(IFLAGS)
ExecCx = $(CxExe)

ifdef CxPpPath
	CxPpExe = $(CxPpPath)/cx
	ExecCxPp = $(CxPpExe)
endif

ifneq (,$(filter wine,$(CONFIG)))
	CC = wine "$(HOME)/.wine/drive_c/Program Files (x86)/CodeBlocks/MinGW/bin/mingw32-gcc-4.4.1.exe"
	#CC = wine "$(HOME)/.wine/drive_c/Program Files/CodeBlocks/MinGW/bin/mingw32-gcc-4.4.1.exe"
	CFLAGS := $(filter-out -ansi -Werror,$(CFLAGS))
define exename
$(addsuffix .exe,$(1))
endef
define execname
$(addprefix wine ,$(call exename,$(1)))
endef
endif

ifneq (,$(filter winegcc,$(CONFIG)))
	CC = winegcc -mno-cygwin
	CFLAGS := $(filter-out -ansi -Werror,$(CFLAGS))
define exename
$(addsuffix .exe,$(1))
endef
define execname
$(addprefix ./,$(addsuffix .exe,$(1)))
endef
endif

ifndef exename
define exename
$(1)
endef
endif

ifndef execname
define execname
$(addprefix ./,$(1))
endef
endif

define binexe
$(addprefix $(BinPath)/,$(call exename,$(1)))
endef


ifdef CxPpPath
	ExecCxPp := $(call execname,$(CxPpExe))
	CxPpExe := $(call exename,$(CxPpExe))
endif
ExecCx := $(call execname,$(CxExe))
CxExe := $(call exename,$(CxExe))
ExeList := $(call exename,$(ExeList))

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

.PRECIOUS: $(CxBldPath)/%.hh
$(CxBldPath)/%.hh: $(CxPath)/%.hh
	cp -f $< $@

$(eval $(shell \
	sed \
	-e 's/\(.*\): *\(.*\)/$$(eval $$(CxBldPath)\/\1: $$(CxBldPath)\/\2)/' \
	$(CxPath)/deps.mk))

$(BldPath)/%.c: %.c $(CxExe) $(CxHFiles)
	$(ExecCx) -x $< -o $@

$(BldPath)/%.h: %.h $(CxExe) $(CxHFiles)
	$(ExecCx) -x $< -o $@

$(BldPathCXX)/%.cc: %.cc $(CxExe)
	cp -f $< $@

$(BldPathCXX)/%.hh: %.hh $(CxExe)
	cp -f $< $@

$(CxBldPath)/%.o: $(CxBldPath)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BldPath)/%.o: $(BldPath)/%.c $(CxHFiles)
	$(CC) -c $(CFLAGS) $< -o $@

$(BldPathCXX)/%.o: $(BldPathCXX)/%.cc $(CxHFiles) $(CxHHFiles)
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(patsubst %.o,%.c,$(CxObjs)): | $(CxBldPath)
$(CxHFiles): | $(CxBldPath) $(BldPath) $(BldPathCXX)
$(ExeList): | $(BinPath)


$(CxBldPath):
	mkdir -p $@
$(BinPath):
	mkdir -p $@
$(BldPath):
	mkdir -p $@
$(BldPathCXX):
	mkdir -p $@

.PHONY: killcmake
killcmake:
	rm -fr CMakeFiles
	rm -f cmake_install.cmake CMakeCache.txt Makefile
	ln -s Makefile.raw Makefile

