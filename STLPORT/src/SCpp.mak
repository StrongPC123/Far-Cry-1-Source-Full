MAKEFILE        = SCpp.mak
¥MondoBuild¥    = {MAKEFILE}  # Make blank to avoid rebuilds when makefile is modified

Sym-68K         	= -sym on
!ifdef DEBUG
config_				= .68K.DBG
SCpp_DebugOptions 	= -inline none,global -opt none -frames {Sym-68K}
STL_DebugOptions 	= -d _STLP_DEBUG -d _STLP_DEBUG_ALLOC -d _STLP_DEBUG_UNINITIALIZED
!else
config_				= .68K
SCpp_DebugOptions 	= 
STL_DebugOptions 	= 
!endif
ObjDir          	= :{config_}:
SrcDir				= :

Includes     	= -i : -i "{STL}" -i "{CIncludes}"

SCpp_Options 	= -model far -ansi on -ansifor -bool on -exceptions on -rtti on -b2  -mbg full -opt all -inline all,global -includes unix_mac -w 12
				
STL_Options  	= 	#-d _STLP_USE_NEWALLOC ¶
					#-d _STLP_NO_SGI_IOSTREAMS ¶
					# end

Link_options = ¶
		-c 'MPS ' ¶
		-mf ¶
		-d ¶
		-w ¶
		-model far ¶
		-srtsg all ¶
		#{Sym-68K} ¶
		# end

### Default Rules ###

"{ObjDir}" Ä "{SrcDir}"

.cpp.o  Ä  .cpp  # {¥MondoBuild¥}
	###
	echo "¶nCompiling:      '"{depDir}{default}.cpp"'"
	"{SCpp}" {depDir}{default}.cpp ¶
		-o {targDir}{default}.cpp.o ¶
		-seg "{default}" ¶
		{Includes} ¶
		{SCpp_Options} {SCpp_DebugOptions} {other_SCpp_Options} ¶
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

STLportLibObjFiles1-68K    =  ¶
				{ObjDir}c_locale_stub.cpp.o	¶
				{ObjDir}codecvt.cpp.o	¶
				{ObjDir}collate.cpp.o	¶
				{ObjDir}complex.cpp.o	¶
				{ObjDir}complex_exp.cpp.o	¶
				{ObjDir}complex_io.cpp.o	¶
				{ObjDir}complex_io_w.cpp.o	¶
				{ObjDir}complex_trig.cpp.o	¶
				{ObjDir}ctype.cpp.o	¶
				{ObjDir}dll_main.cpp.o	¶
				{ObjDir}fstream.cpp.o	¶
				{ObjDir}ios.cpp.o	¶
				{ObjDir}iostream.cpp.o	¶
				{ObjDir}istream.cpp.o	¶
				{ObjDir}locale.cpp.o	¶
				{ObjDir}locale_catalog.cpp.o	¶
				#
STLportLibObjFiles2-68K    =  ¶
				{ObjDir}facets_byname.cpp.o	¶
				{ObjDir}locale_impl.cpp.o	¶
				{ObjDir}messages.cpp.o	¶
				{ObjDir}monetary.cpp.o	¶
				{ObjDir}num_get.cpp.o	¶
				{ObjDir}num_get_float.cpp.o	¶
				{ObjDir}num_put.cpp.o	¶
				{ObjDir}num_put_float.cpp.o	¶
				{ObjDir}numpunct.cpp.o	¶
				{ObjDir}ostream.cpp.o	¶
				{ObjDir}sstream.cpp.o	¶
				{ObjDir}stdio_streambuf.cpp.o	¶
				{ObjDir}streambuf.cpp.o	¶
				{ObjDir}string_w.cpp.o	¶
				{ObjDir}strstream.cpp.o	¶
				{ObjDir}time_facets.cpp.o	¶
				# end		#*TY 11/25/2000 - updated for STLport.4.1


### Build Rules ###

build	ÄÄ	setup
build	ÄÄ	"{ObjDir}"STLportLib{config_}.o

"{ObjDir}"STLportLib{config_}.o	ÄÄ  {STLportLibObjFiles1-68K} {STLportLibObjFiles2-68K} {¥MondoBuild¥}
	###
	echo "¶nLibbing:        {Targ}"
	Lib ¶
		-o "{ObjDir}"STLportLib1{config_}.o ¶
		{STLportLibObjFiles1-68K} ¶
		-mf ¶
		-d ¶
		-sym on ¶
		# end
	Lib ¶
		-o "{ObjDir}"STLportLib2{config_}.o ¶
		{STLportLibObjFiles2-68K} ¶
		-mf ¶
		-d ¶
		-sym on ¶
		# end
	Lib ¶
		-o {Targ} ¶
		"{ObjDir}"STLportLib1{config_}.o "{ObjDir}"STLportLib2{config_}.o ¶
		-mf ¶
		-d ¶
		-sym on ¶
		# end

install  ÄÄ  build
	###
	echo "¶nInstalling:     ¶{stl¶}:lib:STLportLib{config_}.o"
	if !`exists "{stl}":lib:`
		newfolder "{stl}":lib:
	end
	duplicate -y "{ObjDir}"STLportLib{config_}.o "{stl}":lib:STLportLib{config_}.o
