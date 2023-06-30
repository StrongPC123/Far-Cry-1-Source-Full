#
# Note : this makefile has been only tested in como + gcc setup
#

#
# compiler
#
CC = gcc
CXX = como

#
# Basename for libraries
#
LIB_BASENAME = libstlport_como

#
# guts for common stuff
#
#
LINK=ar cr
# 2.95 flag
DYN_LINK=-copt="-shared -all" -G -o 

OBJEXT=o
DYNEXT=so
STEXT=a
RM=rm -rf
PATH_SEP=/
MKDIR=mkdir -p
COMP=COMO_$(ARCH)
INSTALL_STEP = install_unix 

all: all_static symbolic_links 

include common_macros.mak

WARNING_FLAGS=  -DLIBCIO= --diag_suppress=68

CXXFLAGS_COMMON = -I. -I${STLPORT_DIR} ${WARNING_FLAGS}

CXXFLAGS_RELEASE_static = $(CXXFLAGS_COMMON) -O2
CXXFLAGS_RELEASE_dynamic = $(CXXFLAGS_COMMON) -O2 -fPIC

CXXFLAGS_DEBUG_static = $(CXXFLAGS_COMMON) -g -O
CXXFLAGS_DEBUG_dynamic = $(CXXFLAGS_COMMON) -g -O -fPIC

CXXFLAGS_STLDEBUG_static = $(CXXFLAGS_DEBUG_static) -D_STLP_DEBUG
CXXFLAGS_STLDEBUG_dynamic = $(CXXFLAGS_DEBUG_dynamic) -D_STLP_DEBUG

include common_percent_rules.mak
include common_rules.mak


