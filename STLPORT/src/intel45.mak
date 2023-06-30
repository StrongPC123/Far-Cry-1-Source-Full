#
# Makefile for Intel C++ compiler v.4.5 

#
# Tools
#
CXX=icl.exe
CC=icl.exe
# Intel frontend tools
# LINK=xilib.exe

DYN_LINK=xilink.exe


LIB_BASENAME=stlport_icl
COMP=ICL

EXTRA_COMMON_FLAGS= -D_MBCS -Qwd186
EXTRA_DEBUG_FLAGS=
EXTRA_NDEBUG_FLAGS= -Qsox-

all: platform all_dynamic all_static 

!INCLUDE vc_common.mak

#
#



