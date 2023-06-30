# ------------------------------------------------------
# Makefile for IBM C/C++ for OS/390
# ------------------------------------------------------
# point this to proper location
STL_INCL=-I../../stlport

# list of objects
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
	memfunptr.o \
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
        hmap1.o hmmap1.o hset2.o hmset1.o slist1.o string1.o bitset1.o

TEST_EXE  = stl_test.exe
TEST  = stl_test.out

CC = c++
CXX = $(CC)
DEBUG_FLAGS=
# Use this for a debug version
# DEBUG_FLAGS=-g
CXXFLAGS = ${STL_INCL} ${DEBUG_FLAGS} -I. -W c,"langlvl(extended)"

check: $(TEST)

$(TEST) : $(TEST_EXE)
	echo 'a string' | $(TEST_EXE) > $(TEST)

$(TEST_EXE) : $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TEST_EXE)

.SUFFIXES: .cpp .o .exe .out .C

.cpp.o:
	_CXX_CXXSUFFIX=cpp $(CXX) $(CXXFLAGS) -c $<

.C.o:
	_CXX_CXXSUFFIX=C $(CXX) $(CXXFLAGS) -c $<

.cpp.out:
	_CXX_CXXSUFFIX=cpp $(CXX) $(CXXFLAGS) -c -USINGLE -DMAIN $<
	$(CXX) $(CXXFLAGS) $*.o -o $*.exe
	./$*.exe > $@
	rm -f $*.exe

.cpp.exe:
	_CXX_CXXSUFFIX=cpp $(CXX) $(CXXFLAGS) -c -USINGLE -DMAIN $<
	$(CXX) $(CXXFLAGS) $*.o -o $*.exe

istmit1.out: istmit1.cpp
	_CXX_CXXSUFFIX=cpp $(CXX) $(CXXFLAGS) -c -USINGLE -DMAIN $<
	$(CXX) $(CXXFLAGS) $*.o -o $*.exe
	echo 'a string' | ./$*.exe > $@
	rm -f ./$*.exe

clean:
	-rm -fr *.exe *.o *.obj *.out tempinc
