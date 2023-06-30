#
# Basename for libraries
#
LIB_BASENAME = libstlport_sunpro64

#
# This makefile will work for SUN CC 5.0-5.3 (Forte 6 Update 2)
#

CC = CC -xarch=v8plus
CXX = CC -xarch=v8plus

COMP = SUN64
ATOMIC_ASM = sparc_atomic.s

include sunpro-common.mak
