.SUFFIXES: .cpp .c .o .so .a .exe .obj .output

# bug.cpp fstream1.cpp don't work for some reason

# bvec1.cpp seek.cpp and sstream1.cpp only work with the STLport
# iostreams library

SRCS = accum1.cpp accum2.cpp adjdiff0.cpp adjdiff1.cpp adjdiff2.cpp \
	adjfind0.cpp adjfind1.cpp adjfind2.cpp advance.cpp alg1.cpp \
	alg2.cpp alg3.cpp alg4.cpp alg5.cpp bcompos1.cpp bcompos2.cpp \
	bind1st1.cpp bind1st2.cpp bind2nd1.cpp bind2nd2.cpp \
	binsert1.cpp binsert2.cpp binsrch1.cpp binsrch2.cpp \
	bitset1.cpp bnegate1.cpp bnegate2.cpp \
	copy1.cpp copy2.cpp copy3.cpp copy4.cpp copyb.cpp copyb0.cpp \
	count0.cpp count1.cpp countif1.cpp deque1.cpp divides.cpp \
	eqlrnge0.cpp eqlrnge1.cpp eqlrnge2.cpp equal0.cpp equal1.cpp \
	equal2.cpp equalto.cpp fill1.cpp filln1.cpp find0.cpp \
	find1.cpp findif0.cpp findif1.cpp finsert1.cpp finsert2.cpp \
	foreach0.cpp foreach1.cpp func1.cpp func2.cpp \
	func3.cpp gener1.cpp gener2.cpp genern1.cpp genern2.cpp \
	greateq.cpp greater.cpp hmap1.cpp hmmap1.cpp hmset1.cpp \
	hset2.cpp incl0.cpp incl1.cpp incl2.cpp inplmrg1.cpp \
	inplmrg2.cpp inrprod0.cpp inrprod1.cpp inrprod2.cpp \
	insert1.cpp insert2.cpp iota1.cpp istmit1.cpp iter1.cpp \
	iter2.cpp iter3.cpp iter4.cpp iterswp0.cpp iterswp1.cpp \
	less.cpp lesseq.cpp lexcmp1.cpp lexcmp2.cpp list1.cpp \
	list2.cpp list3.cpp list4.cpp logicand.cpp logicnot.cpp \
	logicor.cpp lwrbnd1.cpp lwrbnd2.cpp map1.cpp max1.cpp max2.cpp \
	maxelem1.cpp maxelem2.cpp memfunptr.cpp merge0.cpp merge1.cpp merge2.cpp \
	min1.cpp min2.cpp minelem1.cpp minelem2.cpp minus.cpp \
	mismtch0.cpp mismtch1.cpp mismtch2.cpp mkheap0.cpp mkheap1.cpp \
	mmap1.cpp mmap2.cpp modulus.cpp mset1.cpp mset3.cpp mset4.cpp \
	mset5.cpp negate.cpp nequal.cpp nextprm0.cpp nextprm1.cpp \
	nextprm2.cpp nthelem0.cpp nthelem1.cpp nthelem2.cpp ostmit.cpp \
	pair0.cpp pair1.cpp pair2.cpp parsrt0.cpp parsrt1.cpp \
	parsrt2.cpp parsrtc0.cpp parsrtc1.cpp parsrtc2.cpp \
	partsrt0.cpp partsum0.cpp partsum1.cpp partsum2.cpp pheap1.cpp \
	pheap2.cpp plus.cpp pqueue1.cpp prevprm0.cpp prevprm1.cpp \
	prevprm2.cpp ptition0.cpp ptition1.cpp ptrbinf1.cpp \
	ptrbinf2.cpp ptrunf1.cpp ptrunf2.cpp queue1.cpp rawiter.cpp \
	remcopy1.cpp remcpif1.cpp remif1.cpp remove1.cpp repcpif1.cpp \
	replace0.cpp replace1.cpp replcpy1.cpp replif1.cpp revbit1.cpp \
	revbit2.cpp revcopy1.cpp reverse1.cpp reviter1.cpp \
	reviter2.cpp rndshuf0.cpp rndshuf1.cpp rndshuf2.cpp \
	rotate0.cpp rotate1.cpp rotcopy0.cpp rotcopy1.cpp search0.cpp \
	search1.cpp search2.cpp set1.cpp set2.cpp \
	setdiff0.cpp setdiff1.cpp setdiff2.cpp setintr0.cpp \
	setintr1.cpp setintr2.cpp setsymd0.cpp setsymd1.cpp \
	setsymd2.cpp setunon0.cpp setunon1.cpp setunon2.cpp \
	slist1.cpp sort1.cpp sort2.cpp stack1.cpp \
	stack2.cpp stat.cpp stblptn0.cpp stblptn1.cpp stblsrt1.cpp \
	stblsrt2.cpp strass1.cpp \
	string1.cpp swap1.cpp swprnge1.cpp times.cpp trnsfrm1.cpp \
	trnsfrm2.cpp tstdeq.cpp ucompos1.cpp ucompos2.cpp unegate1.cpp \
	unegate2.cpp uniqcpy1.cpp uniqcpy2.cpp unique1.cpp unique2.cpp \
	uprbnd1.cpp uprbnd2.cpp vec1.cpp vec2.cpp vec3.cpp vec4.cpp \
	vec5.cpp vec6.cpp vec7.cpp vec8.cpp


.cpp.exe:
	dmc -Ae -Ar -DMAIN -I../../stlport $<

all:	$(SRCS:.cpp=.exe)
#	-*- Makefile -*-
.SUFFIXES: .cpp .c .obj .dll .exe .rc .res


CXX=dmc
CC=dmc -cpp
LIB=lib
LINK=dmc
RC=rcc

LIB_BASENAME=stlp45dm

OBJS = build\c_locale.obj build\c_locale_stub.obj build\codecvt.obj \
	build\collate.obj build\complex.obj build\complex_exp.obj \
	build\complex_io.obj build\complex_io_w.obj build\complex_trig.obj \
	build\ctype.obj build\dll_main.obj \
	build\facets_byname.obj build\fstream.obj build\ios.obj \
	build\iostream.obj build\istream.obj build\locale.obj \
	build\locale_catalog.obj build\locale_impl.obj \
	build\messages.obj build\monetary.obj build\num_get.obj \
	build\num_get_float.obj build\num_put.obj \
	build\num_put_float.obj build\numpunct.obj build\ostream.obj \
	build\sstream.obj build\stdio_streambuf.obj \
	build\streambuf.obj build\string_w.obj build\strstream.obj \
	build\time_facets.obj

BUILD_DIRS=..\lib ..\build \
	..\build\static ..\build\static\release \
	..\build\static\debug ..\build\static\stldebug \
	..\build\staticx ..\build\staticx\release \
	..\build\staticx\debug ..\build\staticx\stldebug \
	..\build\dynamic ..\build\dynamic\release \
	..\build\dynamic\debug ..\build\dynamic\stldebug \
	..\build\sdynamic ..\build\sdynamic\release \
	..\build\sdynamic\debug ..\build\sdynamic\stldebug


CXXFLAGS_COMMON = -Ae -Ar -DSTRICT -D__BUILDING_STLPORT -I../stlport

# four versions are currently supported:
#  - static: static STLport library, static RTL
#  - staticx: static STLport library, dynamic RTL
#  - dynamic: dynamic STLport library, dynamic RTL
#  - sdynamic: dynamic STLport library, static RTL

CXXFLAGS_RELEASE_static = $(CXXFLAGS_COMMON) -Nc -o+all -D_MT
CXXFLAGS_RELEASE_staticx = $(CXXFLAGS_COMMON) -Nc -o+all -ND
CXXFLAGS_RELEASE_dynamic = $(CXXFLAGS_COMMON) -WD -o+all -ND
CXXFLAGS_RELEASE_sdynamic = $(CXXFLAGS_COMMON) -WD -o+all -D_MT

CXXFLAGS_DEBUG_static = $(CXXFLAGS_COMMON) -Nc -g -D_MT
CXXFLAGS_DEBUG_staticx = $(CXXFLAGS_COMMON) -Nc -g -ND
CXXFLAGS_DEBUG_dynamic = $(CXXFLAGS_COMMON) -WD -g -ND
CXXFLAGS_DEBUG_sdynamic = $(CXXFLAGS_COMMON) -WD -g -D_MT

CXXFLAGS_STLDEBUG_static = $(CXXFLAGS_DEBUG_static) -D_STLP_DEBUG
CXXFLAGS_STLDEBUG_staticx = $(CXXFLAGS_DEBUG_staticx) -D_STLP_DEBUG
CXXFLAGS_STLDEBUG_dynamic = $(CXXFLAGS_DEBUG_dynamic) -D_STLP_DEBUG
CXXFLAGS_STLDEBUG_sdynamic = $(CXXFLAGS_DEBUG_sdynamic) -D_STLP_DEBUG


.cpp{..\build\static\release}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_RELEASE_static) "$<"

.cpp{..\build\static\debug}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_DEBUG_static) "$<"

.cpp{..\build\static\stldebug}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_STLDEBUG_static) "$<"

.cpp{..\build\staticx\release}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_RELEASE_staticx) "$<"

.cpp{..\build\staticx\debug}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_DEBUG_staticx) "$<"

.cpp{..\build\staticx\stldebug}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_STLDEBUG_staticx) "$<"

.cpp{..\build\dynamic\release}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_RELEASE_dynamic) "$<"

.cpp{..\build\dynamic\debug}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_DEBUG_dynamic) "$<"

.cpp{..\build\dynamic\stldebug}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_STLDEBUG_dynamic) "$<"

.cpp{..\build\sdynamic\release}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_RELEASE_sdynamic) "$<"

.cpp{..\build\sdynamic\debug}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_DEBUG_sdynamic) "$<"

.cpp{..\build\sdynamic\stldebug}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_STLDEBUG_sdynamic) "$<"

.c{..\build\static\release}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_RELEASE_static) "$<"

.c{..\build\static\debug}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_DEBUG_static) "$<"

.c{..\build\static\stldebug}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_STLDEBUG_static) "$<"

.c{..\build\staticx\release}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_RELEASE_staticx) "$<"

.c{..\build\staticx\debug}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_DEBUG_staticx) "$<"

.c{..\build\staticx\stldebug}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_STLDEBUG_staticx) "$<"

.c{..\build\dynamic\release}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_RELEASE_dynamic) "$<"

.c{..\build\dynamic\debug}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_DEBUG_dynamic) "$<"

.c{..\build\dynamic\stldebug}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_STLDEBUG_dynamic) "$<"

.c{..\build\sdynamic\release}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_RELEASE_sdynamic) "$<"

.c{..\build\sdynamic\debug}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_DEBUG_sdynamic) "$<"

.c{..\build\sdynamic\stldebug}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_STLDEBUG_sdynamic) "$<"


.rc{..\build}.res:
	$(RC) -32 -o"$@" "$<"


all:	directories all_static all_staticx all_dynamic all_sdynamic


directories:	$(BUILD_DIRS)

$(BUILD_DIRS):
	mkdir $@


all_static:	..\lib\$(LIB_BASENAME)_static.lib ..\lib\$(LIB_BASENAME)_debug_static.lib ..\lib\$(LIB_BASENAME)_stldebug_static.lib

..\lib\$(LIB_BASENAME)_static.lib:	$(OBJS:build\=..\build\static\release\)
	*$(LIB) -c -n -p64 "$@" "$**"

..\lib\$(LIB_BASENAME)_debug_static.lib:	$(OBJS:build\=..\build\static\debug\)
	*$(LIB) -c -n -p128 "$@" "$**"

..\lib\$(LIB_BASENAME)_stldebug_static.lib:	$(OBJS:build\=..\build\static\stldebug\)
	*$(LIB) -c -n -p256 "$@" "$**"


all_staticx:	..\lib\$(LIB_BASENAME)_staticx.lib ..\lib\$(LIB_BASENAME)_debug_staticx.lib ..\lib\$(LIB_BASENAME)_stldebug_staticx.lib

..\lib\$(LIB_BASENAME)_staticx.lib:	$(OBJS:build\=..\build\staticx\release\)
	*$(LIB) -c -n -p64 "$@" "$**"

..\lib\$(LIB_BASENAME)_debug_staticx.lib:	$(OBJS:build\=..\build\staticx\debug\)
	*$(LIB) -c -n -p128 "$@" "$**"

..\lib\$(LIB_BASENAME)_stldebug_staticx.lib:	$(OBJS:build\=..\build\staticx\stldebug\)
	*$(LIB) -c -n -p256 "$@" "$**"


all_dynamic:	..\lib\$(LIB_BASENAME).dll ..\lib\$(LIB_BASENAME)_debug.dll ..\lib\$(LIB_BASENAME)_stldebug.dll

..\lib\$(LIB_BASENAME).dll:	$(OBJS:build\=..\build\dynamic\release\) ..\build\stlport.res
	*$(LINK) -WD -ND -L/IMPLIB:$(@R).lib -L/DE -L/NODEBUG -o"$@" "$**" user32.lib kernel32.lib <<$(@R).def
LIBRARY "$(@F)"
DESCRIPTION 'STLport DLL for Digital Mars C/C++'
EXETYPE NT
SUBSYSTEM WINDOWS
CODE SHARED EXECUTE
DATA READWRITE

EXPORTS
	"?cin@std@@3V?$basic_istream@std@DV?$char_traits@std@D@1@@1@A"
	"?cout@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?cerr@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?clog@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?wcin@std@@3V?$basic_istream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcout@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcerr@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wclog@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
<<

..\lib\$(LIB_BASENAME)_debug.dll:	$(OBJS:build\=..\build\dynamic\debug\) ..\build\stlport.res
	*$(LINK) -WD -ND -g -L/IMPLIB:$(@R).lib -L/DE -L/DEBUG -o"$@" "$**" user32.lib kernel32.lib <<$(@R).def
LIBRARY "$(@F)"
DESCRIPTION 'STLport DLL for Digital Mars C/C++'
EXETYPE NT
SUBSYSTEM WINDOWS
CODE SHARED EXECUTE
DATA READWRITE

EXPORTS
	"?cin@std@@3V?$basic_istream@std@DV?$char_traits@std@D@1@@1@A"
	"?cout@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?cerr@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?clog@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?wcin@std@@3V?$basic_istream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcout@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcerr@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wclog@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
<<

..\lib\$(LIB_BASENAME)_stldebug.dll:	$(OBJS:build\=..\build\dynamic\stldebug\) ..\build\stlport.res
	*$(LINK) -WD -ND -g -L/IMPLIB:$(@R).lib -L/DE -L/DEBUG -o"$@" "$**" user32.lib kernel32.lib <<$(@R).def
LIBRARY "$(@F)"
DESCRIPTION 'STLport DLL for Digital Mars C/C++'
EXETYPE NT
SUBSYSTEM WINDOWS
CODE SHARED EXECUTE
DATA READWRITE

EXPORTS
	"?cin@std@@3V?$basic_istream@std@DV?$char_traits@std@D@1@@1@A"
	"?cout@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?cerr@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?clog@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?wcin@std@@3V?$basic_istream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcout@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcerr@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wclog@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
<<


all_sdynamic:	..\lib\$(LIB_BASENAME)s.dll ..\lib\$(LIB_BASENAME)s_debug.dll ..\lib\$(LIB_BASENAME)s_stldebug.dll

..\lib\$(LIB_BASENAME)s.dll:	$(OBJS:build\=..\build\sdynamic\release\) ..\build\stlport.res
	*$(LINK) -WD -ND -L/IMPLIB:$(@R).lib -L/DE -L/NODEBUG -o"$@" "$**" user32.lib kernel32.lib <<$(@R).def
LIBRARY "$(@F)"
DESCRIPTION 'STLport DLL for Digital Mars C/C++'
EXETYPE NT
SUBSYSTEM WINDOWS
CODE SHARED EXECUTE
DATA READWRITE

EXPORTS
	"?cin@std@@3V?$basic_istream@std@DV?$char_traits@std@D@1@@1@A"
	"?cout@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?cerr@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?clog@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?wcin@std@@3V?$basic_istream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcout@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcerr@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wclog@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
<<

..\lib\$(LIB_BASENAME)s_debug.dll:	$(OBJS:build\=..\build\sdynamic\debug\) ..\build\stlport.res
	*$(LINK) -WD -ND -g -L/IMPLIB:$(@R).lib -L/DE -L/DEBUG -o"$@" "$**" user32.lib kernel32.lib <<$(@R).def
LIBRARY "$(@F)"
DESCRIPTION 'STLport DLL for Digital Mars C/C++'
EXETYPE NT
SUBSYSTEM WINDOWS
CODE SHARED EXECUTE
DATA READWRITE

EXPORTS
	"?cin@std@@3V?$basic_istream@std@DV?$char_traits@std@D@1@@1@A"
	"?cout@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?cerr@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?clog@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?wcin@std@@3V?$basic_istream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcout@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcerr@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wclog@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
<<

..\lib\$(LIB_BASENAME)s_stldebug.dll:	$(OBJS:build\=..\build\sdynamic\stldebug\) ..\build\stlport.res
	*$(LINK) -WD -ND -g -L/IMPLIB:$(@R).lib -L/DE -L/DEBUG -o"$@" "$**" user32.lib kernel32.lib <<$(@R).def
LIBRARY "$(@F)"
DESCRIPTION 'STLport DLL for Digital Mars C/C++'
EXETYPE NT
SUBSYSTEM WINDOWS
CODE SHARED EXECUTE
DATA READWRITE

EXPORTS
	"?cin@std@@3V?$basic_istream@std@DV?$char_traits@std@D@1@@1@A"
	"?cout@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?cerr@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?clog@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?wcin@std@@3V?$basic_istream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcout@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcerr@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wclog@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
<<
#	-*- Makefile -*-
.SUFFIXES: .cpp .c .obj .dll .exe .rc .res


CXX=dmc
CC=dmc -cpp
LIB=lib
LINK=dmc
RC=rcc

LIB_BASENAME=stlp45dm

OBJS = build\c_locale.obj build\c_locale_stub.obj build\codecvt.obj \
	build\collate.obj build\complex.obj build\complex_exp.obj \
	build\complex_io.obj build\complex_io_w.obj build\complex_trig.obj \
	build\ctype.obj build\dll_main.obj \
	build\facets_byname.obj build\fstream.obj build\ios.obj \
	build\iostream.obj build\istream.obj build\locale.obj \
	build\locale_catalog.obj build\locale_impl.obj \
	build\messages.obj build\monetary.obj build\num_get.obj \
	build\num_get_float.obj build\num_put.obj \
	build\num_put_float.obj build\numpunct.obj build\ostream.obj \
	build\sstream.obj build\stdio_streambuf.obj \
	build\streambuf.obj build\string_w.obj build\strstream.obj \
	build\time_facets.obj

BUILD_DIRS=..\lib ..\build \
	..\build\static ..\build\static\release \
	..\build\static\debug ..\build\static\stldebug \
	..\build\staticx ..\build\staticx\release \
	..\build\staticx\debug ..\build\staticx\stldebug \
	..\build\dynamic ..\build\dynamic\release \
	..\build\dynamic\debug ..\build\dynamic\stldebug \
	..\build\sdynamic ..\build\sdynamic\release \
	..\build\sdynamic\debug ..\build\sdynamic\stldebug


CXXFLAGS_COMMON = -Ae -Ar -DSTRICT -D__BUILDING_STLPORT -I../stlport

# four versions are currently supported:
#  - static: static STLport library, static RTL
#  - staticx: static STLport library, dynamic RTL
#  - dynamic: dynamic STLport library, dynamic RTL
#  - sdynamic: dynamic STLport library, static RTL

CXXFLAGS_RELEASE_static = $(CXXFLAGS_COMMON) -Nc -o+all -D_MT
CXXFLAGS_RELEASE_staticx = $(CXXFLAGS_COMMON) -Nc -o+all -ND
CXXFLAGS_RELEASE_dynamic = $(CXXFLAGS_COMMON) -WD -o+all -ND
CXXFLAGS_RELEASE_sdynamic = $(CXXFLAGS_COMMON) -WD -o+all -D_MT

CXXFLAGS_DEBUG_static = $(CXXFLAGS_COMMON) -Nc -g -D_MT
CXXFLAGS_DEBUG_staticx = $(CXXFLAGS_COMMON) -Nc -g -ND
CXXFLAGS_DEBUG_dynamic = $(CXXFLAGS_COMMON) -WD -g -ND
CXXFLAGS_DEBUG_sdynamic = $(CXXFLAGS_COMMON) -WD -g -D_MT

CXXFLAGS_STLDEBUG_static = $(CXXFLAGS_DEBUG_static) -D_STLP_DEBUG
CXXFLAGS_STLDEBUG_staticx = $(CXXFLAGS_DEBUG_staticx) -D_STLP_DEBUG
CXXFLAGS_STLDEBUG_dynamic = $(CXXFLAGS_DEBUG_dynamic) -D_STLP_DEBUG
CXXFLAGS_STLDEBUG_sdynamic = $(CXXFLAGS_DEBUG_sdynamic) -D_STLP_DEBUG


.cpp{..\build\static\release}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_RELEASE_static) "$<"

.cpp{..\build\static\debug}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_DEBUG_static) "$<"

.cpp{..\build\static\stldebug}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_STLDEBUG_static) "$<"

.cpp{..\build\staticx\release}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_RELEASE_staticx) "$<"

.cpp{..\build\staticx\debug}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_DEBUG_staticx) "$<"

.cpp{..\build\staticx\stldebug}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_STLDEBUG_staticx) "$<"

.cpp{..\build\dynamic\release}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_RELEASE_dynamic) "$<"

.cpp{..\build\dynamic\debug}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_DEBUG_dynamic) "$<"

.cpp{..\build\dynamic\stldebug}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_STLDEBUG_dynamic) "$<"

.cpp{..\build\sdynamic\release}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_RELEASE_sdynamic) "$<"

.cpp{..\build\sdynamic\debug}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_DEBUG_sdynamic) "$<"

.cpp{..\build\sdynamic\stldebug}.obj:
	$(CXX) -c -o"$@" $(CXXFLAGS_STLDEBUG_sdynamic) "$<"

.c{..\build\static\release}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_RELEASE_static) "$<"

.c{..\build\static\debug}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_DEBUG_static) "$<"

.c{..\build\static\stldebug}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_STLDEBUG_static) "$<"

.c{..\build\staticx\release}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_RELEASE_staticx) "$<"

.c{..\build\staticx\debug}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_DEBUG_staticx) "$<"

.c{..\build\staticx\stldebug}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_STLDEBUG_staticx) "$<"

.c{..\build\dynamic\release}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_RELEASE_dynamic) "$<"

.c{..\build\dynamic\debug}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_DEBUG_dynamic) "$<"

.c{..\build\dynamic\stldebug}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_STLDEBUG_dynamic) "$<"

.c{..\build\sdynamic\release}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_RELEASE_sdynamic) "$<"

.c{..\build\sdynamic\debug}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_DEBUG_sdynamic) "$<"

.c{..\build\sdynamic\stldebug}.obj:
	$(CC) -c -o"$@" $(CXXFLAGS_STLDEBUG_sdynamic) "$<"


.rc{..\build}.res:
	$(RC) -32 -o"$@" "$<"


all:	directories all_static all_staticx all_dynamic all_sdynamic


directories:	$(BUILD_DIRS)

$(BUILD_DIRS):
	mkdir $@


all_static:	..\lib\$(LIB_BASENAME)_static.lib ..\lib\$(LIB_BASENAME)_debug_static.lib ..\lib\$(LIB_BASENAME)_stldebug_static.lib

..\lib\$(LIB_BASENAME)_static.lib:	$(OBJS:build\=..\build\static\release\)
	*$(LIB) -c -n -p64 "$@" "$**"

..\lib\$(LIB_BASENAME)_debug_static.lib:	$(OBJS:build\=..\build\static\debug\)
	*$(LIB) -c -n -p128 "$@" "$**"

..\lib\$(LIB_BASENAME)_stldebug_static.lib:	$(OBJS:build\=..\build\static\stldebug\)
	*$(LIB) -c -n -p256 "$@" "$**"


all_staticx:	..\lib\$(LIB_BASENAME)_staticx.lib ..\lib\$(LIB_BASENAME)_debug_staticx.lib ..\lib\$(LIB_BASENAME)_stldebug_staticx.lib

..\lib\$(LIB_BASENAME)_staticx.lib:	$(OBJS:build\=..\build\staticx\release\)
	*$(LIB) -c -n -p64 "$@" "$**"

..\lib\$(LIB_BASENAME)_debug_staticx.lib:	$(OBJS:build\=..\build\staticx\debug\)
	*$(LIB) -c -n -p128 "$@" "$**"

..\lib\$(LIB_BASENAME)_stldebug_staticx.lib:	$(OBJS:build\=..\build\staticx\stldebug\)
	*$(LIB) -c -n -p256 "$@" "$**"


all_dynamic:	..\lib\$(LIB_BASENAME).dll ..\lib\$(LIB_BASENAME)_debug.dll ..\lib\$(LIB_BASENAME)_stldebug.dll

..\lib\$(LIB_BASENAME).dll:	$(OBJS:build\=..\build\dynamic\release\) ..\build\stlport.res
	*$(LINK) -WD -ND -L/IMPLIB:$(@R).lib -L/DE -L/NODEBUG -o"$@" "$**" user32.lib kernel32.lib <<$(@R).def
LIBRARY "$(@F)"
DESCRIPTION 'STLport DLL for Digital Mars C/C++'
EXETYPE NT
SUBSYSTEM WINDOWS
CODE SHARED EXECUTE
DATA READWRITE

EXPORTS
	"?cin@std@@3V?$basic_istream@std@DV?$char_traits@std@D@1@@1@A"
	"?cout@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?cerr@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?clog@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?wcin@std@@3V?$basic_istream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcout@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcerr@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wclog@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
<<

..\lib\$(LIB_BASENAME)_debug.dll:	$(OBJS:build\=..\build\dynamic\debug\) ..\build\stlport.res
	*$(LINK) -WD -ND -g -L/IMPLIB:$(@R).lib -L/DE -L/DEBUG -o"$@" "$**" user32.lib kernel32.lib <<$(@R).def
LIBRARY "$(@F)"
DESCRIPTION 'STLport DLL for Digital Mars C/C++'
EXETYPE NT
SUBSYSTEM WINDOWS
CODE SHARED EXECUTE
DATA READWRITE

EXPORTS
	"?cin@std@@3V?$basic_istream@std@DV?$char_traits@std@D@1@@1@A"
	"?cout@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?cerr@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?clog@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?wcin@std@@3V?$basic_istream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcout@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcerr@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wclog@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
<<

..\lib\$(LIB_BASENAME)_stldebug.dll:	$(OBJS:build\=..\build\dynamic\stldebug\) ..\build\stlport.res
	*$(LINK) -WD -ND -g -L/IMPLIB:$(@R).lib -L/DE -L/DEBUG -o"$@" "$**" user32.lib kernel32.lib <<$(@R).def
LIBRARY "$(@F)"
DESCRIPTION 'STLport DLL for Digital Mars C/C++'
EXETYPE NT
SUBSYSTEM WINDOWS
CODE SHARED EXECUTE
DATA READWRITE

EXPORTS
	"?cin@std@@3V?$basic_istream@std@DV?$char_traits@std@D@1@@1@A"
	"?cout@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?cerr@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?clog@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?wcin@std@@3V?$basic_istream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcout@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcerr@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wclog@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
<<


all_sdynamic:	..\lib\$(LIB_BASENAME)s.dll ..\lib\$(LIB_BASENAME)s_debug.dll ..\lib\$(LIB_BASENAME)s_stldebug.dll

..\lib\$(LIB_BASENAME)s.dll:	$(OBJS:build\=..\build\sdynamic\release\) ..\build\stlport.res
	*$(LINK) -WD -ND -L/IMPLIB:$(@R).lib -L/DE -L/NODEBUG -o"$@" "$**" user32.lib kernel32.lib <<$(@R).def
LIBRARY "$(@F)"
DESCRIPTION 'STLport DLL for Digital Mars C/C++'
EXETYPE NT
SUBSYSTEM WINDOWS
CODE SHARED EXECUTE
DATA READWRITE

EXPORTS
	"?cin@std@@3V?$basic_istream@std@DV?$char_traits@std@D@1@@1@A"
	"?cout@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?cerr@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?clog@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?wcin@std@@3V?$basic_istream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcout@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcerr@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wclog@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
<<

..\lib\$(LIB_BASENAME)s_debug.dll:	$(OBJS:build\=..\build\sdynamic\debug\) ..\build\stlport.res
	*$(LINK) -WD -ND -g -L/IMPLIB:$(@R).lib -L/DE -L/DEBUG -o"$@" "$**" user32.lib kernel32.lib <<$(@R).def
LIBRARY "$(@F)"
DESCRIPTION 'STLport DLL for Digital Mars C/C++'
EXETYPE NT
SUBSYSTEM WINDOWS
CODE SHARED EXECUTE
DATA READWRITE

EXPORTS
	"?cin@std@@3V?$basic_istream@std@DV?$char_traits@std@D@1@@1@A"
	"?cout@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?cerr@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?clog@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?wcin@std@@3V?$basic_istream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcout@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcerr@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wclog@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
<<

..\lib\$(LIB_BASENAME)s_stldebug.dll:	$(OBJS:build\=..\build\sdynamic\stldebug\) ..\build\stlport.res
	*$(LINK) -WD -ND -g -L/IMPLIB:$(@R).lib -L/DE -L/DEBUG -o"$@" "$**" user32.lib kernel32.lib <<$(@R).def
LIBRARY "$(@F)"
DESCRIPTION 'STLport DLL for Digital Mars C/C++'
EXETYPE NT
SUBSYSTEM WINDOWS
CODE SHARED EXECUTE
DATA READWRITE

EXPORTS
	"?cin@std@@3V?$basic_istream@std@DV?$char_traits@std@D@1@@1@A"
	"?cout@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?cerr@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?clog@std@@3V?$basic_ostream@std@DV?$char_traits@std@D@1@@1@A"
	"?wcin@std@@3V?$basic_istream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcout@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wcerr@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
	"?wclog@std@@3V?$basic_ostream@std@_YV?$char_traits@std@_Y@1@@1@A"
<<
