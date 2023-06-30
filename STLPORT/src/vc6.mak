#
# Tools
#

CXX=cl.exe
CC=cl.exe
RC=rc

LIB_BASENAME=stlport_vc6
COMP=VC6

# EXTRA_COMMON_FLAGS=/D "_MBCS"
EXTRA_COMMON_FLAGS=/Zm800 /FI "vc_warning_disable.h" /D "_MBCS"
EXTRA_DEBUG_FLAGS=
EXTRA_NDEBUG_FLAGS=

all: platform all_static all_dynamic 

!INCLUDE vc_common.mak

#
#



