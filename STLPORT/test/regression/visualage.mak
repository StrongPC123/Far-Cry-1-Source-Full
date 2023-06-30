# Makefile for IBM nmake, VisualAge C++ 3.6.5 for OS/2 and windows


# point this to proper location
STL_INCL=..\..\stlport

OBJECTS  = stl_test.obj accum1.obj accum2.obj \
	adjdiff0.obj adjdiff1.obj adjdiff2.obj \
	adjfind0.obj adjfind1.obj adjfind2.obj \
	advance.obj \
	alg1.obj alg2.obj alg3.obj alg4.obj alg5.obj \
	bcompos1.obj bcompos2.obj \
	bind1st1.obj bind1st2.obj \
	bind2nd1.obj bind2nd2.obj \
	binsert1.obj binsert2.obj \
	binsrch1.obj binsrch2.obj \
	bnegate1.obj bnegate2.obj bvec1.obj \
	copy1.obj copy2.obj copy3.obj copy4.obj \
	copyb.obj copyb0.obj \
	count0.obj count1.obj \
	countif1.obj \
	deque1.obj \
	divides.obj \
	eqlrnge0.obj eqlrnge1.obj eqlrnge2.obj \
	equal0.obj equal1.obj equal2.obj \
	equalto.obj \
	fill1.obj filln1.obj \
	find0.obj find1.obj \
	findif0.obj findif1.obj \
	finsert1.obj finsert2.obj \
	foreach0.obj foreach1.obj \
	func1.obj func2.obj func3.obj \
	gener1.obj gener2.obj \
	genern1.obj genern2.obj \
	greateq.obj greater.obj \
	incl0.obj incl1.obj incl2.obj \
	inplmrg1.obj inplmrg2.obj \
	inrprod0.obj inrprod1.obj inrprod2.obj \
	insert1.obj insert2.obj \
	iota1.obj \
	istmit1.obj \
	iter1.obj iter2.obj iter3.obj iter4.obj \
	iterswp0.obj iterswp1.obj \
	less.obj \
	lesseq.obj \
	lexcmp1.obj lexcmp2.obj \
	list1.obj list2.obj list3.obj list4.obj \
	logicand.obj logicnot.obj \
	logicor.obj \
	lwrbnd1.obj lwrbnd2.obj \
	map1.obj \
	max1.obj max2.obj \
	maxelem1.obj maxelem2.obj \
	memfunptr.obj \
	merge0.obj merge1.obj merge2.obj \
	min1.obj min2.obj \
	minelem1.obj minelem2.obj \
	minus.obj \
	mismtch0.obj mismtch1.obj mismtch2.obj \
	mkheap0.obj mkheap1.obj \
	mmap1.obj mmap2.obj \
	modulus.obj \
	mset1.obj mset3.obj mset4.obj mset5.obj \
	negate.obj nequal.obj \
	nextprm0.obj nextprm1.obj nextprm2.obj \
	nthelem0.obj nthelem1.obj nthelem2.obj \
	ostmit.obj \
	pair0.obj pair1.obj pair2.obj \
	parsrt0.obj parsrt1.obj parsrt2.obj \
	parsrtc0.obj parsrtc1.obj parsrtc2.obj \
	partsrt0.obj \
	partsum0.obj partsum1.obj partsum2.obj \
	pheap1.obj pheap2.obj \
	plus.obj \
	pqueue1.obj \
	prevprm0.obj prevprm1.obj prevprm2.obj \
	ptition0.obj ptition1.obj \
	ptrbinf1.obj ptrbinf2.obj \
	ptrunf1.obj ptrunf2.obj \
	queue1.obj \
	rawiter.obj \
	remcopy1.obj \
	remcpif1.obj \
	remif1.obj \
	remove1.obj \
	repcpif1.obj \
	replace0.obj replace1.obj replcpy1.obj replif1.obj \
	revbit1.obj revbit2.obj \
	revcopy1.obj reverse1.obj reviter1.obj reviter2.obj \
	rndshuf0.obj rndshuf1.obj rndshuf2.obj \
	rotate0.obj rotate1.obj rotcopy0.obj rotcopy1.obj \
	search0.obj search1.obj search2.obj \
	set1.obj set2.obj \
	setdiff0.obj setdiff1.obj setdiff2.obj \
	setintr0.obj setintr1.obj setintr2.obj \
	setsymd0.obj setsymd1.obj setsymd2.obj \
	setunon0.obj setunon1.obj setunon2.obj \
	sort1.obj sort2.obj \
	stack1.obj stack2.obj \
	stblptn0.obj stblptn1.obj \
	stblsrt1.obj stblsrt2.obj \
	swap1.obj \
	swprnge1.obj \
	times.obj \
	trnsfrm1.obj trnsfrm2.obj \
	ucompos1.obj ucompos2.obj \
	unegate1.obj unegate2.obj \
	uniqcpy1.obj uniqcpy2.obj \
	unique1.obj unique2.obj \
	uprbnd1.obj uprbnd2.obj \
	vec1.obj vec2.obj vec3.obj vec4.obj vec5.obj vec6.obj vec7.obj vec8.obj \
  hmap1.obj hmmap1.obj hset2.obj hmset1.obj slist1.obj string1.obj bitset1.obj

#STAT_MODULE=stat.o
TEST_EXE  = stl_test.exe
TEST  = stl_test.out

CC = icc.exe
CXX = $(CC) -D_STLP_NO_OWN_IOSTREAMS
DEBUG_FLAGS=
# Use this for a Debug version
#DEBUG_FLAGS=-O- -Ti+

CXXFLAGS = -Q -I$(STL_INCL) $(DEBUG_FLAGS) -I. -Tdp

# Use this for no link time instantiation of templates
#CXXFLAGS = -Q -I$(STL_INCL) $(DEBUG_FLAGS) -I. -Tdp -Ft-

LIBS = 

check: stl_test.out

$(TEST) : $(TEST_EXE)
	echo a string| $(TEST_EXE) > $@

$(TEST_EXE) : $(OBJECTS)
  $(CXX) $(CXXFLAGS) @<<
$(OBJECTS)
$(LIBS)
-Fe$@
<<
  
.SUFFIXES: .cpp .obj .i .exe .out

.cpp.i:
  $(CXX) $(CXXFLAGS) -P $<

.cpp.obj:
	$(CXX) $(CXXFLAGS) -c $<

.cpp.out: 
	$(CXX) $(CXXFLAGS) -c -USINGLE -DMAIN $< 
	$(CXX) $(CXXFLAGS) $*.obj $(LIBS) -Fe$*.exe
	./$*.exe > $@
	del $*.exe
  
.cpp.exe:
	$(CXX) $(CXXFLAGS) -c -USINGLE -DMAIN $< 
	$(CXX) $(CXXFLAGS) $*.obj $(LIBS) -Fe$*.exe

#istmit1.out: istmit1.cpp
#	$(CXX) $(CXXFLAGS) $< $(STAT_MODULE) $(LIBSTDCXX) -lstdc++ $(LIBS) -o 
#istmit1 echo 'a string' | ./istmit1 > istmit1.out
#	-rm -f ./istmit1
#
#$(STAT_MODULE): stat.cpp
#	$(CXX) $(CXXFLAGS) -c $< -o $@
#
#%.s: %.cpp
#	$(CXX) $(CXXFLAGS) -O5 -D_STLP_USE_MALLOC -S -pto $<  -o $@

#	$(CXX) $(CXXFLAGS) -O5 -D_STLP_USE_MALLOC -noex -D_STLP_NO_EXCEPTIONS -S -pto $<  -o $@

#	$(CXX) $(CXXFLAGS) -O4 -noex -D_STLP_NO_EXCEPTIONS -D_STLP_NO_EXCEPTIONS -S -pta $<  -o $@

clean:
  del *.exe
  del *.obj
  del *.out
  del tempinc
	
