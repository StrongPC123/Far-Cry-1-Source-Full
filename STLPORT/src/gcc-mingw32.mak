#
# compiler
#
CC = gcc
CXX = c++

#
# Basename for libraries
#
LIB_BASENAME = libstlport_mingw32

#
# guts for common stuff
#
#
LINK=ar crv
DYN_LINK=c++ -shared -o

OBJEXT=o
DYNEXT=dll
STEXT=a
RM=rm -rf
PATH_SEP=/
MKDIR=mkdir -p
COMP=MINGW32

STATIC_SUFFIX=_static

all: all_dynamic all_static

include common_macros.mak

RESFILE=$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)stlport.o
RESFILE_debug=$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)stlport.o
RESFILE_stldebug=$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)stlport.o

WARNING_FLAGS= -W -Wno-sign-compare -Wno-unused -Wno-uninitialized

CXXFLAGS_COMMON = -I${STLPORT_DIR} ${WARNING_FLAGS}

CXXFLAGS_RELEASE_static = $(CXXFLAGS_COMMON) -O2
# CXXFLAGS_RELEASE_dynamic = $(CXXFLAGS_COMMON) -O2 -shared -D_STLP_USE_DYNAMIC_LIB
CXXFLAGS_RELEASE_dynamic = $(CXXFLAGS_COMMON) -O2 -D_STLP_USE_DYNAMIC_LIB

CXXFLAGS_DEBUG_static = $(CXXFLAGS_COMMON) -O -g
CXXFLAGS_DEBUG_dynamic = $(CXXFLAGS_COMMON) -O -g -shared -D_STLP_USE_DYNAMIC_LIB

CXXFLAGS_STLDEBUG_static = $(CXXFLAGS_DEBUG_static) -D_STLP_DEBUG
CXXFLAGS_STLDEBUG_dynamic = $(CXXFLAGS_DEBUG_dynamic) -D_STLP_DEBUG -shared -D_STLP_USE_DYNAMIC_LIB

LDFLAGS_RELEASE_static =
# LDFLAGS_RELEASE_dynamic = ${CXXFLAGS_RELEASE_dynamic}  -Wl,--no-undefined, -Wl,--export-all-symbols -Wl,-d -Wl,--out-implib,${OUTDIR}/${RELEASE_NAME}.a
LDFLAGS_RELEASE_dynamic = ${CXXFLAGS_RELEASE_dynamic}  -Wl,--export-all-symbols -Wl,-d -Wl,--out-implib,${OUTDIR}/${RELEASE_NAME}.a

LDFLAGS_DEBUG_static =
LDFLAGS_DEBUG_dynamic = ${CXXFLAGS_DEBUG_dynamic}  -Wl,--export-all-symbols -Wl,-d -Wl,--out-implib,${OUTDIR}/${DEBUG_NAME}.a

LDFLAGS_STLDEBUG_static =
LDFLAGS_STLDEBUG_dynamic = ${CXXFLAGS_STLDEBUG_dynamic} -Wl,--export-all-symbols -Wl,--out-implib,${OUTDIR}/${STLDEBUG_NAME}.a


include common_percent_rules.mak
include common_rules.mak


${RESFILE}: stlport.rc
	windres -O coff --define COMP=${COMP} --define BUILD= -o $(RELEASE_OBJDIR_dynamic)$(PATH_SEP)stlport.o stlport.rc

${RESFILE_debug}: stlport.rc
	windres -O coff --define COMP=${COMP} --define BUILD=_DEBUG -o $(DEBUG_OBJDIR_dynamic)$(PATH_SEP)stlport.o stlport.rc

${RESFILE_stldebug}: stlport.rc
	windres -O coff --define COMP=${COMP} --define BUILD=_STLDEBUG -o $(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)stlport.o stlport.rc

#%.s: %.cpp
#	$(CXX) $(CXXFLAGS) -O4 -S -pto $<  -o $@


