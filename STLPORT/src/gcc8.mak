#
# compiler
#
CC = gcc
CXX = c++  -fexceptions -D__EXCEPTIONS -ftemplate-depth-20

#
# Basename for libraries
#
LIB_BASENAME = libstlport_gcc

#
# guts for common stuff
#
#
LINK=ar crv
DYN_LINK=ar crv

OBJEXT=o
DYNEXT=so
STEXT=a
RM=rm -rf
PATH_SEP=/
MKDIR=mkdir -p
COMP=GCC8${ARCH}
INSTALL_STEP = install_unix 

all: all_static

include common_macros.mak

WARNING_FLAGS= -Wall -W -Wno-sign-compare -Wno-unused -Wno-uninitialized

CXXFLAGS_COMMON = -I${STLPORT_DIR} ${WARNING_FLAGS}

# -O does not always work for gcc-2.8
CXXFLAGS_RELEASE_static = $(CXXFLAGS_COMMON)
CXXFLAGS_RELEASE_dynamic = $(CXXFLAGS_COMMON) -fpic

CXXFLAGS_DEBUG_static = $(CXXFLAGS_COMMON) -O -g
CXXFLAGS_DEBUG_dynamic = $(CXXFLAGS_COMMON) -O -g -fpic

CXXFLAGS_STLDEBUG_static = $(CXXFLAGS_DEBUG_static) -D_STLP_DEBUG
CXXFLAGS_STLDEBUG_dynamic = $(CXXFLAGS_DEBUG_dynamic) -D_STLP_DEBUG -fpic

include common_percent_rules.mak
include common_rules.mak


#install: all
#	cp -p $(LIB_TARGET) ${D_LIB_TARGET} ../lib

#%.s: %.cpp
#	$(CXX) $(CXXFLAGS) -O4 -S -pto $<  -o $@


