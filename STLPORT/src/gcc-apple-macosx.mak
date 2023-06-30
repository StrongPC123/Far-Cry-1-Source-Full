#
# Note : this makefile is for gcc-2.95 and later !
#

#
# compiler
#
CC = gcc
CXX = c++ -fexceptions

#
# Basename for libraries
#
LIB_BASENAME = libstlport_gcc

#
# guts for common stuff
#
#
LINK=libtool -static -o
# 2.95 flag
DYN_LINK=libtool -dynamic -framework System -lcc_dynamic -lstdc++ -install_name @executable_path/$(@F) -o

OBJEXT=o
DYNEXT=dylib
STEXT=a
RM=rm -Rf
PATH_SEP=/
MKDIR=mkdir -p
COMP=GCC$(ARCH)
INSTALL_STEP = install_unix 

all: all_dynamic all_static

include common_macros.mak

WARNING_FLAGS= -Wall -W -Wno-sign-compare -Wno-unused -Wno-uninitialized -ftemplate-depth-32

CXXFLAGS_COMMON = -I${STLPORT_DIR} ${WARNING_FLAGS}

CXXFLAGS_RELEASE_static = $(CXXFLAGS_COMMON) -O2
CXXFLAGS_RELEASE_dynamic = $(CXXFLAGS_COMMON) -O2 -fPIC

CXXFLAGS_DEBUG_static = $(CXXFLAGS_COMMON) -O -g
CXXFLAGS_DEBUG_dynamic = $(CXXFLAGS_COMMON) -O -g -fPIC

CXXFLAGS_STLDEBUG_static = $(CXXFLAGS_DEBUG_static) -D_STLP_DEBUG
CXXFLAGS_STLDEBUG_dynamic = $(CXXFLAGS_DEBUG_dynamic) -D_STLP_DEBUG -fPIC

include common_percent_rules.mak
include common_rules.mak

