# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103


!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
RSC=rc.exe
CPP=cl.exe
F90=fl32.exe

OUTDIR=.
INTDIR=.

# set this directories 
STL_INCL=../../stlport
VC_INCL=.
# d:/vc41/msdev/include

Dep_stl = TestClass.obj main.obj nc_alloc.obj \
random_number.obj test_algo.obj test_algobase.obj test_bit_vector.obj test_deque.obj \
test_hash_map.obj test_hash_set.obj test_list.obj test_map.obj test_rope.obj test_set.obj \
test_slist.obj test_vector.obj test_string.obj test_bitset.obj test_valarray.obj

LINK32=link.exe

CPP_PROJ=/nologo /Gr /MTd /W3 /GX /GR /D "WIN32" /D "_CONSOLE" /I$(STL_INCL) /I. /D "_STLP_DEBUG"
# CPP_PROJ=/nologo /MDd /W3 /GX /GR /D "WIN32" /D "_CONSOLE" /I$(STL_INCL) /I.

# linker finds proper STLport lib automatically, only path to the
# library is needed
CPP_LIBS = /link /libpath:"..\..\lib"

check: eh_test.exe
#  fbp : this is to locate DLL
	cd ..\..\lib
	..\test\eh\eh_test.exe -s 100
	echo done

eh_test.exe : $(Dep_stl)
	$(CC) $(CPP_PROJ) $(Dep_stl) -o eh_test.exe $(CPP_LIBS)

clean :
	-@erase "$(INTDIR)\*.obj"
	-@erase "$(OUTDIR)\*.exe"
	-@erase "$(OUTDIR)\*.obj"


.exe.out:
	$< > $@

.cpp.exe:
  $(CPP) $(CPP_PROJ) -DMAIN $<

.c.obj:
   $(CPP) $(CPP_PROJ) /c $<

.cpp.obj:
   $(CPP) $(CPP_PROJ) /c $<

.cxx.obj:
   $(CPP) $(CPP_PROJ) /c $<

.cpp.E:
   $(CPP) $(CPP_PROJ) -E $< >$*.E  

.cpp.sbr:
   $(CPP) $(CPP_PROJ) $<  
