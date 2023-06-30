!include "{stl}:src:MrCpp.mak"

MAKEFILE     = MrCpp.mak


Needed_SysLibs = ¶
#		"{PPCLibraries}MrCIOStreams.o" ¶
		"{STL}:lib:STLportLib{config_}.o" ¶
		"{SharedLibraries}InterfaceLib" ¶
		"{SharedLibraries}StdCLib_3.7" ¶
		"{SharedLibraries}MathLib" ¶
		"{PPCLibraries}StdCRuntime.o" ¶
		"{PPCLibraries}PPCCRuntime.o" ¶
		"{PPCLibraries}PPCToolLibs.o" ¶
		"{PPCLibraries}MrCPlusLib.o" ¶
		"{SharedLibraries}MrCExceptionsLib_4.1" ¶
		# end


##################################################################################
#	test:regression: build rule
##################################################################################
Regression_Objects_PPC  = ¶
	"{ObjDir}accum1.cpp.x" ¶
	"{ObjDir}accum2.cpp.x" ¶
	"{ObjDir}adjdiff0.cpp.x" ¶
	"{ObjDir}adjdiff1.cpp.x" ¶
	"{ObjDir}adjdiff2.cpp.x" ¶
	"{ObjDir}adjfind0.cpp.x" ¶
	"{ObjDir}adjfind1.cpp.x" ¶
	"{ObjDir}adjfind2.cpp.x" ¶
	"{ObjDir}advance.cpp.x" ¶
	"{ObjDir}alg1.cpp.x" ¶
	"{ObjDir}alg2.cpp.x" ¶
	"{ObjDir}alg3.cpp.x" ¶
	"{ObjDir}alg4.cpp.x" ¶
	"{ObjDir}alg5.cpp.x" ¶
	"{ObjDir}bcompos1.cpp.x" ¶
	"{ObjDir}bcompos2.cpp.x" ¶
	"{ObjDir}bind1st1.cpp.x" ¶
	"{ObjDir}bind1st2.cpp.x" ¶
	"{ObjDir}bind2nd1.cpp.x" ¶
	"{ObjDir}bind2nd2.cpp.x" ¶
	"{ObjDir}binsert1.cpp.x" ¶
	"{ObjDir}binsert2.cpp.x" ¶
	"{ObjDir}binsrch1.cpp.x" ¶
	"{ObjDir}binsrch2.cpp.x" ¶
	"{ObjDir}bitset1.cpp.x" ¶
	"{ObjDir}bnegate1.cpp.x" ¶
	"{ObjDir}bnegate2.cpp.x" ¶
	"{ObjDir}bvec1.cpp.x" ¶
	"{ObjDir}copy1.cpp.x" ¶
	"{ObjDir}copy2.cpp.x" ¶
	"{ObjDir}copy3.cpp.x" ¶
	"{ObjDir}copy4.cpp.x" ¶
	"{ObjDir}copyb.cpp.x" ¶
	"{ObjDir}copyb0.cpp.x" ¶
	"{ObjDir}count0.cpp.x" ¶
	"{ObjDir}count1.cpp.x" ¶
	"{ObjDir}countif1.cpp.x" ¶
	"{ObjDir}deque1.cpp.x" ¶
	"{ObjDir}divides.cpp.x" ¶
	"{ObjDir}eqlrnge0.cpp.x" ¶
	"{ObjDir}eqlrnge1.cpp.x" ¶
	"{ObjDir}eqlrnge2.cpp.x" ¶
	"{ObjDir}equal0.cpp.x" ¶
	"{ObjDir}equal1.cpp.x" ¶
	"{ObjDir}equal2.cpp.x" ¶
	"{ObjDir}equalto.cpp.x" ¶
	"{ObjDir}fill1.cpp.x" ¶
	"{ObjDir}filln1.cpp.x" ¶
	"{ObjDir}find0.cpp.x" ¶
	"{ObjDir}find1.cpp.x" ¶
	"{ObjDir}findif0.cpp.x" ¶
	"{ObjDir}findif1.cpp.x" ¶
	"{ObjDir}finsert1.cpp.x" ¶
	"{ObjDir}finsert2.cpp.x" ¶
	"{ObjDir}foreach0.cpp.x" ¶
	"{ObjDir}foreach1.cpp.x" ¶
	"{ObjDir}func1.cpp.x" ¶
	"{ObjDir}func2.cpp.x" ¶
	"{ObjDir}func3.cpp.x" ¶
	"{ObjDir}gener1.cpp.x" ¶
	"{ObjDir}gener2.cpp.x" ¶
	"{ObjDir}genern1.cpp.x" ¶
	"{ObjDir}genern2.cpp.x" ¶
	"{ObjDir}greateq.cpp.x" ¶
	"{ObjDir}greater.cpp.x" ¶
	"{ObjDir}hmap1.cpp.x" ¶
	"{ObjDir}hmmap1.cpp.x" ¶
	"{ObjDir}hmset1.cpp.x" ¶
	"{ObjDir}hset2.cpp.x" ¶
	"{ObjDir}incl0.cpp.x" ¶
	"{ObjDir}incl1.cpp.x" ¶
	"{ObjDir}incl2.cpp.x" ¶
	"{ObjDir}inplmrg1.cpp.x" ¶
	"{ObjDir}inplmrg2.cpp.x" ¶
	"{ObjDir}inrprod0.cpp.x" ¶
	"{ObjDir}inrprod1.cpp.x" ¶
	"{ObjDir}inrprod2.cpp.x" ¶
	"{ObjDir}insert1.cpp.x" ¶
	"{ObjDir}insert2.cpp.x" ¶
	"{ObjDir}iota1.cpp.x" ¶
	"{ObjDir}istmit1.cpp.x" ¶
	"{ObjDir}iter1.cpp.x" ¶
	"{ObjDir}iter2.cpp.x" ¶
	"{ObjDir}iter3.cpp.x" ¶
	"{ObjDir}iter4.cpp.x" ¶
	"{ObjDir}iterswp0.cpp.x" ¶
	"{ObjDir}iterswp1.cpp.x" ¶
	"{ObjDir}less.cpp.x" ¶
	"{ObjDir}lesseq.cpp.x" ¶
	"{ObjDir}lexcmp1.cpp.x" ¶
	"{ObjDir}lexcmp2.cpp.x" ¶
	"{ObjDir}list1.cpp.x" ¶
	"{ObjDir}list2.cpp.x" ¶
	"{ObjDir}list3.cpp.x" ¶
	"{ObjDir}list4.cpp.x" ¶
	"{ObjDir}logicand.cpp.x" ¶
	"{ObjDir}logicnot.cpp.x" ¶
	"{ObjDir}logicor.cpp.x" ¶
	"{ObjDir}lwrbnd1.cpp.x" ¶
	"{ObjDir}lwrbnd2.cpp.x" ¶
	"{ObjDir}map1.cpp.x" ¶
	"{ObjDir}max1.cpp.x" ¶
	"{ObjDir}max2.cpp.x" ¶
	"{ObjDir}maxelem1.cpp.x" ¶
	"{ObjDir}maxelem2.cpp.x" ¶
	"{ObjDir}memfunptr.cpp.x" ¶
	"{ObjDir}merge0.cpp.x" ¶
	"{ObjDir}merge1.cpp.x" ¶
	"{ObjDir}merge2.cpp.x" ¶
	"{ObjDir}min1.cpp.x" ¶
	"{ObjDir}min2.cpp.x" ¶
	"{ObjDir}minelem1.cpp.x" ¶
	"{ObjDir}minelem2.cpp.x" ¶
	"{ObjDir}minus.cpp.x" ¶
	"{ObjDir}mismtch0.cpp.x" ¶
	"{ObjDir}mismtch1.cpp.x" ¶
	"{ObjDir}mismtch2.cpp.x" ¶
	"{ObjDir}mkheap0.cpp.x" ¶
	"{ObjDir}mkheap1.cpp.x" ¶
	"{ObjDir}mmap1.cpp.x" ¶
	"{ObjDir}mmap2.cpp.x" ¶
	"{ObjDir}modulus.cpp.x" ¶
	"{ObjDir}mset1.cpp.x" ¶
	"{ObjDir}mset3.cpp.x" ¶
	"{ObjDir}mset4.cpp.x" ¶
	"{ObjDir}mset5.cpp.x" ¶
	"{ObjDir}negate.cpp.x" ¶
	"{ObjDir}nequal.cpp.x" ¶
	"{ObjDir}nextprm0.cpp.x" ¶
	"{ObjDir}nextprm1.cpp.x" ¶
	"{ObjDir}nextprm2.cpp.x" ¶
	"{ObjDir}nthelem0.cpp.x" ¶
	"{ObjDir}nthelem1.cpp.x" ¶
	"{ObjDir}nthelem2.cpp.x" ¶
	"{ObjDir}ostmit.cpp.x" ¶
	"{ObjDir}pair0.cpp.x" ¶
	"{ObjDir}pair1.cpp.x" ¶
	"{ObjDir}pair2.cpp.x" ¶
	"{ObjDir}parsrt0.cpp.x" ¶
	"{ObjDir}parsrt1.cpp.x" ¶
	"{ObjDir}parsrt2.cpp.x" ¶
	"{ObjDir}parsrtc0.cpp.x" ¶
	"{ObjDir}parsrtc1.cpp.x" ¶
	"{ObjDir}parsrtc2.cpp.x" ¶
	"{ObjDir}partsrt0.cpp.x" ¶
	"{ObjDir}partsum0.cpp.x" ¶
	"{ObjDir}partsum1.cpp.x" ¶
	"{ObjDir}partsum2.cpp.x" ¶
	"{ObjDir}pheap1.cpp.x" ¶
	"{ObjDir}pheap2.cpp.x" ¶
	"{ObjDir}plus.cpp.x" ¶
	"{ObjDir}pqueue1.cpp.x" ¶
	"{ObjDir}prevprm0.cpp.x" ¶
	"{ObjDir}prevprm1.cpp.x" ¶
	"{ObjDir}prevprm2.cpp.x" ¶
	"{ObjDir}ptition0.cpp.x" ¶
	"{ObjDir}ptition1.cpp.x" ¶
	"{ObjDir}ptrbinf1.cpp.x" ¶
	"{ObjDir}ptrbinf2.cpp.x" ¶
	"{ObjDir}ptrunf1.cpp.x" ¶
	"{ObjDir}ptrunf2.cpp.x" ¶
	"{ObjDir}queue1.cpp.x" ¶
	"{ObjDir}rawiter.cpp.x" ¶
	"{ObjDir}remcopy1.cpp.x" ¶
	"{ObjDir}remcpif1.cpp.x" ¶
	"{ObjDir}remif1.cpp.x" ¶
	"{ObjDir}remove1.cpp.x" ¶
	"{ObjDir}repcpif1.cpp.x" ¶
	"{ObjDir}replace0.cpp.x" ¶
	"{ObjDir}replace1.cpp.x" ¶
	"{ObjDir}replcpy1.cpp.x" ¶
	"{ObjDir}replif1.cpp.x" ¶
	"{ObjDir}revbit1.cpp.x" ¶
	"{ObjDir}revbit2.cpp.x" ¶
	"{ObjDir}revcopy1.cpp.x" ¶
	"{ObjDir}reverse1.cpp.x" ¶
	"{ObjDir}reviter1.cpp.x" ¶
	"{ObjDir}reviter2.cpp.x" ¶
	"{ObjDir}rndshuf0.cpp.x" ¶
	"{ObjDir}rndshuf1.cpp.x" ¶
	"{ObjDir}rndshuf2.cpp.x" ¶
	"{ObjDir}rotate0.cpp.x" ¶
	"{ObjDir}rotate1.cpp.x" ¶
	"{ObjDir}rotcopy0.cpp.x" ¶
	"{ObjDir}rotcopy1.cpp.x" ¶
	"{ObjDir}search0.cpp.x" ¶
	"{ObjDir}search1.cpp.x" ¶
	"{ObjDir}search2.cpp.x" ¶
	"{ObjDir}set1.cpp.x" ¶
	"{ObjDir}set2.cpp.x" ¶
	"{ObjDir}setdiff0.cpp.x" ¶
	"{ObjDir}setdiff1.cpp.x" ¶
	"{ObjDir}setdiff2.cpp.x" ¶
	"{ObjDir}setintr0.cpp.x" ¶
	"{ObjDir}setintr1.cpp.x" ¶
	"{ObjDir}setintr2.cpp.x" ¶
	"{ObjDir}setsymd0.cpp.x" ¶
	"{ObjDir}setsymd1.cpp.x" ¶
	"{ObjDir}setsymd2.cpp.x" ¶
	"{ObjDir}setunon0.cpp.x" ¶
	"{ObjDir}setunon1.cpp.x" ¶
	"{ObjDir}setunon2.cpp.x" ¶
#	"{ObjDir}single.cpp.x" ¶
	"{ObjDir}slist1.cpp.x" ¶
	"{ObjDir}sort1.cpp.x" ¶
	"{ObjDir}sort2.cpp.x" ¶
	"{ObjDir}stack1.cpp.x" ¶
	"{ObjDir}stack2.cpp.x" ¶
#	"{ObjDir}stat.cpp.x" ¶
	"{ObjDir}stblptn0.cpp.x" ¶
	"{ObjDir}stblptn1.cpp.x" ¶
	"{ObjDir}stblsrt1.cpp.x" ¶
	"{ObjDir}stblsrt2.cpp.x" ¶
#	"{ObjDir}stl_single.cpp.x" ¶
	"{ObjDir}stl_test.cpp.x" ¶
	"{ObjDir}string1.cpp.x" ¶
	"{ObjDir}swap1.cpp.x" ¶
	"{ObjDir}swprnge1.cpp.x" ¶
	"{ObjDir}times.cpp.x" ¶
	"{ObjDir}trnsfrm1.cpp.x" ¶
	"{ObjDir}trnsfrm2.cpp.x" ¶
	"{ObjDir}ucompos1.cpp.x" ¶
	"{ObjDir}ucompos2.cpp.x" ¶
	"{ObjDir}unegate1.cpp.x" ¶
	"{ObjDir}unegate2.cpp.x" ¶
	"{ObjDir}uniqcpy1.cpp.x" ¶
	"{ObjDir}uniqcpy2.cpp.x" ¶
	"{ObjDir}unique1.cpp.x" ¶
	"{ObjDir}unique2.cpp.x" ¶
	"{ObjDir}uprbnd1.cpp.x" ¶
	"{ObjDir}uprbnd2.cpp.x" ¶
	"{ObjDir}vec1.cpp.x" ¶
	"{ObjDir}vec2.cpp.x" ¶
	"{ObjDir}vec3.cpp.x" ¶
	"{ObjDir}vec4.cpp.x" ¶
	"{ObjDir}vec5.cpp.x" ¶
	"{ObjDir}vec6.cpp.x" ¶
	"{ObjDir}vec7.cpp.x" ¶
	"{ObjDir}vec8.cpp.x" ¶
	# end

Regression_test ÄÄ setup
Regression_test ÄÄ "{ObjDir}"Regression_test.PPC
	echo "¶n'{ObjDir}Regression_test.PPC' < '{stl}:test:regression:stdin' # execute it"

"{ObjDir}"Regression_test.PPC ÄÄ {¥MondoBuild¥} {Regression_Objects_PPC} {Needed_SysLibs}
	###########
	if "{compile_status}"
		echo "### LINK NOT PERFORMED DUE TO COMPILE ERROR"
		exit "{compile_status}"
	end
	echo "¶nLinking:        {Targ}"
	if "`exists {Targ}`" 
		delete {Targ}		#*TY 01/14/1999 - it is faster to generate executable from the ground up than modifying the existing ones.
	end
	PPCLink ¶
		-t 'MPST' ¶
		-o {Targ} ¶
		{Regression_Objects_PPC} ¶
		{Needed_SysLibs} ¶
		{Link_options} ¶
		# end



##################################################################################
#	test:eh: build rule
##################################################################################
eh_Objects_PPC  = ¶
	"{ObjDir}main.cpp.x" ¶
	"{ObjDir}nc_alloc.cpp.x" ¶
	"{ObjDir}random_number.cpp.x" ¶
	"{ObjDir}test_algo.cpp.x" ¶
	"{ObjDir}test_algobase.cpp.x" ¶
	"{ObjDir}test_bit_vector.cpp.x" ¶
	"{ObjDir}test_bitset.cpp.x" ¶
	"{ObjDir}test_deque.cpp.x" ¶
	"{ObjDir}test_hash_map.cpp.x" ¶
	"{ObjDir}test_hash_set.cpp.x" ¶
	"{ObjDir}test_list.cpp.x" ¶
	"{ObjDir}test_map.cpp.x" ¶
	"{ObjDir}test_rope.cpp.x" ¶
	"{ObjDir}test_set.cpp.x" ¶
	"{ObjDir}test_slist.cpp.x" ¶
	"{ObjDir}test_string.cpp.x" ¶
	"{ObjDir}test_valarray.cpp.x" ¶
	"{ObjDir}test_vector.cpp.x" ¶
	"{ObjDir}TestClass.cpp.x" ¶
	#end

EH_test ÄÄ setup
EH_test ÄÄ "{ObjDir}"EH_test.PPC
	echo "¶n'{ObjDir}EH_test.PPC' # execute it"

"{ObjDir}"EH_test.PPC ÄÄ "{stl}:test:regression:{¥MondoBuild¥}" {eh_Objects_PPC} {Needed_SysLibs}
	###########
	if "{compile_status}"
		echo "### LINK NOT PERFORMED DUE TO COMPILE TIME ERROR"
		exit "{compile_status}"
	end
	echo "¶nLinking:        {Targ}"
	if "`exists {Targ}`" 
		delete {Targ}
	end
	PPCLink ¶
		-t 'MPST' ¶
		-o {Targ} ¶
		{eh_Objects_PPC} ¶
		{Link_options} ¶
		{Needed_SysLibs} ¶
		# end

