#
# Note: This makefile is for the Linux Intel C++ compiler 'icc'
#

#
# compiler
#
CC = icc 
CXX = icpc -pthread

#
# Basename for libraries
#
LIB_BASENAME = libstlport_icc

#
# guts for common stuff
#
#
LINK=ar cr
DYN_LINK=icc -pthread -shared -o

OBJEXT=o
DYNEXT=so
STEXT=a
RM=rm -rf
PATH_SEP=/
MKDIR=mkdir -p
COMP=ICC
INSTALL_STEP = install_unix 

all:  all_dynamic all_static symbolic_links 

include common_macros.mak

WARNING_FLAGS= -w1

CXXFLAGS_COMMON = -I${STLPORT_DIR} ${WARNING_FLAGS}

CXXFLAGS_RELEASE_static = $(CXXFLAGS_COMMON) -O3 -ipo -ipo_obj
CXXFLAGS_RELEASE_dynamic = $(CXXFLAGS_COMMON) -O3 -fPIC -ipo -ipo_obj

CXXFLAGS_DEBUG_static = $(CXXFLAGS_COMMON) -O0 -g
CXXFLAGS_DEBUG_dynamic = $(CXXFLAGS_COMMON) -O0 -g -fPIC

CXXFLAGS_STLDEBUG_static = $(CXXFLAGS_DEBUG_static) -D_STLP_DEBUG
CXXFLAGS_STLDEBUG_dynamic = $(CXXFLAGS_DEBUG_dynamic) -D_STLP_DEBUG

include common_percent_rules.mak
include common_rules.mak


#install: all
#	cp -p $(LIB_TARGET) ${D_LIB_TARGET} ../lib

#%.s: %.cpp
#	$(CXX) $(CXXFLAGS) -O4 -S -pto $<  -o $@



