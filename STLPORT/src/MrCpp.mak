MAKEFILE        = MrCpp.mak
¥MondoBuild¥    = {MAKEFILE}  # Make blank to avoid rebuilds when makefile is modified

Sym-PPC         	= -sym on
!ifdef DEBUG
config_				= .PPC.DBG
MrCpp_DebugOptions 	= -inline none,global -opt none {Sym-PPC}
STL_DebugOptions 	= -d _STLP_DEBUG -d _STLP_DEBUG_ALLOC -d _STLP_DEBUG_UNINITIALIZED
!else
config_				= .PPC
MrCpp_DebugOptions 	= 
STL_DebugOptions 	= 
!endif
ObjDir          	= :{config_}:
SrcDir				= :

Includes     	= -i : -i "{STL}" -i "{CIncludes}"

MrCpp_Options 	= -ansi on -ansifor -bool on -exceptions on -rtti on -align power -j0 -traceback -opt size -inline 3,global -includes unix_mac
				
STL_Options  	= 	#-d _STLP_USE_NEWALLOC ¶
					#-d _STLP_NO_SGI_IOSTREAMS ¶
					# end

Link_options = ¶
		-c 'MPS ' ¶
		-mf ¶
		-d ¶
		-Linkfaster off ¶
		#{Sym-PPC} ¶
		# end

### Default Rules ###

"{ObjDir}" Ä "{SrcDir}"

.cpp.x  Ä  .cpp  # {¥MondoBuild¥}
	###
	echo "¶nCompiling:      '"{depDir}{default}.cpp"'"
	"{MrCpp}" {depDir}{default}.cpp ¶
		-o {targDir}{default}.cpp.x ¶
		{Includes} ¶
		{MrCpp_Options} {MrCpp_DebugOptions} {other_MrCpp_Options} ¶
		{STL_Options} {STL_DebugOptions} {other_STL_Options}
		if "{status}"
			set compile_status 1
		end


### Optional Dependencies ###

setup Ä $OutOfDate
	###
	echo "¶n# Target:       '"{ObjDir}"'"
	unset compile_status
	if !`exists "{ObjDir}"`
		newfolder "{ObjDir}"
	end


### Build this target to generate "include file" dependencies. ###

Dependencies  Ä  $OutOfDate #*TY 02/26/2000 - MakeDepend does not work unless all mentioned include directory exists
	###
	echo "¶nUpdating:       {MAKEFILE} Dependencies"
	MakeDepend ¶
		-append {MAKEFILE} ¶
		-ignore "{CIncludes}" ¶
		-objdir "{ObjDir}" ¶
		-objext .x ¶
		{Includes} ¶
		{SrcFiles}


##################################################################################
#	{stl}:src: build rule
##################################################################################

### Source Files ###

STLportLibSrcFiles        =  ¶
				c_locale_stub.cpp	¶
				codecvt.cpp	¶
				collate.cpp	¶
				complex.cpp	¶
				complex_exp.cpp	¶
				complex_io.cpp	¶
				complex_io_w.cpp	¶
				complex_trig.cpp	¶
				ctype.cpp	¶
				dll_main.cpp	¶
				fstream.cpp	¶
				ios.cpp	¶
				iostream.cpp	¶
				istream.cpp	¶
				locale.cpp	¶
				locale_catalog.cpp	¶
				facets_byname.cpp	¶
				locale_impl.cpp	¶
				messages.cpp	¶
				monetary.cpp	¶
				num_get.cpp	¶
				num_get_float.cpp	¶
				num_put.cpp	¶
				num_put_float.cpp	¶
				numpunct.cpp	¶
				ostream.cpp	¶
				sstream.cpp	¶
				stdio_streambuf.cpp	¶
				streambuf.cpp	¶
				string_w.cpp	¶
				strstream.cpp	¶
				time_facets.cpp	¶
				# end		#*TY 11/25/2000 - updated for STLport.4.1


### Object Files ###

STLportLibObjFiles-PPC    =  ¶
				{ObjDir}c_locale_stub.cpp.x	¶
				{ObjDir}codecvt.cpp.x	¶
				{ObjDir}collate.cpp.x	¶
				{ObjDir}complex.cpp.x	¶
				{ObjDir}complex_exp.cpp.x	¶
				{ObjDir}complex_io.cpp.x	¶
				{ObjDir}complex_io_w.cpp.x	¶
				{ObjDir}complex_trig.cpp.x	¶
				{ObjDir}ctype.cpp.x	¶
				{ObjDir}dll_main.cpp.x	¶
				{ObjDir}fstream.cpp.x	¶
				{ObjDir}ios.cpp.x	¶
				{ObjDir}iostream.cpp.x	¶
				{ObjDir}istream.cpp.x	¶
				{ObjDir}locale.cpp.x	¶
				{ObjDir}locale_catalog.cpp.x	¶
				{ObjDir}facets_byname.cpp.x	¶
				{ObjDir}locale_impl.cpp.x	¶
				{ObjDir}messages.cpp.x	¶
				{ObjDir}monetary.cpp.x	¶
				{ObjDir}num_get.cpp.x	¶
				{ObjDir}num_get_float.cpp.x	¶
				{ObjDir}num_put.cpp.x	¶
				{ObjDir}num_put_float.cpp.x	¶
				{ObjDir}numpunct.cpp.x	¶
				{ObjDir}ostream.cpp.x	¶
				{ObjDir}sstream.cpp.x	¶
				{ObjDir}stdio_streambuf.cpp.x	¶
				{ObjDir}streambuf.cpp.x	¶
				{ObjDir}string_w.cpp.x	¶
				{ObjDir}strstream.cpp.x	¶
				{ObjDir}time_facets.cpp.x	¶
				# end		#*TY 11/25/2000 - updated for STLport.4.1


### Build Rules ###

build	ÄÄ	setup
build	ÄÄ	"{ObjDir}"STLportLib{config_}.o

install  ÄÄ  build
	###
	echo "¶nInstalling:     ¶{stl¶}:lib:STLportLib{config_}.o"
	if !`exists "{stl}":lib:`
		newfolder "{stl}":lib:
	end
	duplicate -y "{ObjDir}"STLportLib{config_}.o "{stl}":lib:STLportLib{config_}.o

"{ObjDir}"STLportLib{config_}.o  ÄÄ  {STLportLibObjFiles-PPC} {¥MondoBuild¥}
	###
	echo "¶nLibbing:        {Targ}"
	PPCLink ¶
		-xm l ¶
		-t 'XCOF' ¶
		-o {Targ} ¶
		{STLportLibObjFiles-PPC} ¶
		{Link_options} ¶
		{Sym-PPC} ¶
		# end

