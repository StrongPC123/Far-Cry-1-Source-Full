
CUR_DIR= ${PWD}/

STL_INCL= -I. -I${PWD}/../stlport

#
# guts for common stuff
#
#
LINK=$(CC) -xar -o
DYN_LINK=$(CC) -G -o

OBJEXT=o
DYNEXT=so
STEXT=a
RM=rm -rf
PATH_SEP=/
MKDIR=mkdir -p
INSTALL_STEP = install_sun
PREPARE_STEP = prepare_sunpro

all: all_dynamic symbolic_links  all_static

include common_macros.mak

CXXFLAGS_COMMON = -mt +w2 -library=no%Cstd -features=rtti -xildoff ${STL_INCL} $(EXTRA_COMMON_FLAGS)

SHCXXFLAGS = -PIC

DEBUG_FLAGS = -O -g +w2 -D_STLP_DEBUG

#
# Try those flags to see if they help to get maximum efficiency :
# -Qoption iropt -R,-Ml30,-Ms30,-Mi1000000,-Mm1000000,-Mr1000000,-Ma1000000,-Mc1000000,-Mt1000000
RELEASE_FLAGS = -O2 +w2 -qoption ccfe -expand=1000


# install: $(TARGETS)
#	cp -p $(TARGETS) ${INSTALLDIR}

RELEASE_static_rep = -ptr${RELEASE_OBJDIR_static}
RELEASE_dynamic_rep = -ptr${RELEASE_OBJDIR_dynamic}
DEBUG_static_rep = -ptr${DEBUG_OBJDIR_static}
DEBUG_dynamic_rep = -ptr${DEBUG_OBJDIR_dynamic}
STLDEBUG_static_rep = -ptr${STLDEBUG_OBJDIR_static}
STLDEBUG_dynamic_rep = -ptr${STLDEBUG_OBJDIR_dynamic}

CXXFLAGS_RELEASE_static = $(CXXFLAGS_COMMON) ${RELEASE_FLAGS} ${RELEASE_static_rep}
CXXFLAGS_RELEASE_dynamic = $(CXXFLAGS_COMMON) ${RELEASE_FLAGS} $(SHCXXFLAGS) ${RELEASE_dynamic_rep}

CXXFLAGS_DEBUG_static = $(CXXFLAGS_COMMON) -O -g ${DEBUG_static_rep}
CXXFLAGS_DEBUG_dynamic = $(CXXFLAGS_COMMON) -O -g $(SHCXXFLAGS) ${DEBUG_dynamic_rep}

CXXFLAGS_STLDEBUG_static = $(CXXFLAGS_COMMON) -O -g ${STLDEBUG_static_rep} -D_STLP_DEBUG
CXXFLAGS_STLDEBUG_dynamic = $(CXXFLAGS_COMMON) -O -g $(SHCXXFLAGS) ${STLDEBUG_dynamic_rep} -D_STLP_DEBUG


LDFLAGS_RELEASE_static = ${CXXFLAGS_RELEASE_static} ${ATOMIC_ASM}
LDFLAGS_RELEASE_dynamic = ${CXXFLAGS_RELEASE_dynamic} -h${RELEASE_DYNLIB_SONAME} ${ATOMIC_ASM}

LDFLAGS_DEBUG_static = ${CXXFLAGS_DEBUG_static} ${ATOMIC_ASM}
LDFLAGS_DEBUG_dynamic = ${CXXFLAGS_DEBUG_dynamic} -h${DEBUG_DYNLIB_SONAME} ${ATOMIC_ASM}

LDFLAGS_STLDEBUG_static = ${CXXFLAGS_STLDEBUG_static} ${ATOMIC_ASM}
LDFLAGS_STLDEBUG_dynamic = ${CXXFLAGS_STLDEBUG_dynamic} -h${STLDEBUG_DYNLIB_SONAME} ${ATOMIC_ASM}

#LDLIBS_RELEASE_dynamic =  -lposix4
#LDLIBS_STLDEBUG_dynamic = -lposix4

include common_percent_rules.mak

#	for file in `cat ../etc/std_headers.txt ../etc/std_headers_classic_iostreams.txt ../etc/std_headers_cpp_runtime.txt ../etc/std_headers_cpp_runtime_h.txt`;
#	for file in `cat ../etc/std_headers.txt ../etc/std_headers_classic_iostreams.txt ../etc/std_headers_c.txt   ../etc/std_headers_cpp_runtime.txt ../etc/std_headers_c_h.txt  ../etc/std_headers_cpp_runtime_h.txt`;

../stlport/algorithm.SUNWCCh :
	for file in `cat ../etc/std_headers.txt`; \
	do \
	  rm -f ../stlport/$$file.SUNWCCh ; \
	  (cd ../stlport ; ln -s $$file $$file.SUNWCCh) ; \
        done
	for file in `cat ../etc/std_headers_classic_iostreams.txt`; \
	do \
	  rm -f ../stlport/clsssic_iostreams/$$file.SUNWCCh ; \
	  (cd ../stlport/classic_iostreams ; ln -s $$file $$file.SUNWCCh) ; \
        done
	for file in `cat ../etc/std_headers_c.txt`; \
	do \
	  rm -f ../stlport/cstd/$$file.SUNWCCh ; \
	  (cd ../stlport/cstd ; ln -s $$file $$file.SUNWCCh) ; \
        done
	for file in `cat ../etc/std_headers_cpp_runtime.txt`; \
	do \
	  rm -f ../stlport/cpp_runtime/$$file.SUNWCCh ; \
	  (cd ../stlport/cpp_runtime ; ln -s $$file $$file.SUNWCCh) ; \
        done

prepare_sunpro : ../stlport/algorithm.SUNWCCh

remove_c_headers : 
	for file in `cat ../etc/std_headers_c_h.txt ../etc/std_headers_c.txt`; \
	do \
	rm -f ../stlport/$$file ../stlport/$$file.SUNWCCh; \
	done

include common_rules.mak
