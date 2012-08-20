
$(CxBldPath)/alphatab.h: $(CxBldPath)/table.h
$(CxBldPath)/associa.h: $(CxBldPath)/lgtable.h
$(CxBldPath)/associa.h: $(CxBldPath)/rbtree.h
$(CxBldPath)/bittable.h: $(CxBldPath)/table.h
$(CxBldPath)/bstree.c: $(CxBldPath)/bstree.h
$(CxBldPath)/bstree.h: $(CxBldPath)/def.h
$(CxBldPath)/cx.c: $(CxBldPath)/syscx.h
$(CxBldPath)/cx.c: $(CxBldPath)/associa.h
$(CxBldPath)/cx.c: $(CxBldPath)/fileb.h
$(CxBldPath)/cx.c: $(CxBldPath)/sxpn.h
$(CxBldPath)/cx.c: $(CxBldPath)/table.h
$(CxBldPath)/def.h: $(CxBldPath)/synhax.h
$(CxBldPath)/fileb.c: $(CxBldPath)/fileb.h
$(CxBldPath)/fileb.h: $(CxBldPath)/alphatab.h
$(CxBldPath)/lgtable.h: $(CxBldPath)/bittable.h
$(CxBldPath)/ospc.c: $(CxBldPath)/ospc.h
$(CxBldPath)/ospc.h: $(CxBldPath)/syscx.h
$(CxBldPath)/ospc.h: $(CxBldPath)/fileb.h
$(CxBldPath)/rbtree.c: $(CxBldPath)/rbtree.h
$(CxBldPath)/rbtree.h: $(CxBldPath)/bstree.h
$(CxBldPath)/sxpn.c: $(CxBldPath)/sxpn.h
$(CxBldPath)/sxpn.h: $(CxBldPath)/lgtable.h
$(CxBldPath)/sxpn.h: $(CxBldPath)/fileb.h
$(CxBldPath)/syscx.c: $(CxBldPath)/syscx.h
$(CxBldPath)/syscx.c: $(CxBldPath)/fileb.h
$(CxBldPath)/table.h: $(CxBldPath)/def.h

