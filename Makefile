
BldPath=bld
BinPath=bin

SrcPath=src
DepPath=dep
CxPath=$(DepPath)/cx

ScanBldPath=clang
ScanRptPath=$(ScanBldPath)/report
SCAN_BUILD=scan-build -o $(PWD)/$(ScanRptPath)

CMakeExe=cmake
CMAKE=$(CMakeExe)
GODO=$(CMakeExe) -E chdir
MKDIR=$(CMakeExe) -E make_directory
CTAGS=ctags

.PHONY: default all cmake proj \
	pp \
	test analyze tags \
	clean distclean \
	init update pull

default:
	$(MAKE) init
	if [ ! -d $(BldPath) ] ; then $(MAKE) cmake ; fi
	$(MAKE) proj

all:
	$(MAKE) init
	$(MAKE) cmake
	$(MAKE) proj

cmake:
	if [ ! -d $(BldPath) ] ; then $(MKDIR) $(BldPath) ; fi
	$(GODO) $(BldPath) $(CMAKE) ../src

proj:
	$(GODO) $(BldPath) $(MAKE)

pp:
	$(GODO) $(BldPath) $(MAKE) pp

test:
	$(GODO) $(BldPath) $(MAKE) test

analyze:
	rm -fr $(ScanRptPath)
	$(MAKE) 'BldPath=$(ScanBldPath)' 'CMAKE=$(SCAN_BUILD) cmake' 'MAKE=$(SCAN_BUILD) make'

tags:
	$(CTAGS) -R src -R dep/cx/src

clean:
	$(GODO) $(BldPath) $(MAKE) clean

distclean:
	rm -fr $(BldPath) $(BinPath) $(ScanBldPath) tags

init:
	# okay

update:
	git pull origin master

pull:
	git pull origin master

