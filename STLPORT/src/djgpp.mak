
#
# This is makefile for compiling with DJGPP.
# Since DJGPP is used to create DOS protected mode, it has no
# dynamic link library. Only static libraries are created.
# With DJGPP, gcc is used to compile c++. It will know the 
# language being compiled automatically from file extension of
# the source code being compiled.
#
# Tanes Sriviroolchai (tanes73@yahoo.com)
# Jan 4, 2001
#

#
# compiler
#
CC = gcc
CXX = gcc

#
# Basename for libraries
#
LIB_BASENAME = libstlport_djgpp

#
# guts for common stuff
#
#
LINK=ar crv

OBJEXT=o
STEXT=a
RM=rm -rf
PATH_SEP=/
MKDIR=mkdir -p
COMP=DJGPP

STATIC_SUFFIX=_static

all: all_static

include common_macros.mak

RESFILE=$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)stlport.o
RESFILE_debug=$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)stlport.o
RESFILE_stldebug=$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)stlport.o

WARNING_FLAGS= -W -Wno-sign-compare -Wno-unused -Wno-uninitialized

CXXFLAGS_COMMON = -I${STLPORT_DIR} ${WARNING_FLAGS} -ftemplate-depth-32 -mbnu210

CXXFLAGS_RELEASE_static = $(CXXFLAGS_COMMON) -O2

CXXFLAGS_DEBUG_static = $(CXXFLAGS_COMMON) -O -g

CXXFLAGS_STLDEBUG_static = $(CXXFLAGS_DEBUG_static) -D_STLP_DEBUG

LDFLAGS_RELEASE_static = 

LDFLAGS_DEBUG_static = 

LDFLAGS_STLDEBUG_static = 


include common_percent_rules.mak
include common_rules.mak


#%.s: %.cpp
#	$(CXX) $(CXXFLAGS) -O4 -S -pto $<  -o $@


