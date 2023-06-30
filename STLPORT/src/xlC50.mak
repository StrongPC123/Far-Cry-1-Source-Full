#
# compiler
#
CC = xlc_r -qcpluscmt
CXX = xlC_r -qnotempinc

#
# Basename for libraries
#
LIB_BASENAME = libstlport_xlC50

#
# guts for common stuff
#
#

LINK=ar crv

OBJEXT=o
DYNEXT=so
STEXT=a
RM=rm -rf
PATH_SEP=/
MKDIR=mkdir -p
COMP=XLC50-AIX
INSTALL_STEP = install_unix 

# Alex Vanic, 07Aug2001, only want static libraries.
all: all_static

include common_macros.mak

CXXFLAGS_COMMON = -I${STLPORT_DIR}

CXXFLAGS_RELEASE_static = $(CXXFLAGS_COMMON) -O2

CXXFLAGS_DEBUG_static = $(CXXFLAGS_COMMON) -O -g

CXXFLAGS_STLDEBUG_static = $(CXXFLAGS_DEBUG_static) -D_STLP_DEBUG

include common_percent_rules.mak
include common_rules.mak

