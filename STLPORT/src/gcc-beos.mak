#
# compiler
#
CC = gcc
CXX = c++

#
# Basename for libraries
#
LIB_BASENAME = libstlport_gcc

#
# guts for common stuff
#
#
LINK=ar crv
DYN_LINK=g++ -nostart -o

OBJEXT=o
DYNEXT=so
STEXT=a
RM=rm -rf
PATH_SEP=/
MKDIR=mkdir -p
COMP=GCC-BEOS

all: msg all_static

msg:
	mkdir -p ../stlport/beos
	./beos-setup -setup

include common_macros.mak

WARNING_FLAGS= -W -Wno-sign-compare -Wno-unused -Wno-uninitialized

# boris : real locale implementation does not really work
# CXXFLAGS_COMMON = -I${STLPORT_DIR} ${WARNING_FLAGS} -D_STLP_REAL_LOCALE_IMPLEMENTED
CXXFLAGS_COMMON = -I${STLPORT_DIR} ${WARNING_FLAGS}

CXXFLAGS_RELEASE_static = $(CXXFLAGS_COMMON) -O2
CXXFLAGS_RELEASE_dynamic = $(CXXFLAGS_COMMON) -O2 -fPIC

CXXFLAGS_DEBUG_static = $(CXXFLAGS_COMMON) -O -g
CXXFLAGS_DEBUG_dynamic = $(CXXFLAGS_COMMON) -O -g -fPIC

CXXFLAGS_STLDEBUG_static = $(CXXFLAGS_DEBUG_static) -D_STLP_DEBUG
CXXFLAGS_STLDEBUG_dynamic = $(CXXFLAGS_DEBUG_dynamic) -D_STLP_DEBUG -fPIC

include common_percent_rules.mak
include common_rules.mak


install: all
	./beos-setup -install

uninstall:
	./beos-setup -uninstall

#%.s: %.cpp
#	$(CXX) $(CXXFLAGS) -O4 -S -pto $<  -o $@


