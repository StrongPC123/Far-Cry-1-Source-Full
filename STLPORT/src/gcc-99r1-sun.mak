#
# Note : this makefile is for gcc-2.95 and later !
#

#
# compiler
#
CC = gcc
CXX = c++ -pthreads

#
# Basename for libraries
#
LIB_BASENAME = libstlport_gcc99r1

#
# guts for common stuff
#
#
LINK=ar cr
# 2.95 flag
DYN_LINK=c++ -pthreads -shared -o

OBJEXT=o
DYNEXT=so
STEXT=a
RM=rm -rf
PATH_SEP=/
MKDIR=mkdir -p
COMP=GCC-99r1
INSTALL_STEP = install_unix 

all:  all_dynamic all_static symbolic_links

include common_macros.mak

WARNING_FLAGS= -Wall -W -Wno-sign-compare -Wno-unused -Wno-uninitialized

CXXFLAGS_COMMON =  -nostdinc++ -fexceptions -ftemplate-depth-32 -D_STLP_HAS_NO_NAMESPACES -shared -I${STLPORT_DIR} ${WARNING_FLAGS}

CXXFLAGS_RELEASE_static = $(CXXFLAGS_COMMON) -O2
CXXFLAGS_RELEASE_dynamic = $(CXXFLAGS_COMMON) -O2 -fPIC

CXXFLAGS_DEBUG_static = $(CXXFLAGS_COMMON) -gstabs
CXXFLAGS_DEBUG_dynamic = $(CXXFLAGS_COMMON) -gstabs -fPIC

CXXFLAGS_STLDEBUG_static = $(CXXFLAGS_DEBUG_static) -D_STLP_DEBUG
CXXFLAGS_STLDEBUG_dynamic = $(CXXFLAGS_DEBUG_dynamic) -D_STLP_DEBUG

LDFLAGS_RELEASE_dynamic = ${CXXFLAGS_RELEASE_dynamic}

LDFLAGS_DEBUG_dynamic = ${CXXFLAGS_DEBUG_dynamic}

LDFLAGS_STLDEBUG_dynamic = ${CXXFLAGS_STLDEBUG_dynamic}

LDLIBS_RELEASE_dynamic =  -lposix4

LDLIBS_STLDEBUG_dynamic = -lposix4

include common_percent_rules.mak
include common_rules.mak

#%.s: %.cpp
#	$(CXX) $(CXXFLAGS) -O4 -S -pto $<  -o $@


