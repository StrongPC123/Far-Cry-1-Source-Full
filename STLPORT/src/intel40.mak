#
# Tools
#

CXX=icl.exe
CC=icl.exe
RC=rc

LIB_BASENAME=stlport_icl

# EXTRA_COMMON_FLAGS=/D "_MBCS"
EXTRA_COMMON_FLAGS= -D_MBCS
EXTRA_DEBUG_FLAGS=
EXTRA_NDEBUG_FLAGS= -Qipo -Qsox-

COMP=ICL

all: all_static

!INCLUDE vc_common.mak

#
#



