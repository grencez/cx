
BldPath=bld
BinPath=bin

SrcPath=src
DepPath=dep
CxPath=$(DepPath)/cx

CMAKE=cmake
GODO=$(CMAKE) -E chdir
MKDIR=$(CMAKE) -E make_directory

.PHONY: default all cmake proj test clean distclean init update pp

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

clean:
	$(GODO) $(BldPath) $(MAKE) clean

distclean:
	rm -fr $(BldPath) $(BinPath)

init:
	# okay

update:
	git pull

