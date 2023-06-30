#
# Note : this makefile is for gcc-2.95 and later (HP-UX)!
#

#
# compiler
#
CC = gcc
CXX = c++

# -fkeep-inline-functions
# -finline-functions
# -fno-default-inline

#
# Basename for libraries
#
LIB_BASENAME = libstlport_gcc

#
# guts for common stuff
#
#
LINK=ar cr
# 2.95 flag
# DYN_LINK=c++ -pthreads -fPIC -shared -o
# DYN_LINK=c++ -pthreads -nostdinc++ -fexceptions -frtti -fPIC -O -fno-implement-inlines -Winline -Wextern-inline -shared -D_STLP_DEBUG -o
# DYN_LINK=ld -shared -export-dynamic -Bdynamic -Bsymbolic -Ur -o
# DYN_LINK=/usr/bin/ld -z -b -o

DYN_LINK= c++ -shared -o 

OBJEXT=o
DYNEXT=sl
STEXT=a
RM=rm -rf
PATH_SEP=/
MKDIR=mkdir -p
COMP=GCC-HP
INSTALL_STEP = install_unix 

all: all_dynamic all_static symbolic_links  

include common_macros.mak

WARNING_FLAGS= -Wall -W -Wno-sign-compare -Wno-unused -Wno-uninitialized -D_REENTRANT -D_POSIX_C_SOURCE=199506L -D__EXTENSIONS__

# CXXFLAGS_COMMON = -ftemplate-depth-32 -I${STLPORT_DIR} ${WARNING_FLAGS}
CXXFLAGS_COMMON = -I${STLPORT_DIR} ${WARNING_FLAGS}

CXXFLAGS_RELEASE_static = $(CXXFLAGS_COMMON) -O
CXXFLAGS_RELEASE_dynamic = $(CXXFLAGS_COMMON) -O -fPIC

CXXFLAGS_DEBUG_static = $(CXXFLAGS_COMMON) -O -g
CXXFLAGS_DEBUG_dynamic = $(CXXFLAGS_COMMON) -O -g -fPIC

CXXFLAGS_STLDEBUG_static = $(CXXFLAGS_DEBUG_static) -D_STLP_DEBUG
CXXFLAGS_STLDEBUG_dynamic = $(CXXFLAGS_DEBUG_dynamic) -D_STLP_DEBUG

# LDFLAGS_RELEASE_dynamic = ${CXXFLAGS_RELEASE_dynamic} -Wl,+h${RELEASE_DYNLIB_SONAME}
# LDFLAGS_DEBUG_dynamic = ${CXXFLAGS_DEBUG_dynamic} -Wl,+h${DEBUG_DYNLIB_SONAME}
# LDFLAGS_STLDEBUG_dynamic = ${CXXFLAGS_STLDEBUG_dynamic} -Wl,+h${STLDEBUG_DYNLIB_SONAME}

LDFLAGS_RELEASE_dynamic = ${CXXFLAGS_RELEASE_dynamic}
LDFLAGS_DEBUG_dynamic = ${CXXFLAGS_DEBUG_dynamic}
LDFLAGS_STLDEBUG_dynamic = ${CXXFLAGS_STLDEBUG_dynamic}

include common_percent_rules.mak
include common_rules.mak

#install: all
#	cp -p $(LIB_TARGET) ${D_LIB_TARGET} ../lib

#%.s: %.cpp
#	$(CXX) $(CXXFLAGS) -O4 -S -pto $<  -o $@
