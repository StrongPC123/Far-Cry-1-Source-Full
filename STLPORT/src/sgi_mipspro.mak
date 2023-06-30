#
# Basename for libraries
#
.POSIX:

SHELL=/bin/sh

LIB_BASENAME = libstlport_mipspro

STL_INCL= -I. -I${PWD}/../stlport/
CUR_DIR=./

CC = CC
CXX = CC

#
# guts for common stuff
#
#
LINK=$(CC) -ar -all -o
DYN_LINK=$(CC) -shared -all -o

OBJEXT=o
DYNEXT=so
STEXT=a
RM=rm -rf
PATH_SEP=/
MKDIR=mkdir -p
COMP=MIPS
INSTALL_STEP = install_unix 

all: msg all_dynamic all_static symbolic_links 

msg:
	@echo "*** ATTENTION! ***"
	@echo "This makefile requires GNU make!"
	@echo "******************"

include common_macros.mak

CXXFLAGS_COMMON = -J 4 -ansi -LANG:std -I. -D_PTHREADS ${STL_INCL} 

DEBUG_FLAGS = -g +w2 -D_STLP_DEBUG
RELEASE_FLAGS = -O2


# install: $(TARGETS)
#	cp -p $(TARGETS) ${INSTALLDIR}

CXXFLAGS_RELEASE_static = $(CXXFLAGS_COMMON) ${RELEASE_FLAGS}
CXXFLAGS_RELEASE_dynamic = $(CXXFLAGS_COMMON) ${RELEASE_FLAGS} $(SHCXXFLAGS)

CXXFLAGS_DEBUG_static = $(CXXFLAGS_COMMON) -O -g
CXXFLAGS_DEBUG_dynamic = $(CXXFLAGS_COMMON) -O -g

CXXFLAGS_STLDEBUG_static = $(CXXFLAGS_COMMON) -O -g -D_STLP_DEBUG
CXXFLAGS_STLDEBUG_dynamic = $(CXXFLAGS_COMMON) -O -g -D_STLP_DEBUG

LDFLAGS_RELEASE_static = ${CXXFLAGS_RELEASE_static}
LDFLAGS_RELEASE_dynamic = ${CXXFLAGS_RELEASE_dynamic}

LDFLAGS_DEBUG_static = ${CXXFLAGS_DEBUG_static}
LDFLAGS_DEBUG_dynamic = ${CXXFLAGS_DEBUG_dynamic}

LDFLAGS_STLDEBUG_static = ${CXXFLAGS_STLDEBUG_static}
LDFLAGS_STLDEBUG_dynamic = ${CXXFLAGS_STLDEBUG_dynamic}

include common_percent_rules.mak
include common_rules.mak

