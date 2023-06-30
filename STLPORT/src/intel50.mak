#
# Makefile for Intel C++ compiler v.5.0

#
# Tools
#

CXX=icl.exe
CC=icl.exe
RC=rc

# Intel frontend tools
# LINK=xilib.exe

DYN_LINK=xilink.exe


LIB_BASENAME=stlport_icl
COMP=ICL

EXTRA_COMMON_FLAGS= -D_MBCS -Qwd186,985
EXTRA_DEBUG_FLAGS=
EXTRA_NDEBUG_FLAGS= -Qsox-

all: platform all_static all_dynamic

!INCLUDE vc_common.mak

#
#



