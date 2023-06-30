#
# Note : this makefile is for gcc-2.95.3 and later !
#

#
# compiler
#
CC = gcc -noixemul -m68020
CXX = g++ -noixemul -m68020 -ftemplate-depth-32

#
# Basename for libraries
#
LIB_BASENAME = libstlport_gcc

#
# guts for common stuff
#
#
LINK=ar cr
OBJEXT=o
STEXT=a
RM=rm -rf
PATH_SEP=/
MKDIR=mkdir -p
COMP=GCC$(ARCH)

all: release_static

include common_macros.mak

WARNING_FLAGS= -Wall -W -Wno-sign-compare -Wno-unused -Wno-uninitialized

CXXFLAGS_COMMON = -I${STLPORT_DIR} ${WARNING_FLAGS}

CXXFLAGS_RELEASE_static = $(CXXFLAGS_COMMON) -O2

CXXFLAGS_DEBUG_static = $(CXXFLAGS_COMMON) -g

CXXFLAGS_STLDEBUG_static = $(CXXFLAGS_DEBUG_static) -D_STLP_DEBUG

include common_percent_rules.mak
include common_rules.mak

$(OUTDIR)$(PATH_SEP)$(RELEASE_LIB) : $(OUTDIR) $(RELEASE_OBJDIR_static) $(DEF_FILE) $(RELEASE_OBJECTS_static)
	$(LINK) $(LINK_OUT)$(OUTDIR)$(PATH_SEP)$(RELEASE_LIB) $(LDFLAGS_RELEASE_static) $(RELEASE_OBJDIR_static)$(PATH_SEP)*.$(OBJEXT) $(LDLIBS_RELEASE_static)
