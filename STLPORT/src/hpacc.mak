#
# Makefile for HP aCC 1.23 compiler or later
#
CC = aCC
CXX = aCC

#
# Basename for libraries
#
LIB_BASENAME = libstlport_aCC

OPTIM=+O2 +Onolimit

#
# guts for common stuff
#
#
LINK=ar cr
# 2.95 flag
DYN_LINK=aCC -b ${OPTIM} +nostl -o

OBJEXT=o
DYNEXT=sl
STEXT=a
RM=rm -rf
PATH_SEP=/
MKDIR=mkdir -p
COMP=ACC$(ARCH)
INSTALL_STEP = install_unix 

all: all_dynamic all_static symbolic_links

include common_macros.mak

WARNING_FLAGS= 

# CXXFLAGS_COMMON = +ESlit -Aa -I${STLPORT_DIR} ${WARNING_FLAGS}
# CXXFLAGS_COMMON = -Aa -I${STLPORT_DIR} -D_REENTRANT +inst_close ${WARNING_FLAGS}
CXXFLAGS_COMMON = -AA -I${STLPORT_DIR} +inst_close ${WARNING_FLAGS} ${CXX_EXTRA_FLAGS}

CXXFLAGS_RELEASE_static = $(CXXFLAGS_COMMON) ${OPTIM} -z
CXXFLAGS_RELEASE_dynamic = $(CXXFLAGS_COMMON) ${OPTIM} -z +Z

CXXFLAGS_DEBUG_static = $(CXXFLAGS_COMMON) -g ${OPTIM} -z
CXXFLAGS_DEBUG_dynamic = $(CXXFLAGS_COMMON) -g ${OPTIM} -z +Z

CXXFLAGS_STLDEBUG_static = $(CXXFLAGS_DEBUG_static) -D_STLP_DEBUG
CXXFLAGS_STLDEBUG_dynamic = $(CXXFLAGS_DEBUG_dynamic) -D_STLP_DEBUG 

include common_percent_rules.mak
include common_rules.mak


#install: all
#	cp -p $(LIB_TARGET) ${D_LIB_TARGET} ../lib

#%.s: %.cpp
#	$(CXX) $(CXXFLAGS) -O4 -S -pto $<  -o $@
