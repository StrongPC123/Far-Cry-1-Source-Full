#
# compiler
#
CC = gcc
CXX = c++  -pthread -fexceptions

#
# Basename for libraries
#
LIB_BASENAME = libstlport_gcc

#
# guts for common stuff
#
#
LINK=ar crv
DYN_LINK=gcc -shared -o

OBJEXT=o
DYNEXT=so
STEXT=a
RM=rm -rf
PATH_SEP=/
MKDIR=mkdir -p
COMP=GCC-LINUX
INSTALL_STEP = install_unix 

all: msg all_dynamic all_static symbolic_links

msg:
	@echo "*** ATTENTION! ***"
	@echo "This makefile tries to use system locale which might not work well with all glibc flavours."
	@echo "If build fails, please resort to gcc.mak which will build C-locale only version for STLport"
	@echo "******************"
include common_macros.mak

WARNING_FLAGS= -W -Wno-sign-compare -Wno-unused -Wno-uninitialized -D_STLP_USE_GLIBC

# boris : real locale implementation may not really work
CXXFLAGS_COMMON = -I${STLPORT_DIR} ${WARNING_FLAGS} -D_STLP_REAL_LOCALE_IMPLEMENTED -D_GNU_SOURCE
# CXXFLAGS_COMMON = -I${STLPORT_DIR} ${WARNING_FLAGS}

CXXFLAGS_RELEASE_static = $(CXXFLAGS_COMMON) -O2
CXXFLAGS_RELEASE_dynamic = $(CXXFLAGS_COMMON) -O2 -fPIC

CXXFLAGS_DEBUG_static = $(CXXFLAGS_COMMON) -O -g
CXXFLAGS_DEBUG_dynamic = $(CXXFLAGS_COMMON) -O -g -fPIC

CXXFLAGS_STLDEBUG_static = $(CXXFLAGS_DEBUG_static) -D_STLP_DEBUG
CXXFLAGS_STLDEBUG_dynamic = $(CXXFLAGS_DEBUG_dynamic) -D_STLP_DEBUG -fPIC

include common_percent_rules.mak
include common_rules.mak


#install: all
#	cp -p $(LIB_TARGET) ${D_LIB_TARGET} ../lib

#%.s: %.cpp
#	$(CXX) $(CXXFLAGS) -O4 -S -pto $<  -o $@


