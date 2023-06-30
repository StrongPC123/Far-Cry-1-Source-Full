#
# Note : this requires GNU make
#

# Basename for libraries
#
LIB_BASENAME = libstlport_dec

CUR_DIR= $(PWD)/

# point this to proper location
STL_INCL= -I. -I../stlport -I/usr/include

#
# guts for common stuff
#
#
CC = cxx
CXX = cxx

LINK=$(CXX) -o
DYN_LINK=$(CXX) -o

OBJEXT=o
DYNEXT=so
STEXT=a
RM=rm -rf
PATH_SEP=/
MKDIR=mkdir -p
COMP=DEC

# LINK_OUT=-xar -o  
# DYNLINK_OUT=-o 

# all: all_dynamic all_static
# Boris : do not see a reasonable way to link static library witout instantiating in .o 
# Anyone ?
all: msg all_dynamic symbolic_links

msg:
	@echo "*** ATTENTION! ***"
	@echo "This makefile requires GNU make!"
	@echo "******************"

include common_macros.mak

# Rules

MTFLAGS = -pthread

CXXFLAGS_COMMON = -std ansi -nousing_std -pt -rtti $(MTFLAGS) ${STL_INCL} -D_PTHREADS

SHCXXFLAGS = -shared
RELEASE_FLAGS = -O

LIBS =  

RELEASE_static_rep = -ptr ${RELEASE_OBJDIR_static}
RELEASE_dynamic_rep = -ptr ${RELEASE_OBJDIR_dynamic}
DEBUG_static_rep = -ptr ${DEBUG_OBJDIR_static}
DEBUG_dynamic_rep = -ptr ${DEBUG_OBJDIR_dynamic}
STLDEBUG_static_rep = -ptr ${STLDEBUG_OBJDIR_static}
STLDEBUG_dynamic_rep = -ptr ${STLDEBUG_OBJDIR_dynamic}

CXXFLAGS_RELEASE_static = $(CXXFLAGS_COMMON) ${RELEASE_FLAGS} ${RELEASE_static_rep}
CXXFLAGS_RELEASE_dynamic = $(CXXFLAGS_COMMON) ${RELEASE_FLAGS} $(SHCXXFLAGS) ${RELEASE_dynamic_rep}

CXXFLAGS_DEBUG_static = $(CXXFLAGS_COMMON) -O -g ${DEBUG_static_rep}
CXXFLAGS_DEBUG_dynamic = $(CXXFLAGS_COMMON) -O -g $(SHCXXFLAGS) ${DEBUG_dynamic_rep}

CXXFLAGS_STLDEBUG_static = $(CXXFLAGS_COMMON) -O -g ${STLDEBUG_static_rep} -D_STLP_DEBUG
CXXFLAGS_STLDEBUG_dynamic = $(CXXFLAGS_COMMON) -O -g $(SHCXXFLAGS) ${STLDEBUG_dynamic_rep} -D_STLP_DEBUG

LDFLAGS_RELEASE_static = ${CXXFLAGS_RELEASE_static}
LDFLAGS_RELEASE_dynamic = ${CXXFLAGS_RELEASE_dynamic}
LDLIBS_RELEASE_dynamic = -lm
LDLIBS_RELEASE_static = -lm

LDFLAGS_DEBUG_static = ${CXXFLAGS_DEBUG_static}
LDFLAGS_DEBUG_dynamic = ${CXXFLAGS_DEBUG_dynamic}
LDLIBS_DEBUG_dynamic = -lm
LDLIBS_DEBUG_static = -lm

LDFLAGS_STLDEBUG_static = ${CXXFLAGS_STLDEBUG_static}
LDFLAGS_STLDEBUG_dynamic = ${CXXFLAGS_STLDEBUG_dynamic}
LDLIBS_STLDEBUG_dynamic = -lm
LDLIBS_STLDEBUG_static = -lm

INSTALL_STEP = install_unix 

include common_percent_rules.mak
include common_rules.mak


%.i : %.cpp
	$(CXX) $(CXXFLAGS) $*.cpp -v -E > $@

%.s: %.cpp
	$(CXX) $(CXXFLAGS) -O4 -S -pto $<  -o $@

