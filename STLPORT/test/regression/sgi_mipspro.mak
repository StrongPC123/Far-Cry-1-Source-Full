# ;;; -*- Mode:makefile;-*- 
# Generated automatically from Makefile.in by configure.
# This requires GNU make.
.POSIX:

srcdir = .
VPATH = .

# point this to proper location
STL_INCL=-I${PWD}/../../stlport/

LIST  = stl_test.cpp accum1.cpp accum2.cpp \
	adjdiff0.cpp adjdiff1.cpp adjdiff2.cpp \
	adjfind0.cpp adjfind1.cpp adjfind2.cpp \
	advance.cpp \
	alg1.cpp alg2.cpp alg3.cpp alg4.cpp alg5.cpp \
	bcompos1.cpp bcompos2.cpp \
	bind1st1.cpp bind1st2.cpp \
	bind2nd1.cpp bind2nd2.cpp \
	binsert1.cpp binsert2.cpp \
	binsrch1.cpp binsrch2.cpp \
	bnegate1.cpp bnegate2.cpp bvec1.cpp \
	copy1.cpp copy2.cpp copy3.cpp copy4.cpp \
	copyb.cpp copyb0.cpp \
	count0.cpp count1.cpp \
	countif1.cpp \
	deque1.cpp \
	divides.cpp \
	eqlrnge0.cpp eqlrnge1.cpp eqlrnge2.cpp \
	equal0.cpp equal1.cpp equal2.cpp \
	equalto.cpp \
	fill1.cpp filln1.cpp \
	find0.cpp find1.cpp \
	findif0.cpp findif1.cpp \
	finsert1.cpp finsert2.cpp \
	foreach0.cpp foreach1.cpp \
	func1.cpp func2.cpp func3.cpp \
	gener1.cpp gener2.cpp \
	genern1.cpp genern2.cpp \
	greateq.cpp greater.cpp \
	incl0.cpp incl1.cpp incl2.cpp \
	inplmrg1.cpp inplmrg2.cpp \
	inrprod0.cpp inrprod1.cpp inrprod2.cpp \
	insert1.cpp insert2.cpp \
	iota1.cpp \
	istmit1.cpp \
	iter1.cpp iter2.cpp iter3.cpp iter4.cpp \
	iterswp0.cpp iterswp1.cpp \
	less.cpp \
	lesseq.cpp \
	lexcmp1.cpp lexcmp2.cpp \
	list1.cpp list2.cpp list3.cpp list4.cpp \
	logicand.cpp logicnot.cpp \
	logicor.cpp \
	lwrbnd1.cpp lwrbnd2.cpp \
	map1.cpp \
	max1.cpp max2.cpp \
	maxelem1.cpp maxelem2.cpp \
	memfunptr.cpp \
	merge0.cpp merge1.cpp merge2.cpp \
	min1.cpp min2.cpp \
	minelem1.cpp minelem2.cpp \
	minus.cpp \
	mismtch0.cpp mismtch1.cpp mismtch2.cpp \
	mkheap0.cpp mkheap1.cpp \
	mmap1.cpp mmap2.cpp \
	modulus.cpp \
	mset1.cpp mset3.cpp mset4.cpp mset5.cpp \
	negate.cpp nequal.cpp \
	nextprm0.cpp nextprm1.cpp nextprm2.cpp \
	nthelem0.cpp nthelem1.cpp nthelem2.cpp \
	ostmit.cpp \
	pair0.cpp pair1.cpp pair2.cpp \
	parsrt0.cpp parsrt1.cpp parsrt2.cpp \
	parsrtc0.cpp parsrtc1.cpp parsrtc2.cpp \
	partsrt0.cpp \
	partsum0.cpp partsum1.cpp partsum2.cpp \
	pheap1.cpp pheap2.cpp \
	plus.cpp \
	pqueue1.cpp \
	prevprm0.cpp prevprm1.cpp prevprm2.cpp \
	ptition0.cpp ptition1.cpp \
	ptrbinf1.cpp ptrbinf2.cpp \
	ptrunf1.cpp ptrunf2.cpp \
	queue1.cpp \
	rawiter.cpp \
	remcopy1.cpp \
	remcpif1.cpp \
	remif1.cpp \
	remove1.cpp \
	repcpif1.cpp \
	replace0.cpp replace1.cpp replcpy1.cpp replif1.cpp \
	revbit1.cpp revbit2.cpp \
	revcopy1.cpp reverse1.cpp reviter1.cpp reviter2.cpp \
	rndshuf0.cpp rndshuf1.cpp rndshuf2.cpp \
	rotate0.cpp rotate1.cpp rotcopy0.cpp rotcopy1.cpp \
	search0.cpp search1.cpp search2.cpp \
	set1.cpp set2.cpp \
	setdiff0.cpp setdiff1.cpp setdiff2.cpp \
	setintr0.cpp setintr1.cpp setintr2.cpp \
	setsymd0.cpp setsymd1.cpp setsymd2.cpp \
	setunon0.cpp setunon1.cpp setunon2.cpp \
	sort1.cpp sort2.cpp \
	stack1.cpp stack2.cpp \
	stblptn0.cpp stblptn1.cpp \
	stblsrt1.cpp stblsrt2.cpp \
	swap1.cpp \
	swprnge1.cpp \
	times.cpp \
	trnsfrm1.cpp trnsfrm2.cpp \
	ucompos1.cpp ucompos2.cpp \
	unegate1.cpp unegate2.cpp \
	uniqcpy1.cpp uniqcpy2.cpp \
	unique1.cpp unique2.cpp \
	uprbnd1.cpp uprbnd2.cpp \
	vec1.cpp vec2.cpp vec3.cpp vec4.cpp vec5.cpp vec6.cpp vec7.cpp vec8.cpp \
        hmap1.cpp hmmap1.cpp hset2.cpp hmset1.cpp slist1.cpp string1.cpp bitset1.cpp

OBJECTS  = stl_test.o accum1.o accum2.o \
	adjdiff0.o adjdiff1.o adjdiff2.o \
	adjfind0.o adjfind1.o adjfind2.o \
	advance.o \
	alg1.o alg2.o alg3.o alg4.o alg5.o \
	bcompos1.o bcompos2.o \
	bind1st1.o bind1st2.o \
	bind2nd1.o bind2nd2.o \
	binsert1.o binsert2.o \
	binsrch1.o binsrch2.o \
	bnegate1.o bnegate2.o bvec1.o \
	copy1.o copy2.o copy3.o copy4.o \
	copyb.o copyb0.o \
	count0.o count1.o \
	countif1.o \
	deque1.o \
	divides.o \
	eqlrnge0.o eqlrnge1.o eqlrnge2.o \
	equal0.o equal1.o equal2.o \
	equalto.o \
	fill1.o filln1.o \
	find0.o find1.o \
	findif0.o findif1.o \
	finsert1.o finsert2.o \
	foreach0.o foreach1.o \
	func1.o func2.o func3.o \
	gener1.o gener2.o \
	genern1.o genern2.o \
	greateq.o greater.o \
	incl0.o incl1.o incl2.o \
	inplmrg1.o inplmrg2.o \
	inrprod0.o inrprod1.o inrprod2.o \
	insert1.o insert2.o \
	iota1.o \
	istmit1.o \
	iter1.o iter2.o iter3.o iter4.o \
	iterswp0.o iterswp1.o \
	less.o \
	lesseq.o \
	lexcmp1.o lexcmp2.o \
	list1.o list2.o list3.o list4.o \
	logicand.o logicnot.o \
	logicor.o \
	lwrbnd1.o lwrbnd2.o \
	map1.o \
	max1.o max2.o \
	maxelem1.o maxelem2.o \
	merge0.o merge1.o merge2.o \
	min1.o min2.o \
	minelem1.o minelem2.o \
	minus.o \
	mismtch0.o mismtch1.o mismtch2.o \
	mkheap0.o mkheap1.o \
	mmap1.o mmap2.o \
	modulus.o \
	mset1.o mset3.o mset4.o mset5.o \
	negate.o nequal.o \
	nextprm0.o nextprm1.o nextprm2.o \
	nthelem0.o nthelem1.o nthelem2.o \
	ostmit.o \
	pair0.o pair1.o pair2.o \
	parsrt0.o parsrt1.o parsrt2.o \
	parsrtc0.o parsrtc1.o parsrtc2.o \
	partsrt0.o \
	partsum0.o partsum1.o partsum2.o \
	pheap1.o pheap2.o \
	plus.o \
	pqueue1.o \
	prevprm0.o prevprm1.o prevprm2.o \
	ptition0.o ptition1.o \
	ptrbinf1.o ptrbinf2.o \
	ptrunf1.o ptrunf2.o \
	queue1.o \
	rawiter.o \
	remcopy1.o \
	remcpif1.o \
	remif1.o \
	remove1.o \
	repcpif1.o \
	replace0.o replace1.o replcpy1.o replif1.o \
	revbit1.o revbit2.o \
	revcopy1.o reverse1.o reviter1.o reviter2.o \
	rndshuf0.o rndshuf1.o rndshuf2.o \
	rotate0.o rotate1.o rotcopy0.o rotcopy1.o \
	search0.o search1.o search2.o \
	set1.o set2.o \
	setdiff0.o setdiff1.o setdiff2.o \
	setintr0.o setintr1.o setintr2.o \
	setsymd0.o setsymd1.o setsymd2.o \
	setunon0.o setunon1.o setunon2.o \
	sort1.o sort2.o \
	stack1.o stack2.o \
	stblptn0.o stblptn1.o \
	stblsrt1.o stblsrt2.o \
	swap1.o \
	swprnge1.o \
	times.o \
	trnsfrm1.o trnsfrm2.o \
	ucompos1.o ucompos2.o \
	unegate1.o unegate2.o \
	uniqcpy1.o uniqcpy2.o \
	unique1.o unique2.o \
	uprbnd1.o uprbnd2.o \
	vec1.o vec2.o vec3.o vec4.o vec5.o vec6.o vec7.o vec8.o \
        hmap1.o hmmap1.o hset2.o hmset1.o string1.o bitset1.o slist1.o

EXECS = $(LIST:%.cpp=%.exe)
TESTS = $(LIST:%.cpp=%.out)
TEST_EXE  = stl_test.exe
TEST  = stl_test.out

CC = CC
CXX = $(CC)

CXXFLAGS = -J 4 -ansi -LANG:std -I. -D_STLP_DEBUG ${STL_INCL} ${DEBUG_FLAGS} -I. -D_STLP_NO_OWN_IOSTREAMS -D_STLP_NO_NEW_IOSTREAMS
CXXFLAGS = -J 4 -ansi -LANG:std -I. -D_STLP_DEBUG ${STL_INCL} ${DEBUG_FLAGS} -I. -D_STLP_NO_OWN_IOSTREAMS


LIBS = -lm 
LIBSTDCXX = 

check: $(TEST)

$(TEST) : $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) $(OBJECTS) $(LIBS) -o $(TEST_EXE)
	echo 'a string' | ./$(TEST_EXE) > $(TEST)

.SUFFIXES: .cpp .o .exe .out .res

.cpp.o:
	$(CXX) $(CXXFLAGS) $< -c -o $@

.cpp.out:
	$(CXX) $(CXXFLAGS) $< -c -USINGLE -DMAIN -o $*.o
	$(CXX) $(CXXFLAGS) $*.o $(LIBS) -g -o $*.exe
	./$*.exe > $@
	-rm -f $*.exe

istmit1.out: istmit1.cpp
	$(CXX) $(CXXFLAGS) $< $(STAT_MODULE) $(LIBSTDCXX) -lstdc++ $(LIBS) -o istmit1
	echo 'a string' | ./istmit1 > istmit1.out
	-rm -f ./istmit1

.cpp.s:
	$(CXX) $(CXXFLAGS) -O5 -D_STLP_USE_MALLOC -S -pto $<  -o $@

#	$(CXX) $(CXXFLAGS) -O5 -D_STLP_USE_MALLOC -noex -D_STLP_NO_EXCEPTIONS -S -pto $<  -o $@

#	$(CXX) $(CXXFLAGS) -O4 -noex -D_STLP_NO_EXCEPTIONS -D_STLP_NO_EXCEPTIONS -S -pta $<  -o $@

clean:
	-rm -fr *.exe *.o *.rpo *.obj *.out Templates.DB SunWS_cache
