!include "{stl}:src:SCpp.mak"

MAKEFILE     = SCpp.mak


Needed_SysLibs = ¶
#		"{CLibraries}IOStreams.o" ¶
		"{STL}:lib:STLportLib{config_}.o" ¶
		"{Libraries}Stubs.o" ¶
		"{Libraries}MathLib.o" ¶
		"{CLibraries}CPlusLib.o" ¶
		"{CLibraries}StdCLib.o" ¶
		"{Libraries}MacRuntime.o" ¶
		"{Libraries}IntEnv.o" ¶
		#"{Libraries}ToolLibs.o" ¶
		"{Libraries}Interface.o" ¶
		# end


##################################################################################
#	test:regression: build rule
##################################################################################
Regression_Objects_68K  = ¶
	"{ObjDir}accum1.cpp.o" ¶
	"{ObjDir}accum2.cpp.o" ¶
	"{ObjDir}adjdiff0.cpp.o" ¶
	"{ObjDir}adjdiff1.cpp.o" ¶
	"{ObjDir}adjdiff2.cpp.o" ¶
	"{ObjDir}adjfind0.cpp.o" ¶
	"{ObjDir}adjfind1.cpp.o" ¶
	"{ObjDir}adjfind2.cpp.o" ¶
	"{ObjDir}advance.cpp.o" ¶
	"{ObjDir}alg1.cpp.o" ¶
	"{ObjDir}alg2.cpp.o" ¶
	"{ObjDir}alg3.cpp.o" ¶
	"{ObjDir}alg4.cpp.o" ¶
	"{ObjDir}alg5.cpp.o" ¶
	"{ObjDir}bcompos1.cpp.o" ¶
	"{ObjDir}bcompos2.cpp.o" ¶
	"{ObjDir}bind1st1.cpp.o" ¶
	"{ObjDir}bind1st2.cpp.o" ¶
	"{ObjDir}bind2nd1.cpp.o" ¶
	"{ObjDir}bind2nd2.cpp.o" ¶
	"{ObjDir}binsert1.cpp.o" ¶
	"{ObjDir}binsert2.cpp.o" ¶
	"{ObjDir}binsrch1.cpp.o" ¶
	"{ObjDir}binsrch2.cpp.o" ¶
	"{ObjDir}bitset1.cpp.o" ¶
	"{ObjDir}bnegate1.cpp.o" ¶
	"{ObjDir}bnegate2.cpp.o" ¶
	"{ObjDir}bvec1.cpp.o" ¶
	"{ObjDir}copy1.cpp.o" ¶
	"{ObjDir}copy2.cpp.o" ¶
	"{ObjDir}copy3.cpp.o" ¶
	"{ObjDir}copy4.cpp.o" ¶
	"{ObjDir}copyb.cpp.o" ¶
	"{ObjDir}copyb0.cpp.o" ¶
	"{ObjDir}count0.cpp.o" ¶
	"{ObjDir}count1.cpp.o" ¶
	"{ObjDir}countif1.cpp.o" ¶
	"{ObjDir}deque1.cpp.o" ¶
	"{ObjDir}divides.cpp.o" ¶
	"{ObjDir}eqlrnge0.cpp.o" ¶
	"{ObjDir}eqlrnge1.cpp.o" ¶
	"{ObjDir}eqlrnge2.cpp.o" ¶
	"{ObjDir}equal0.cpp.o" ¶
	"{ObjDir}equal1.cpp.o" ¶
	"{ObjDir}equal2.cpp.o" ¶
	"{ObjDir}equalto.cpp.o" ¶
	"{ObjDir}fill1.cpp.o" ¶
	"{ObjDir}filln1.cpp.o" ¶
	"{ObjDir}find0.cpp.o" ¶
	"{ObjDir}find1.cpp.o" ¶
	"{ObjDir}findif0.cpp.o" ¶
	"{ObjDir}findif1.cpp.o" ¶
	"{ObjDir}finsert1.cpp.o" ¶
	"{ObjDir}finsert2.cpp.o" ¶
	"{ObjDir}foreach0.cpp.o" ¶
	"{ObjDir}foreach1.cpp.o" ¶
	"{ObjDir}func1.cpp.o" ¶
	"{ObjDir}func2.cpp.o" ¶
	"{ObjDir}func3.cpp.o" ¶
	"{ObjDir}gener1.cpp.o" ¶
	"{ObjDir}gener2.cpp.o" ¶
	"{ObjDir}genern1.cpp.o" ¶
	"{ObjDir}genern2.cpp.o" ¶
	"{ObjDir}greateq.cpp.o" ¶
	"{ObjDir}greater.cpp.o" ¶
	"{ObjDir}hmap1.cpp.o" ¶
	"{ObjDir}hmmap1.cpp.o" ¶
	"{ObjDir}hmset1.cpp.o" ¶
	"{ObjDir}hset2.cpp.o" ¶
	"{ObjDir}incl0.cpp.o" ¶
	"{ObjDir}incl1.cpp.o" ¶
	"{ObjDir}incl2.cpp.o" ¶
	"{ObjDir}inplmrg1.cpp.o" ¶
	"{ObjDir}inplmrg2.cpp.o" ¶
	"{ObjDir}inrprod0.cpp.o" ¶
	"{ObjDir}inrprod1.cpp.o" ¶
	"{ObjDir}inrprod2.cpp.o" ¶
	"{ObjDir}insert1.cpp.o" ¶
	"{ObjDir}insert2.cpp.o" ¶
	"{ObjDir}iota1.cpp.o" ¶
	"{ObjDir}istmit1.cpp.o" ¶
	"{ObjDir}iter1.cpp.o" ¶
	"{ObjDir}iter2.cpp.o" ¶
	"{ObjDir}iter3.cpp.o" ¶
	"{ObjDir}iter4.cpp.o" ¶
	"{ObjDir}iterswp0.cpp.o" ¶
	"{ObjDir}iterswp1.cpp.o" ¶
	"{ObjDir}less.cpp.o" ¶
	"{ObjDir}lesseq.cpp.o" ¶
	"{ObjDir}lexcmp1.cpp.o" ¶
	"{ObjDir}lexcmp2.cpp.o" ¶
	"{ObjDir}list1.cpp.o" ¶
	"{ObjDir}list2.cpp.o" ¶
	"{ObjDir}list3.cpp.o" ¶
	"{ObjDir}list4.cpp.o" ¶
	"{ObjDir}logicand.cpp.o" ¶
	"{ObjDir}logicnot.cpp.o" ¶
	"{ObjDir}logicor.cpp.o" ¶
	"{ObjDir}lwrbnd1.cpp.o" ¶
	"{ObjDir}lwrbnd2.cpp.o" ¶
	"{ObjDir}map1.cpp.o" ¶
	"{ObjDir}max1.cpp.o" ¶
	"{ObjDir}max2.cpp.o" ¶
	"{ObjDir}maxelem1.cpp.o" ¶
	"{ObjDir}maxelem2.cpp.o" ¶
	"{ObjDir}merge0.cpp.o" ¶
	"{ObjDir}merge1.cpp.o" ¶
	"{ObjDir}merge2.cpp.o" ¶
	"{ObjDir}min1.cpp.o" ¶
	"{ObjDir}min2.cpp.o" ¶
	"{ObjDir}minelem1.cpp.o" ¶
	"{ObjDir}minelem2.cpp.o" ¶
	"{ObjDir}minus.cpp.o" ¶
	"{ObjDir}mismtch0.cpp.o" ¶
	"{ObjDir}mismtch1.cpp.o" ¶
	"{ObjDir}mismtch2.cpp.o" ¶
	"{ObjDir}mkheap0.cpp.o" ¶
	"{ObjDir}mkheap1.cpp.o" ¶
	"{ObjDir}mmap1.cpp.o" ¶
	"{ObjDir}mmap2.cpp.o" ¶
	"{ObjDir}modulus.cpp.o" ¶
	"{ObjDir}mset1.cpp.o" ¶
	"{ObjDir}mset3.cpp.o" ¶
	"{ObjDir}mset4.cpp.o" ¶
	"{ObjDir}mset5.cpp.o" ¶
	"{ObjDir}negate.cpp.o" ¶
	"{ObjDir}nequal.cpp.o" ¶
	"{ObjDir}nextprm0.cpp.o" ¶
	"{ObjDir}nextprm1.cpp.o" ¶
	"{ObjDir}nextprm2.cpp.o" ¶
	"{ObjDir}nthelem0.cpp.o" ¶
	"{ObjDir}nthelem1.cpp.o" ¶
	"{ObjDir}nthelem2.cpp.o" ¶
	"{ObjDir}ostmit.cpp.o" ¶
	"{ObjDir}pair0.cpp.o" ¶
	"{ObjDir}pair1.cpp.o" ¶
	"{ObjDir}pair2.cpp.o" ¶
	"{ObjDir}parsrt0.cpp.o" ¶
	"{ObjDir}parsrt1.cpp.o" ¶
	"{ObjDir}parsrt2.cpp.o" ¶
	"{ObjDir}parsrtc0.cpp.o" ¶
	"{ObjDir}parsrtc1.cpp.o" ¶
	"{ObjDir}parsrtc2.cpp.o" ¶
	"{ObjDir}partsrt0.cpp.o" ¶
	"{ObjDir}partsum0.cpp.o" ¶
	"{ObjDir}partsum1.cpp.o" ¶
	"{ObjDir}partsum2.cpp.o" ¶
	"{ObjDir}pheap1.cpp.o" ¶
	"{ObjDir}pheap2.cpp.o" ¶
	"{ObjDir}plus.cpp.o" ¶
	"{ObjDir}pqueue1.cpp.o" ¶
	"{ObjDir}prevprm0.cpp.o" ¶
	"{ObjDir}prevprm1.cpp.o" ¶
	"{ObjDir}prevprm2.cpp.o" ¶
	"{ObjDir}ptition0.cpp.o" ¶
	"{ObjDir}ptition1.cpp.o" ¶
	"{ObjDir}ptrbinf1.cpp.o" ¶
	"{ObjDir}ptrbinf2.cpp.o" ¶
	"{ObjDir}ptrunf1.cpp.o" ¶
	"{ObjDir}ptrunf2.cpp.o" ¶
	"{ObjDir}queue1.cpp.o" ¶
	"{ObjDir}rawiter.cpp.o" ¶
	"{ObjDir}remcopy1.cpp.o" ¶
	"{ObjDir}remcpif1.cpp.o" ¶
	"{ObjDir}remif1.cpp.o" ¶
	"{ObjDir}remove1.cpp.o" ¶
	"{ObjDir}repcpif1.cpp.o" ¶
	"{ObjDir}replace0.cpp.o" ¶
	"{ObjDir}replace1.cpp.o" ¶
	"{ObjDir}replcpy1.cpp.o" ¶
	"{ObjDir}replif1.cpp.o" ¶
	"{ObjDir}revbit1.cpp.o" ¶
	"{ObjDir}revbit2.cpp.o" ¶
	"{ObjDir}revcopy1.cpp.o" ¶
	"{ObjDir}reverse1.cpp.o" ¶
	"{ObjDir}reviter1.cpp.o" ¶
	"{ObjDir}reviter2.cpp.o" ¶
	"{ObjDir}rndshuf0.cpp.o" ¶
	"{ObjDir}rndshuf1.cpp.o" ¶
	"{ObjDir}rndshuf2.cpp.o" ¶
	"{ObjDir}rotate0.cpp.o" ¶
	"{ObjDir}rotate1.cpp.o" ¶
	"{ObjDir}rotcopy0.cpp.o" ¶
	"{ObjDir}rotcopy1.cpp.o" ¶
	"{ObjDir}search0.cpp.o" ¶
	"{ObjDir}search1.cpp.o" ¶
	"{ObjDir}search2.cpp.o" ¶
	"{ObjDir}set1.cpp.o" ¶
	"{ObjDir}set2.cpp.o" ¶
	"{ObjDir}setdiff0.cpp.o" ¶
	"{ObjDir}setdiff1.cpp.o" ¶
	"{ObjDir}setdiff2.cpp.o" ¶
	"{ObjDir}setintr0.cpp.o" ¶
	"{ObjDir}setintr1.cpp.o" ¶
	"{ObjDir}setintr2.cpp.o" ¶
	"{ObjDir}setsymd0.cpp.o" ¶
	"{ObjDir}setsymd1.cpp.o" ¶
	"{ObjDir}setsymd2.cpp.o" ¶
	"{ObjDir}setunon0.cpp.o" ¶
	"{ObjDir}setunon1.cpp.o" ¶
	"{ObjDir}setunon2.cpp.o" ¶
#	"{ObjDir}single.cpp.o" ¶
	"{ObjDir}slist1.cpp.o" ¶
	"{ObjDir}sort1.cpp.o" ¶
	"{ObjDir}sort2.cpp.o" ¶
	"{ObjDir}stack1.cpp.o" ¶
	"{ObjDir}stack2.cpp.o" ¶
#	"{ObjDir}stat.cpp.o" ¶
	"{ObjDir}stblptn0.cpp.o" ¶
	"{ObjDir}stblptn1.cpp.o" ¶
	"{ObjDir}stblsrt1.cpp.o" ¶
	"{ObjDir}stblsrt2.cpp.o" ¶
#	"{ObjDir}stl_single.cpp.o" ¶
	"{ObjDir}stl_test.cpp.o" ¶
	"{ObjDir}string1.cpp.o" ¶
	"{ObjDir}swap1.cpp.o" ¶
	"{ObjDir}swprnge1.cpp.o" ¶
	"{ObjDir}times.cpp.o" ¶
	"{ObjDir}trnsfrm1.cpp.o" ¶
	"{ObjDir}trnsfrm2.cpp.o" ¶
	"{ObjDir}ucompos1.cpp.o" ¶
	"{ObjDir}ucompos2.cpp.o" ¶
	"{ObjDir}unegate1.cpp.o" ¶
	"{ObjDir}unegate2.cpp.o" ¶
	"{ObjDir}uniqcpy1.cpp.o" ¶
	"{ObjDir}uniqcpy2.cpp.o" ¶
	"{ObjDir}unique1.cpp.o" ¶
	"{ObjDir}unique2.cpp.o" ¶
	"{ObjDir}uprbnd1.cpp.o" ¶
	"{ObjDir}uprbnd2.cpp.o" ¶
	"{ObjDir}vec1.cpp.o" ¶
	"{ObjDir}vec2.cpp.o" ¶
	"{ObjDir}vec3.cpp.o" ¶
	"{ObjDir}vec4.cpp.o" ¶
	"{ObjDir}vec5.cpp.o" ¶
	"{ObjDir}vec6.cpp.o" ¶
	"{ObjDir}vec7.cpp.o" ¶
	"{ObjDir}vec8.cpp.o" ¶
#	end


Regression_test ÄÄ setup
Regression_test ÄÄ "{ObjDir}"Regression_test.68K
	echo "¶n'{ObjDir}Regression_test.68K' < '{stl}:test:regression:stdin' # execute it"

"{ObjDir}"Regression_test.68K ÄÄ {¥MondoBuild¥} {Regression_Objects_68K} {Needed_SysLibs}
	###########
	if "{compile_status}"
		echo "### LINK NOT PERFORMED DUE TO COMPILE ERROR"
		exit "{compile_status}"
	end
	echo ¶n¶nLinking:    "{Targ}"
	ILink ¶
	#Link ¶
		-t 'MPST' ¶
		-o {Targ} ¶
		{Regression_Objects_68K} ¶
		{Link_options} ¶
		{Needed_SysLibs} ¶
		# end


##################################################################################
#	test:eh: build rule
##################################################################################
eh_Objects_68K  = ¶
	"{ObjDir}main.cpp.o" ¶
	"{ObjDir}nc_alloc.cpp.o" ¶
	"{ObjDir}random_number.cpp.o" ¶
	"{ObjDir}test_algo.cpp.o" ¶
	"{ObjDir}test_algobase.cpp.o" ¶
	"{ObjDir}test_bit_vector.cpp.o" ¶
	"{ObjDir}test_bitset.cpp.o" ¶
	"{ObjDir}test_deque.cpp.o" ¶
	"{ObjDir}test_hash_map.cpp.o" ¶
	"{ObjDir}test_hash_set.cpp.o" ¶
	"{ObjDir}test_list.cpp.o" ¶
	"{ObjDir}test_map.cpp.o" ¶
	"{ObjDir}test_rope.cpp.o" ¶
	"{ObjDir}test_set.cpp.o" ¶
	"{ObjDir}test_slist.cpp.o" ¶
	"{ObjDir}test_string.cpp.o" ¶
	"{ObjDir}test_valarray.cpp.o" ¶
	"{ObjDir}test_vector.cpp.o" ¶
	"{ObjDir}TestClass.cpp.o" ¶
	#end

EH_test ÄÄ setup
EH_test ÄÄ "{ObjDir}"EH_test.68K
	echo "¶n'{ObjDir}EH_test.68K' # execute it"


"{ObjDir}"EH_test.68K ÄÄ "{stl}:test:regression:{¥MondoBuild¥}" {eh_Objects_68K} {Needed_SysLibs}
	###########
	if "{compile_status}"
		echo "### LINK NOT PERFORMED DUE TO COMPILE TIME ERROR"
		exit "{compile_status}"
	end
	echo ¶n¶nLinking:    "{Targ}"
	ILink ¶
	#Link ¶
		-t 'MPST' ¶
		-o {Targ} ¶
		{eh_Objects_68K} ¶
		{Link_options} ¶
		{Needed_SysLibs} ¶
		# end


=======
!include "{stl}:src:SCpp.mak"



MAKEFILE     = SCpp.mak





Needed_SysLibs = ¶

#		"{CLibraries}IOStreams.o" ¶

		"{STL}:lib:STLportLib{config_}.o" ¶

		"{Libraries}Stubs.o" ¶

		"{Libraries}MathLib.o" ¶

		"{CLibraries}CPlusLib.o" ¶

		"{CLibraries}StdCLib.o" ¶

		"{Libraries}MacRuntime.o" ¶

		"{Libraries}IntEnv.o" ¶

		#"{Libraries}ToolLibs.o" ¶

		"{Libraries}Interface.o" ¶

		# end





##################################################################################

#	test:regression: build rule

##################################################################################

Regression_Objects_68K  = ¶

	"{ObjDir}accum1.cpp.o" ¶

	"{ObjDir}accum2.cpp.o" ¶

	"{ObjDir}adjdiff0.cpp.o" ¶

	"{ObjDir}adjdiff1.cpp.o" ¶

	"{ObjDir}adjdiff2.cpp.o" ¶

	"{ObjDir}adjfind0.cpp.o" ¶

	"{ObjDir}adjfind1.cpp.o" ¶

	"{ObjDir}adjfind2.cpp.o" ¶

	"{ObjDir}advance.cpp.o" ¶

	"{ObjDir}alg1.cpp.o" ¶

	"{ObjDir}alg2.cpp.o" ¶

	"{ObjDir}alg3.cpp.o" ¶

	"{ObjDir}alg4.cpp.o" ¶

	"{ObjDir}alg5.cpp.o" ¶

	"{ObjDir}bcompos1.cpp.o" ¶

	"{ObjDir}bcompos2.cpp.o" ¶

	"{ObjDir}bind1st1.cpp.o" ¶

	"{ObjDir}bind1st2.cpp.o" ¶

	"{ObjDir}bind2nd1.cpp.o" ¶

	"{ObjDir}bind2nd2.cpp.o" ¶

	"{ObjDir}binsert1.cpp.o" ¶

	"{ObjDir}binsert2.cpp.o" ¶

	"{ObjDir}binsrch1.cpp.o" ¶

	"{ObjDir}binsrch2.cpp.o" ¶

	"{ObjDir}bitset1.cpp.o" ¶

	"{ObjDir}bnegate1.cpp.o" ¶

	"{ObjDir}bnegate2.cpp.o" ¶

	"{ObjDir}bvec1.cpp.o" ¶

	"{ObjDir}copy1.cpp.o" ¶

	"{ObjDir}copy2.cpp.o" ¶

	"{ObjDir}copy3.cpp.o" ¶

	"{ObjDir}copy4.cpp.o" ¶

	"{ObjDir}copyb.cpp.o" ¶

	"{ObjDir}copyb0.cpp.o" ¶

	"{ObjDir}count0.cpp.o" ¶

	"{ObjDir}count1.cpp.o" ¶

	"{ObjDir}countif1.cpp.o" ¶

	"{ObjDir}deque1.cpp.o" ¶

	"{ObjDir}divides.cpp.o" ¶

	"{ObjDir}eqlrnge0.cpp.o" ¶

	"{ObjDir}eqlrnge1.cpp.o" ¶

	"{ObjDir}eqlrnge2.cpp.o" ¶

	"{ObjDir}equal0.cpp.o" ¶

	"{ObjDir}equal1.cpp.o" ¶

	"{ObjDir}equal2.cpp.o" ¶

	"{ObjDir}equalto.cpp.o" ¶

	"{ObjDir}fill1.cpp.o" ¶

	"{ObjDir}filln1.cpp.o" ¶

	"{ObjDir}find0.cpp.o" ¶

	"{ObjDir}find1.cpp.o" ¶

	"{ObjDir}findif0.cpp.o" ¶

	"{ObjDir}findif1.cpp.o" ¶

	"{ObjDir}finsert1.cpp.o" ¶

	"{ObjDir}finsert2.cpp.o" ¶

	"{ObjDir}foreach0.cpp.o" ¶

	"{ObjDir}foreach1.cpp.o" ¶

	"{ObjDir}func1.cpp.o" ¶

	"{ObjDir}func2.cpp.o" ¶

	"{ObjDir}func3.cpp.o" ¶

	"{ObjDir}gener1.cpp.o" ¶

	"{ObjDir}gener2.cpp.o" ¶

	"{ObjDir}genern1.cpp.o" ¶

	"{ObjDir}genern2.cpp.o" ¶

	"{ObjDir}greateq.cpp.o" ¶

	"{ObjDir}greater.cpp.o" ¶

	"{ObjDir}hmap1.cpp.o" ¶

	"{ObjDir}hmmap1.cpp.o" ¶

	"{ObjDir}hmset1.cpp.o" ¶

	"{ObjDir}hset2.cpp.o" ¶

	"{ObjDir}incl0.cpp.o" ¶

	"{ObjDir}incl1.cpp.o" ¶

	"{ObjDir}incl2.cpp.o" ¶

	"{ObjDir}inplmrg1.cpp.o" ¶

	"{ObjDir}inplmrg2.cpp.o" ¶

	"{ObjDir}inrprod0.cpp.o" ¶

	"{ObjDir}inrprod1.cpp.o" ¶

	"{ObjDir}inrprod2.cpp.o" ¶

	"{ObjDir}insert1.cpp.o" ¶

	"{ObjDir}insert2.cpp.o" ¶

	"{ObjDir}iota1.cpp.o" ¶

	"{ObjDir}istmit1.cpp.o" ¶

	"{ObjDir}iter1.cpp.o" ¶

	"{ObjDir}iter2.cpp.o" ¶

	"{ObjDir}iter3.cpp.o" ¶

	"{ObjDir}iter4.cpp.o" ¶

	"{ObjDir}iterswp0.cpp.o" ¶

	"{ObjDir}iterswp1.cpp.o" ¶

	"{ObjDir}less.cpp.o" ¶

	"{ObjDir}lesseq.cpp.o" ¶

	"{ObjDir}lexcmp1.cpp.o" ¶

	"{ObjDir}lexcmp2.cpp.o" ¶

	"{ObjDir}list1.cpp.o" ¶

	"{ObjDir}list2.cpp.o" ¶

	"{ObjDir}list3.cpp.o" ¶

	"{ObjDir}list4.cpp.o" ¶

	"{ObjDir}logicand.cpp.o" ¶

	"{ObjDir}logicnot.cpp.o" ¶

	"{ObjDir}logicor.cpp.o" ¶

	"{ObjDir}lwrbnd1.cpp.o" ¶

	"{ObjDir}lwrbnd2.cpp.o" ¶

	"{ObjDir}map1.cpp.o" ¶

	"{ObjDir}max1.cpp.o" ¶

	"{ObjDir}max2.cpp.o" ¶

	"{ObjDir}maxelem1.cpp.o" ¶

	"{ObjDir}maxelem2.cpp.o" ¶
	"{ObjDir}memfunptr.cpp.o" ¶
	"{ObjDir}merge0.cpp.o" ¶

	"{ObjDir}merge1.cpp.o" ¶

	"{ObjDir}merge2.cpp.o" ¶

	"{ObjDir}min1.cpp.o" ¶

	"{ObjDir}min2.cpp.o" ¶

	"{ObjDir}minelem1.cpp.o" ¶

	"{ObjDir}minelem2.cpp.o" ¶

	"{ObjDir}minus.cpp.o" ¶

	"{ObjDir}mismtch0.cpp.o" ¶

	"{ObjDir}mismtch1.cpp.o" ¶

	"{ObjDir}mismtch2.cpp.o" ¶

	"{ObjDir}mkheap0.cpp.o" ¶

	"{ObjDir}mkheap1.cpp.o" ¶

	"{ObjDir}mmap1.cpp.o" ¶

	"{ObjDir}mmap2.cpp.o" ¶

	"{ObjDir}modulus.cpp.o" ¶

	"{ObjDir}mset1.cpp.o" ¶

	"{ObjDir}mset3.cpp.o" ¶

	"{ObjDir}mset4.cpp.o" ¶

	"{ObjDir}mset5.cpp.o" ¶

	"{ObjDir}negate.cpp.o" ¶

	"{ObjDir}nequal.cpp.o" ¶

	"{ObjDir}nextprm0.cpp.o" ¶

	"{ObjDir}nextprm1.cpp.o" ¶

	"{ObjDir}nextprm2.cpp.o" ¶

	"{ObjDir}nthelem0.cpp.o" ¶

	"{ObjDir}nthelem1.cpp.o" ¶

	"{ObjDir}nthelem2.cpp.o" ¶

	"{ObjDir}ostmit.cpp.o" ¶

	"{ObjDir}pair0.cpp.o" ¶

	"{ObjDir}pair1.cpp.o" ¶

	"{ObjDir}pair2.cpp.o" ¶

	"{ObjDir}parsrt0.cpp.o" ¶

	"{ObjDir}parsrt1.cpp.o" ¶

	"{ObjDir}parsrt2.cpp.o" ¶

	"{ObjDir}parsrtc0.cpp.o" ¶

	"{ObjDir}parsrtc1.cpp.o" ¶

	"{ObjDir}parsrtc2.cpp.o" ¶

	"{ObjDir}partsrt0.cpp.o" ¶

	"{ObjDir}partsum0.cpp.o" ¶

	"{ObjDir}partsum1.cpp.o" ¶

	"{ObjDir}partsum2.cpp.o" ¶

	"{ObjDir}pheap1.cpp.o" ¶

	"{ObjDir}pheap2.cpp.o" ¶

	"{ObjDir}plus.cpp.o" ¶

	"{ObjDir}pqueue1.cpp.o" ¶

	"{ObjDir}prevprm0.cpp.o" ¶

	"{ObjDir}prevprm1.cpp.o" ¶

	"{ObjDir}prevprm2.cpp.o" ¶

	"{ObjDir}ptition0.cpp.o" ¶

	"{ObjDir}ptition1.cpp.o" ¶

	"{ObjDir}ptrbinf1.cpp.o" ¶

	"{ObjDir}ptrbinf2.cpp.o" ¶

	"{ObjDir}ptrunf1.cpp.o" ¶

	"{ObjDir}ptrunf2.cpp.o" ¶

	"{ObjDir}queue1.cpp.o" ¶

	"{ObjDir}rawiter.cpp.o" ¶

	"{ObjDir}remcopy1.cpp.o" ¶

	"{ObjDir}remcpif1.cpp.o" ¶

	"{ObjDir}remif1.cpp.o" ¶

	"{ObjDir}remove1.cpp.o" ¶

	"{ObjDir}repcpif1.cpp.o" ¶

	"{ObjDir}replace0.cpp.o" ¶

	"{ObjDir}replace1.cpp.o" ¶

	"{ObjDir}replcpy1.cpp.o" ¶

	"{ObjDir}replif1.cpp.o" ¶

	"{ObjDir}revbit1.cpp.o" ¶

	"{ObjDir}revbit2.cpp.o" ¶

	"{ObjDir}revcopy1.cpp.o" ¶

	"{ObjDir}reverse1.cpp.o" ¶

	"{ObjDir}reviter1.cpp.o" ¶

	"{ObjDir}reviter2.cpp.o" ¶

	"{ObjDir}rndshuf0.cpp.o" ¶

	"{ObjDir}rndshuf1.cpp.o" ¶

	"{ObjDir}rndshuf2.cpp.o" ¶

	"{ObjDir}rotate0.cpp.o" ¶

	"{ObjDir}rotate1.cpp.o" ¶

	"{ObjDir}rotcopy0.cpp.o" ¶

	"{ObjDir}rotcopy1.cpp.o" ¶

	"{ObjDir}search0.cpp.o" ¶

	"{ObjDir}search1.cpp.o" ¶

	"{ObjDir}search2.cpp.o" ¶

	"{ObjDir}set1.cpp.o" ¶

	"{ObjDir}set2.cpp.o" ¶

	"{ObjDir}setdiff0.cpp.o" ¶

	"{ObjDir}setdiff1.cpp.o" ¶

	"{ObjDir}setdiff2.cpp.o" ¶

	"{ObjDir}setintr0.cpp.o" ¶

	"{ObjDir}setintr1.cpp.o" ¶

	"{ObjDir}setintr2.cpp.o" ¶

	"{ObjDir}setsymd0.cpp.o" ¶

	"{ObjDir}setsymd1.cpp.o" ¶

	"{ObjDir}setsymd2.cpp.o" ¶

	"{ObjDir}setunon0.cpp.o" ¶

	"{ObjDir}setunon1.cpp.o" ¶

	"{ObjDir}setunon2.cpp.o" ¶

#	"{ObjDir}single.cpp.o" ¶

	"{ObjDir}slist1.cpp.o" ¶

	"{ObjDir}sort1.cpp.o" ¶

	"{ObjDir}sort2.cpp.o" ¶

	"{ObjDir}stack1.cpp.o" ¶

	"{ObjDir}stack2.cpp.o" ¶

#	"{ObjDir}stat.cpp.o" ¶

	"{ObjDir}stblptn0.cpp.o" ¶

	"{ObjDir}stblptn1.cpp.o" ¶

	"{ObjDir}stblsrt1.cpp.o" ¶

	"{ObjDir}stblsrt2.cpp.o" ¶

#	"{ObjDir}stl_single.cpp.o" ¶

	"{ObjDir}stl_test.cpp.o" ¶

	"{ObjDir}string1.cpp.o" ¶

	"{ObjDir}swap1.cpp.o" ¶

	"{ObjDir}swprnge1.cpp.o" ¶

	"{ObjDir}times.cpp.o" ¶

	"{ObjDir}trnsfrm1.cpp.o" ¶

	"{ObjDir}trnsfrm2.cpp.o" ¶

	"{ObjDir}ucompos1.cpp.o" ¶

	"{ObjDir}ucompos2.cpp.o" ¶

	"{ObjDir}unegate1.cpp.o" ¶

	"{ObjDir}unegate2.cpp.o" ¶

	"{ObjDir}uniqcpy1.cpp.o" ¶

	"{ObjDir}uniqcpy2.cpp.o" ¶

	"{ObjDir}unique1.cpp.o" ¶

	"{ObjDir}unique2.cpp.o" ¶

	"{ObjDir}uprbnd1.cpp.o" ¶

	"{ObjDir}uprbnd2.cpp.o" ¶

	"{ObjDir}vec1.cpp.o" ¶

	"{ObjDir}vec2.cpp.o" ¶

	"{ObjDir}vec3.cpp.o" ¶

	"{ObjDir}vec4.cpp.o" ¶

	"{ObjDir}vec5.cpp.o" ¶

	"{ObjDir}vec6.cpp.o" ¶

	"{ObjDir}vec7.cpp.o" ¶

	"{ObjDir}vec8.cpp.o" ¶

#	end





Regression_test ÄÄ setup

Regression_test ÄÄ "{ObjDir}"Regression_test.68K

	echo "¶n'{ObjDir}Regression_test.68K' < '{stl}:test:regression:stdin' # execute it"



"{ObjDir}"Regression_test.68K ÄÄ {¥MondoBuild¥} {Regression_Objects_68K} {Needed_SysLibs}

	###########

	if "{compile_status}"

		echo "### LINK NOT PERFORMED DUE TO COMPILE ERROR"

		exit "{compile_status}"

	end

	echo ¶n¶nLinking:    "{Targ}"

	ILink ¶

	#Link ¶

		-t 'MPST' ¶

		-o {Targ} ¶

		{Regression_Objects_68K} ¶

		{Link_options} ¶

		{Needed_SysLibs} ¶

		# end





##################################################################################

#	test:eh: build rule

##################################################################################

eh_Objects_68K  = ¶

	"{ObjDir}main.cpp.o" ¶

	"{ObjDir}nc_alloc.cpp.o" ¶

	"{ObjDir}random_number.cpp.o" ¶

	"{ObjDir}test_algo.cpp.o" ¶

	"{ObjDir}test_algobase.cpp.o" ¶

	"{ObjDir}test_bit_vector.cpp.o" ¶

	"{ObjDir}test_bitset.cpp.o" ¶

	"{ObjDir}test_deque.cpp.o" ¶

	"{ObjDir}test_hash_map.cpp.o" ¶

	"{ObjDir}test_hash_set.cpp.o" ¶

	"{ObjDir}test_list.cpp.o" ¶

	"{ObjDir}test_map.cpp.o" ¶

	"{ObjDir}test_rope.cpp.o" ¶

	"{ObjDir}test_set.cpp.o" ¶

	"{ObjDir}test_slist.cpp.o" ¶

	"{ObjDir}test_string.cpp.o" ¶

	"{ObjDir}test_valarray.cpp.o" ¶

	"{ObjDir}test_vector.cpp.o" ¶

	"{ObjDir}TestClass.cpp.o" ¶

	#end



EH_test ÄÄ setup

EH_test ÄÄ "{ObjDir}"EH_test.68K

	echo "¶n'{ObjDir}EH_test.68K' # execute it"





"{ObjDir}"EH_test.68K ÄÄ "{stl}:test:regression:{¥MondoBuild¥}" {eh_Objects_68K} {Needed_SysLibs}

	###########

	if "{compile_status}"

		echo "### LINK NOT PERFORMED DUE TO COMPILE TIME ERROR"

		exit "{compile_status}"

	end

	echo ¶n¶nLinking:    "{Targ}"

	ILink ¶

	#Link ¶

		-t 'MPST' ¶

		-o {Targ} ¶

		{eh_Objects_68K} ¶

		{Link_options} ¶

		{Needed_SysLibs} ¶

		# end






