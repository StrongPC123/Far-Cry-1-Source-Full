.AUTODEPEND


#
# Borland C++ tools
#

# set this variables to your locations
BCROOT = D:\BC5
STLINCL = ..\..\STLPORT\OLD_HP;..\..\STLPORT

IMPLIB  = Implib
BCC=bcc32 +BccW32.cfg
BCC32   = Bcc32 +BccW32.cfg 
BCC32I  = Bcc32i +BccW32.cfg 
TLINK32 = TLink32
TLIB    = TLib
BRC32   = Brc32
TASM32  = Tasm32

#
# Options
#

IDE_LinkFLAGS32 =  -L${BCROOT}\LIB
LinkerLocalOptsAtC32_osdexe =  -Tpe -ap -c
ResLocalOptsAtC32_osdexe = 
BLocalOptsAtC32_osdexe = 
CompInheritOptsAt_osdexe = -I.;${STLINCL};${BCROOT}\INCLUDE -D_RTLDLL -D_STLP_NO_OWN_IOSTREAMS;
LinkerInheritOptsAt_osdexe = -x
LinkerOptsAt_osdexe = $(LinkerLocalOptsAtC32_osdexe)
ResOptsAt_osdexe = $(ResLocalOptsAtC32_osdexe)
BOptsAt_osdexe = $(BLocalOptsAtC32_osdexe)

#
# Dependency List
#
Dep_os = stl_test.exe

check: $(Dep_os)
	stl_test.exe < stdin > stl_test.out

Dep_osdexe = \
   vec2.obj\
   vec7.obj\
   vec6.obj\
   vec5.obj\
   vec4.obj\
   vec3.obj\
   vec8.obj\
   stat.obj\
   uprbnd2.obj\
   uprbnd1.obj\
   unique2.obj\
   unique1.obj\
   uniqcpy2.obj\
   uniqcpy1.obj\
   unegate2.obj\
   unegate1.obj\
   ucompos2.obj\
   ucompos1.obj\
   trnsfrm2.obj\
   trnsfrm1.obj\
   times.obj\
   swprnge1.obj\
   swap1.obj\
   stl_test.obj\
   stblsrt2.obj\
   stblsrt1.obj\
   stblptn1.obj\
   stblptn0.obj\
   vec1.obj\
   rotcopy0.obj\
   stack1.obj\
   sort2.obj\
   sort1.obj\
   setunon2.obj\
   setunon1.obj\
   setunon0.obj\
   setsymd2.obj\
   setsymd1.obj\
   setsymd0.obj\
   setintr2.obj\
   setintr1.obj\
   setintr0.obj\
   setdiff2.obj\
   setdiff1.obj\
   setdiff0.obj\
   set2.obj\
   set1.obj\
   search2.obj\
   search1.obj\
   search0.obj\
   rotcopy1.obj\
   stack2.obj\
   ptrunf1.obj\
   rotate0.obj\
   rndshuf2.obj\
   rndshuf1.obj\
   rndshuf0.obj\
   reviter2.obj\
   reviter1.obj\
   reverse1.obj\
   revcopy1.obj\
   revbit2.obj\
   revbit1.obj\
   replif1.obj\
   replcpy1.obj\
   replace1.obj\
   replace0.obj\
   repcpif1.obj\
   remove1.obj\
   remif1.obj\
   remcpif1.obj\
   remcopy1.obj\
   rawiter.obj\
   queue1.obj\
   ptrunf2.obj\
   rotate1.obj\
   pair0.obj\
   ptrbinf1.obj\
   ptition1.obj\
   ptition0.obj\
   prevprm2.obj\
   prevprm1.obj\
   prevprm0.obj\
   pqueue1.obj\
   plus.obj\
   pheap2.obj\
   pheap1.obj\
   partsum2.obj\
   partsum1.obj\
   partsum0.obj\
   partsrt0.obj\
   parsrtc2.obj\
   parsrtc1.obj\
   parsrtc0.obj\
   parsrt2.obj\
   parsrt1.obj\
   parsrt0.obj\
   pair2.obj\
   pair1.obj\
   ptrbinf2.obj\
   minelem1.obj\
   nthelem2.obj\
   nthelem1.obj\
   nthelem0.obj\
   nextprm2.obj\
   nextprm1.obj\
   nextprm0.obj\
   nequal.obj\
   negate.obj\
   mset5.obj\
   mset4.obj\
   mset3.obj\
   mset1.obj\
   modulus.obj\
   mmap2.obj\
   mmap1.obj\
   mkheap1.obj\
   mkheap0.obj\
   mismtch2.obj\
   mismtch1.obj\
   mismtch0.obj\
   minus.obj\
   minelem2.obj\
   ostmit.obj\
   iterswp1.obj\
   min1.obj\
   memfunptr.obj \
   merge2.obj\
   merge1.obj\
   merge0.obj\
   maxelem2.obj\
   maxelem1.obj\
   max2.obj\
   max1.obj\
   map1.obj\
   lwrbnd2.obj\
   lwrbnd1.obj\
   logicor.obj\
   logicnot.obj\
   logicand.obj\
   list4.obj\
   list3.obj\
   list2.obj\
   list1.obj\
   lexcmp2.obj\
   lexcmp1.obj\
   lesseq.obj\
   less.obj\
   min2.obj\
   greater.obj\
   iter4.obj\
   iter3.obj\
   iter2.obj\
   iter1.obj\
   istmit1.obj\
   iota1.obj\
   insert2.obj\
   insert1.obj\
   inrprod2.obj\
   inrprod1.obj\
   inrprod0.obj\
   inplmrg2.obj\
   inplmrg1.obj\
   incl2.obj\
   incl1.obj\
   incl0.obj\
   iterswp0.obj\
   eqlrnge2.obj\
   genern2.obj\
   genern1.obj\
   gener2.obj\
   gener1.obj\
   func3.obj\
   func2.obj\
   func1.obj\
   foreach1.obj\
   foreach0.obj\
   finsert2.obj\
   finsert1.obj\
   findif1.obj\
   findif0.obj\
   find1.obj\
   find0.obj\
   filln1.obj\
   fill1.obj\
   equalto.obj\
   equal2.obj\
   equal1.obj\
   equal0.obj\
   greateq.obj\
   binsrch1.obj\
   eqlrnge0.obj\
   divides.obj\
   deque1.obj\
   countif1.obj\
   count1.obj\
   count0.obj\
   copyb0.obj\
   copyb.obj\
   copy4.obj\
   copy3.obj\
   copy2.obj\
   copy1.obj\
   bvec1.obj\
   bnegate2.obj\
   bnegate1.obj\
   binsrch2.obj\
   eqlrnge1.obj\
   accum1.obj\
   binsert1.obj\
   bind2nd2.obj\
   bind2nd1.obj\
   bind1st1.obj\
   bind1st2.obj\
   bcompos2.obj\
   bcompos1.obj\
   alg5.obj\
   alg4.obj\
   alg3.obj\
   alg2.obj\
   alg1.obj\
   advance.obj\
   adjfind2.obj\
   adjfind1.obj\
   adjfind0.obj\
   adjdiff2.obj\
   adjdiff1.obj\
   adjdiff0.obj\
   accum2.obj\
   binsert2.obj\
   hmap1.obj hmmap1.obj hset2.obj hmset1.obj \
   os.def

stl_test.exe : $(Dep_osdexe)
  $(TLINK32) @&&|
 /v $(IDE_LinkFLAGS32) $(LinkerOptsAt_osdexe) $(LinkerInheritOptsAt_osdexe) +
${BCROOT}\LIB\c0x32.obj+
vec2.obj+
vec7.obj+
vec6.obj+
vec5.obj+
vec4.obj+
vec3.obj+
vec8.obj+
stat.obj+
uprbnd2.obj+
uprbnd1.obj+
unique2.obj+
unique1.obj+
uniqcpy2.obj+
uniqcpy1.obj+
unegate2.obj+
unegate1.obj+
ucompos2.obj+
ucompos1.obj+
trnsfrm2.obj+
trnsfrm1.obj+
times.obj+
swprnge1.obj+
swap1.obj+
stl_test.obj+
stblsrt2.obj+
stblsrt1.obj+
stblptn1.obj+
stblptn0.obj+
vec1.obj+
rotcopy0.obj+
stack1.obj+
sort2.obj+
sort1.obj+
setunon2.obj+
setunon1.obj+
setunon0.obj+
setsymd2.obj+
setsymd1.obj+
setsymd0.obj+
setintr2.obj+
setintr1.obj+
setintr0.obj+
setdiff2.obj+
setdiff1.obj+
setdiff0.obj+
set2.obj+
set1.obj+
search2.obj+
search1.obj+
search0.obj+
rotcopy1.obj+
stack2.obj+
ptrunf1.obj+
rotate0.obj+
rndshuf2.obj+
rndshuf1.obj+
rndshuf0.obj+
reviter2.obj+
reviter1.obj+
reverse1.obj+
revcopy1.obj+
revbit2.obj+
revbit1.obj+
replif1.obj+
replcpy1.obj+
replace1.obj+
replace0.obj+
repcpif1.obj+
remove1.obj+
remif1.obj+
remcpif1.obj+
remcopy1.obj+
rawiter.obj+
queue1.obj+
ptrunf2.obj+
rotate1.obj+
pair0.obj+
ptrbinf1.obj+
ptition1.obj+
ptition0.obj+
prevprm2.obj+
prevprm1.obj+
prevprm0.obj+
pqueue1.obj+
plus.obj+
pheap2.obj+
pheap1.obj+
partsum2.obj+
partsum1.obj+
partsum0.obj+
partsrt0.obj+
parsrtc2.obj+
parsrtc1.obj+
parsrtc0.obj+
parsrt2.obj+
parsrt1.obj+
parsrt0.obj+
pair2.obj+
pair1.obj+
ptrbinf2.obj+
minelem1.obj+
nthelem2.obj+
nthelem1.obj+
nthelem0.obj+
nextprm2.obj+
nextprm1.obj+
nextprm0.obj+
nequal.obj+
negate.obj+
mset5.obj+
mset4.obj+
mset3.obj+
mset1.obj+
modulus.obj+
mmap2.obj+
mmap1.obj+
mkheap1.obj+
mkheap0.obj+
mismtch2.obj+
mismtch1.obj+
mismtch0.obj+
minus.obj+
minelem2.obj+
ostmit.obj+
iterswp1.obj+
min1.obj+
merge2.obj+
merge1.obj+
merge0.obj+
maxelem2.obj+
maxelem1.obj+
max2.obj+
max1.obj+
map1.obj+
lwrbnd2.obj+
lwrbnd1.obj+
logicor.obj+
logicnot.obj+
logicand.obj+
list4.obj+
list3.obj+
list2.obj+
list1.obj+
lexcmp2.obj+
lexcmp1.obj+
lesseq.obj+
less.obj+
min2.obj+
greater.obj+
iter4.obj+
iter3.obj+
iter2.obj+
iter1.obj+
istmit1.obj+
iota1.obj+
insert2.obj+
insert1.obj+
inrprod2.obj+
inrprod1.obj+
inrprod0.obj+
inplmrg2.obj+
inplmrg1.obj+
incl2.obj+
incl1.obj+
incl0.obj+
iterswp0.obj+
eqlrnge2.obj+
genern2.obj+
genern1.obj+
gener2.obj+
gener1.obj+
func3.obj+
func2.obj+
func1.obj+
foreach1.obj+
foreach0.obj+
finsert2.obj+
finsert1.obj+
findif1.obj+
findif0.obj+
find1.obj+
find0.obj+
filln1.obj+
fill1.obj+
equalto.obj+
equal2.obj+
equal1.obj+
equal0.obj+
greateq.obj+
binsrch1.obj+
eqlrnge0.obj+
divides.obj+
deque1.obj+
countif1.obj+
count1.obj+
count0.obj+
copyb0.obj+
copyb.obj+
copy4.obj+
copy3.obj+
copy2.obj+
copy1.obj+
bvec1.obj+
bnegate2.obj+
bnegate1.obj+
binsrch2.obj+
eqlrnge1.obj+
accum1.obj+
binsert1.obj+
bind2nd2.obj+
bind2nd1.obj+
bind1st1.obj+
bind1st2.obj+
bcompos2.obj+
bcompos1.obj+
alg5.obj+
alg4.obj+
alg3.obj+
alg2.obj+
alg1.obj+
advance.obj+
adjfind2.obj+
adjfind1.obj+
adjfind0.obj+
adjdiff2.obj+
adjdiff1.obj+
adjdiff0.obj+
accum2.obj+
hmap1.obj+
hmmap1.obj+
hset2.obj+
hmset1.obj+
binsert2.obj
$<,$*
${BCROOT}\LIB\import32.lib+
${BCROOT}\LIB\cw32i.lib
stl_test.def

|

hmset1.obj :  hmset1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ hmset1.cpp
|

hset2.obj :  hset2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ hset2.cpp
|

hmmap1.obj :  hmmap1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ hmmap1.cpp
|

hmap1.obj :  hmap1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ hmap1.cpp
|

slist1.obj :  slist1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ slist1.cpp
|

vec2.obj :  vec2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ vec2.cpp
|

vec7.obj :  vec7.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ vec7.cpp
|

vec6.obj :  vec6.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ vec6.cpp
|

vec5.obj :  vec5.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ vec5.cpp
|

vec4.obj :  vec4.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ vec4.cpp
|

vec3.obj :  vec3.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ vec3.cpp
|

vec8.obj :  vec8.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ vec8.cpp
|

stat.obj :  stat.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ stat.cpp
|

uprbnd2.obj :  uprbnd2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ uprbnd2.cpp
|

uprbnd1.obj :  uprbnd1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ uprbnd1.cpp
|

unique2.obj :  unique2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ unique2.cpp
|

unique1.obj :  unique1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ unique1.cpp
|

uniqcpy2.obj :  uniqcpy2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ uniqcpy2.cpp
|

uniqcpy1.obj :  uniqcpy1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ uniqcpy1.cpp
|

unegate2.obj :  unegate2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ unegate2.cpp
|

unegate1.obj :  unegate1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ unegate1.cpp
|

ucompos2.obj :  ucompos2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ ucompos2.cpp
|

ucompos1.obj :  ucompos1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ ucompos1.cpp
|

trnsfrm2.obj :  trnsfrm2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ trnsfrm2.cpp
|

trnsfrm1.obj :  trnsfrm1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ trnsfrm1.cpp
|

times.obj :  times.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ times.cpp
|

swprnge1.obj :  swprnge1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ swprnge1.cpp
|

swap1.obj :  swap1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ swap1.cpp
|

stl_test.obj :  stl_test.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ stl_test.cpp
|

stblsrt2.obj :  stblsrt2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ stblsrt2.cpp
|

stblsrt1.obj :  stblsrt1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ stblsrt1.cpp
|

stblptn1.obj :  stblptn1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ stblptn1.cpp
|

stblptn0.obj :  stblptn0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ stblptn0.cpp
|

vec1.obj :  vec1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ vec1.cpp
|

rotcopy0.obj :  rotcopy0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ rotcopy0.cpp
|

stack1.obj :  stack1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ stack1.cpp
|

sort2.obj :  sort2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ sort2.cpp
|

sort1.obj :  sort1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ sort1.cpp
|

setunon2.obj :  setunon2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ setunon2.cpp
|

setunon1.obj :  setunon1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ setunon1.cpp
|

setunon0.obj :  setunon0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ setunon0.cpp
|

setsymd2.obj :  setsymd2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ setsymd2.cpp
|

setsymd1.obj :  setsymd1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ setsymd1.cpp
|

setsymd0.obj :  setsymd0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ setsymd0.cpp
|

setintr2.obj :  setintr2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ setintr2.cpp
|

setintr1.obj :  setintr1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ setintr1.cpp
|

setintr0.obj :  setintr0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ setintr0.cpp
|

setdiff2.obj :  setdiff2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ setdiff2.cpp
|

setdiff1.obj :  setdiff1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ setdiff1.cpp
|

setdiff0.obj :  setdiff0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ setdiff0.cpp
|

set2.obj :  set2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ set2.cpp
|

set1.obj :  set1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ set1.cpp
|

search2.obj :  search2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ search2.cpp
|

search1.obj :  search1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ search1.cpp
|

search0.obj :  search0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ search0.cpp
|

rotcopy1.obj :  rotcopy1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ rotcopy1.cpp
|

stack2.obj :  stack2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ stack2.cpp
|

ptrunf1.obj :  ptrunf1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ ptrunf1.cpp
|

rotate0.obj :  rotate0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ rotate0.cpp
|

rndshuf2.obj :  rndshuf2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ rndshuf2.cpp
|

rndshuf1.obj :  rndshuf1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ rndshuf1.cpp
|

rndshuf0.obj :  rndshuf0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ rndshuf0.cpp
|

reviter2.obj :  reviter2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ reviter2.cpp
|

reviter1.obj :  reviter1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ reviter1.cpp
|

reverse1.obj :  reverse1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ reverse1.cpp
|

revcopy1.obj :  revcopy1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ revcopy1.cpp
|

revbit2.obj :  revbit2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ revbit2.cpp
|

revbit1.obj :  revbit1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ revbit1.cpp
|

replif1.obj :  replif1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ replif1.cpp
|

replcpy1.obj :  replcpy1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ replcpy1.cpp
|

replace1.obj :  replace1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ replace1.cpp
|

replace0.obj :  replace0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ replace0.cpp
|

repcpif1.obj :  repcpif1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ repcpif1.cpp
|

remove1.obj :  remove1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ remove1.cpp
|

remif1.obj :  remif1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ remif1.cpp
|

remcpif1.obj :  remcpif1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ remcpif1.cpp
|

remcopy1.obj :  remcopy1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ remcopy1.cpp
|

rawiter.obj :  rawiter.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ rawiter.cpp
|

queue1.obj :  queue1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ queue1.cpp
|

ptrunf2.obj :  ptrunf2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ ptrunf2.cpp
|

rotate1.obj :  rotate1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ rotate1.cpp
|

pair0.obj :  pair0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ pair0.cpp
|

ptrbinf1.obj :  ptrbinf1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ ptrbinf1.cpp
|

ptition1.obj :  ptition1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ ptition1.cpp
|

ptition0.obj :  ptition0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ ptition0.cpp
|

prevprm2.obj :  prevprm2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ prevprm2.cpp
|

prevprm1.obj :  prevprm1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ prevprm1.cpp
|

prevprm0.obj :  prevprm0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ prevprm0.cpp
|

pqueue1.obj :  pqueue1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ pqueue1.cpp
|

plus.obj :  plus.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ plus.cpp
|

pheap2.obj :  pheap2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ pheap2.cpp
|

pheap1.obj :  pheap1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ pheap1.cpp
|

partsum2.obj :  partsum2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ partsum2.cpp
|

partsum1.obj :  partsum1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ partsum1.cpp
|

partsum0.obj :  partsum0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ partsum0.cpp
|

partsrt0.obj :  partsrt0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ partsrt0.cpp
|

parsrtc2.obj :  parsrtc2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ parsrtc2.cpp
|

parsrtc1.obj :  parsrtc1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ parsrtc1.cpp
|

parsrtc0.obj :  parsrtc0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ parsrtc0.cpp
|

parsrt2.obj :  parsrt2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ parsrt2.cpp
|

parsrt1.obj :  parsrt1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ parsrt1.cpp
|

parsrt0.obj :  parsrt0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ parsrt0.cpp
|

pair2.obj :  pair2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ pair2.cpp
|

pair1.obj :  pair1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ pair1.cpp
|

ptrbinf2.obj :  ptrbinf2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ ptrbinf2.cpp
|

minelem1.obj :  minelem1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ minelem1.cpp
|

nthelem2.obj :  nthelem2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ nthelem2.cpp
|

nthelem1.obj :  nthelem1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ nthelem1.cpp
|

nthelem0.obj :  nthelem0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ nthelem0.cpp
|

nextprm2.obj :  nextprm2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ nextprm2.cpp
|

nextprm1.obj :  nextprm1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ nextprm1.cpp
|

nextprm0.obj :  nextprm0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ nextprm0.cpp
|

nequal.obj :  nequal.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ nequal.cpp
|

negate.obj :  negate.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ negate.cpp
|

mset5.obj :  mset5.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ mset5.cpp
|

mset4.obj :  mset4.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ mset4.cpp
|

mset3.obj :  mset3.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ mset3.cpp
|

mset1.obj :  mset1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ mset1.cpp
|

modulus.obj :  modulus.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ modulus.cpp
|

mmap2.obj :  mmap2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ mmap2.cpp
|

mmap1.obj :  mmap1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ mmap1.cpp
|

mkheap1.obj :  mkheap1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ mkheap1.cpp
|

mkheap0.obj :  mkheap0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ mkheap0.cpp
|

mismtch2.obj :  mismtch2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ mismtch2.cpp
|

mismtch1.obj :  mismtch1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ mismtch1.cpp
|

mismtch0.obj :  mismtch0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ mismtch0.cpp
|

minus.obj :  minus.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ minus.cpp
|

minelem2.obj :  minelem2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ minelem2.cpp
|

ostmit.obj :  ostmit.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ ostmit.cpp
|

iterswp1.obj :  iterswp1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ iterswp1.cpp
|

min1.obj :  min1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ min1.cpp
|

merge2.obj :  merge2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ merge2.cpp
|

merge1.obj :  merge1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ merge1.cpp
|

merge0.obj :  merge0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ merge0.cpp
|

maxelem2.obj :  maxelem2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ maxelem2.cpp
|

maxelem1.obj :  maxelem1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ maxelem1.cpp
|

max2.obj :  max2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ max2.cpp
|

max1.obj :  max1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ max1.cpp
|

map1.obj :  map1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ map1.cpp
|

lwrbnd2.obj :  lwrbnd2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ lwrbnd2.cpp
|

lwrbnd1.obj :  lwrbnd1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ lwrbnd1.cpp
|

logicor.obj :  logicor.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ logicor.cpp
|

logicnot.obj :  logicnot.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ logicnot.cpp
|

logicand.obj :  logicand.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ logicand.cpp
|

list4.obj :  list4.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ list4.cpp
|

list3.obj :  list3.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ list3.cpp
|

list2.obj :  list2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ list2.cpp
|

list1.obj :  list1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ list1.cpp
|

lexcmp2.obj :  lexcmp2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ lexcmp2.cpp
|

lexcmp1.obj :  lexcmp1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ lexcmp1.cpp
|

lesseq.obj :  lesseq.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ lesseq.cpp
|

less.obj :  less.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ less.cpp
|

min2.obj :  min2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ min2.cpp
|

greater.obj :  greater.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ greater.cpp
|

iter4.obj :  iter4.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ iter4.cpp
|

iter3.obj :  iter3.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ iter3.cpp
|

iter2.obj :  iter2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ iter2.cpp
|

iter1.obj :  iter1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ iter1.cpp
|

istmit1.obj :  istmit1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ istmit1.cpp
|

iota1.obj :  iota1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ iota1.cpp
|

insert2.obj :  insert2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ insert2.cpp
|

insert1.obj :  insert1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ insert1.cpp
|

inrprod2.obj :  inrprod2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ inrprod2.cpp
|

inrprod1.obj :  inrprod1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ inrprod1.cpp
|

inrprod0.obj :  inrprod0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ inrprod0.cpp
|

inplmrg2.obj :  inplmrg2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ inplmrg2.cpp
|

inplmrg1.obj :  inplmrg1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ inplmrg1.cpp
|

incl2.obj :  incl2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ incl2.cpp
|

incl1.obj :  incl1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ incl1.cpp
|

incl0.obj :  incl0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ incl0.cpp
|

iterswp0.obj :  iterswp0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ iterswp0.cpp
|

eqlrnge2.obj :  eqlrnge2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ eqlrnge2.cpp
|

genern2.obj :  genern2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ genern2.cpp
|

genern1.obj :  genern1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ genern1.cpp
|

gener2.obj :  gener2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ gener2.cpp
|

gener1.obj :  gener1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ gener1.cpp
|

func3.obj :  func3.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ func3.cpp
|

func2.obj :  func2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ func2.cpp
|

func1.obj :  func1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ func1.cpp
|

foreach1.obj :  foreach1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ foreach1.cpp
|

foreach0.obj :  foreach0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ foreach0.cpp
|

finsert2.obj :  finsert2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ finsert2.cpp
|

finsert1.obj :  finsert1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ finsert1.cpp
|

findif1.obj :  findif1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ findif1.cpp
|

findif0.obj :  findif0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ findif0.cpp
|

find1.obj :  find1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ find1.cpp
|

find0.obj :  find0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ find0.cpp
|

filln1.obj :  filln1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ filln1.cpp
|

fill1.obj :  fill1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ fill1.cpp
|

equalto.obj :  equalto.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ equalto.cpp
|

equal2.obj :  equal2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ equal2.cpp
|

equal1.obj :  equal1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ equal1.cpp
|

equal0.obj :  equal0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ equal0.cpp
|

greateq.obj :  greateq.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ greateq.cpp
|

binsrch1.obj :  binsrch1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ binsrch1.cpp
|

eqlrnge0.obj :  eqlrnge0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ eqlrnge0.cpp
|

divides.obj :  divides.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ divides.cpp
|

deque1.obj :  deque1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ deque1.cpp
|

countif1.obj :  countif1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ countif1.cpp
|

count1.obj :  count1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ count1.cpp
|

count0.obj :  count0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ count0.cpp
|

copyb0.obj :  copyb0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ copyb0.cpp
|

copyb.obj :  copyb.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ copyb.cpp
|

copy4.obj :  copy4.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ copy4.cpp
|

copy3.obj :  copy3.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ copy3.cpp
|

copy2.obj :  copy2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ copy2.cpp
|

copy1.obj :  copy1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ copy1.cpp
|

bvec1.obj :  bvec1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ bvec1.cpp
|

bnegate2.obj :  bnegate2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ bnegate2.cpp
|

bnegate1.obj :  bnegate1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ bnegate1.cpp
|

binsrch2.obj :  binsrch2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ binsrch2.cpp
|

eqlrnge1.obj :  eqlrnge1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ eqlrnge1.cpp
|

accum1.obj :  accum1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ accum1.cpp
|

binsert1.obj :  binsert1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ binsert1.cpp
|

bind2nd2.obj :  bind2nd2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ bind2nd2.cpp
|

bind2nd1.obj :  bind2nd1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ bind2nd1.cpp
|

bind1st1.obj :  bind1st1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ bind1st1.cpp
|

bind1st2.obj :  bind1st2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ bind1st2.cpp
|

bcompos2.obj :  bcompos2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ bcompos2.cpp
|

bcompos1.obj :  bcompos1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ bcompos1.cpp
|

alg5.obj :  alg5.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ alg5.cpp
|

alg4.obj :  alg4.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ alg4.cpp
|

alg3.obj :  alg3.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ alg3.cpp
|

alg2.obj :  alg2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ alg2.cpp
|

alg1.obj :  alg1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ alg1.cpp
|

advance.obj :  advance.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ advance.cpp
|

adjfind2.obj :  adjfind2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ adjfind2.cpp
|

adjfind1.obj :  adjfind1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ adjfind1.cpp
|

adjfind0.obj :  adjfind0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ adjfind0.cpp
|

adjdiff2.obj :  adjdiff2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ adjdiff2.cpp
|

adjdiff1.obj :  adjdiff1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ adjdiff1.cpp
|

adjdiff0.obj :  adjdiff0.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ adjdiff0.cpp
|

accum2.obj :  accum2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ accum2.cpp
|

binsert2.obj :  binsert2.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ binsert2.cpp
|

string1.obj :  string1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ string1.cpp
|

bitset1.obj :  bitset1.cpp
  $(BCC32) -c @&&|
 $(CompOptsAt_osdexe) $(CompInheritOptsAt_osdexe) -o$@ bitset1.cpp
|

# Compiler configuration file
BccW32.cfg : 
   Copy &&|
-w
-R
-v
-vi
-H
-WC
-g255
-H-
| $@


