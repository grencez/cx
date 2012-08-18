
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

