#
# compiler
#
CC = gcc
CXX = c++

COMP = cygwin

#
# Basename for libraries
#
LIB_BASENAME = libstlport_cygwin

#
# guts for common stuff
#
#
LINK=ar crv
DYN_LINK=c++ -o

OBJEXT=o
DYNEXT=dll
STEXT=a
RM=rm -rf
PATH_SEP=/
MKDIR=mkdir -p
COMP=cygwin

# STATIC_SUFFIX=_static

all: all_static
INSTALL_STEP = install_unix

include common_macros.mak

WARNING_FLAGS= -W -Wno-sign-compare -Wno-unused -Wno-uninitialized

CXXFLAGS_COMMON = -I${STLPORT_DIR} ${WARNING_FLAGS}

CXXFLAGS_RELEASE_static = $(CXXFLAGS_COMMON) -O2
CXXFLAGS_RELEASE_dynamic = $(CXXFLAGS_COMMON) -O2 -D_STLP_USE_DYNAMIC_LIB

CXXFLAGS_DEBUG_static = $(CXXFLAGS_COMMON) -O -g
CXXFLAGS_DEBUG_dynamic = $(CXXFLAGS_COMMON) -O -g -D_STLP_USE_DYNAMIC_LIB

CXXFLAGS_STLDEBUG_static = $(CXXFLAGS_DEBUG_static) -D_STLP_DEBUG
CXXFLAGS_STLDEBUG_dynamic = $(CXXFLAGS_DEBUG_dynamic) -D_STLP_DEBUG -D_STLP_USE_DYNAMIC_LIB

LDFLAGS_RELEASE_static =
LDFLAGS_RELEASE_dynamic = ${CXXFLAGS_RELEASE_dynamic}  -Wl,-d

LDFLAGS_DEBUG_static = 
LDFLAGS_DEBUG_dynamic = ${CXXFLAGS_DEBUG_dynamic}  -Wl,--export-all-symbols -Wl,-d -Wl,--out-implib,${OUTDIR}/${DEBUG_NAME}.a

LDFLAGS_STLDEBUG_static = 
LDFLAGS_STLDEBUG_dynamic = ${CXXFLAGS_STLDEBUG_dynamic} -Wl,--export-all-symbols -Wl,--out-implib,${OUTDIR}/${STLDEBUG_NAME}.a

include common_percent_rules.mak

include common_rules.mak


