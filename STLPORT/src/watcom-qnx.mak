#
# compiler
#
CC = cc
CXX = $(CC)

#
# Basename for libraries
#
LIB_BASENAME = libstlport_watcom

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
COMP=WATCOM
INSTALL_STEP = install_unix 

all: all_dynamic symbolic_links  all_static 

CXXFLAGS_COMMON = -I${STLPORT_DIR} ${WARNING_FLAGS}

CXXFLAGS_RELEASE_static = $(CXXFLAGS_COMMON) -O2
CXXFLAGS_RELEASE_dynamic = $(CXXFLAGS_COMMON) -O2

CXXFLAGS_DEBUG_static = $(CXXFLAGS_COMMON) -O -g
CXXFLAGS_DEBUG_dynamic = $(CXXFLAGS_COMMON) -O -g

CXXFLAGS_STLDEBUG_static = $(CXXFLAGS_DEBUG_static) -D_STLP_DEBUG
CXXFLAGS_STLDEBUG_dynamic = $(CXXFLAGS_DEBUG_dynamic) -D_STLP_DEBUG

include common_percent_rules.mak

include common_rules.mak


