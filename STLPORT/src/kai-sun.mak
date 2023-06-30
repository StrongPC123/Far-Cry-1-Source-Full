#
# compiler
#
CC = KCC
CXX = $(CC)

#
# Basename for libraries
#
LIB_BASENAME = libstlport_kcc

#
# guts for common stuff
#
#

WARNING_FLAGS= --one_per --thread_safe --exceptions --abstract_pointer

CXXFLAGS_COMMON = -I${STLPORT_DIR} ${WARNING_FLAGS}

LINK=KCC ${CXXFLAGS_COMMON} -o
DYN_LINK=KCC ${CXXFLAGS_COMMON} -o

OBJEXT=o
DYNEXT=so
STEXT=a
RM=rm -rf
PATH_SEP=/
MKDIR=mkdir -p
COMP=KAI${ARCH}
INSTALL_STEP = install_unix 

all: sparc_atomic.o all_static all_dynamic symbolic_links

include common_macros.mak

CXXFLAGS_RELEASE_static = $(CXXFLAGS_COMMON) +K3 -O2
CXXFLAGS_RELEASE_dynamic = $(CXXFLAGS_COMMON) +K3 -O2  -KPIC

CXXFLAGS_DEBUG_static = $(CXXFLAGS_COMMON) +K0
CXXFLAGS_DEBUG_dynamic = $(CXXFLAGS_COMMON) +K0 -KPIC

CXXFLAGS_STLDEBUG_static = $(CXXFLAGS_DEBUG_static) -D_STLP_DEBUG
CXXFLAGS_STLDEBUG_dynamic = $(CXXFLAGS_DEBUG_dynamic) -D_STLP_DEBUG

include common_percent_rules.mak
include common_rules.mak

sparc_atomic.o : sparc_atomic.s
	${AS} sparc_atomic.s -o sparc_atomic.o
sparc_atomic64.o : sparc_atomic64.s
	${AS} sparc_atomic64.s -o sparc_atomic64.o

LDFLAGS_RELEASE_static = ${CXXFLAGS_RELEASE_static} 
LDFLAGS_RELEASE_dynamic = ${CXXFLAGS_RELEASE_dynamic} --soname ${RELEASE_DYNLIB_SONAME} sparc_atomic.o

LDFLAGS_DEBUG_static = ${CXXFLAGS_DEBUG_static} sparc_atomic.o
LDFLAGS_DEBUG_dynamic = ${CXXFLAGS_DEBUG_dynamic} --soname ${DEBUG_DYNLIB_SONAME} sparc_atomic.o

LDFLAGS_STLDEBUG_static = ${CXXFLAGS_STLDEBUG_static} sparc_atomic.o
LDFLAGS_STLDEBUG_dynamic = ${CXXFLAGS_STLDEBUG_dynamic} --soname ${STLDEBUG_DYNLIB_SONAME} sparc_atomic.o

# LDLIBS_RELEASE_dynamic =  -lposix4
# LDLIBS_STLDEBUG_dynamic = -lposix4



