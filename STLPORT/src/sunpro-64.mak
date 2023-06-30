#
# Basename for libraries
#
LIB_BASENAME = libstlport_sunpro64

#
# This makefile will work for SUN CC 5.0-5.3 (Forte 6 Update 2)
#

CC = CC -xarch=v9
CXX = CC -xarch=v9

COMP = SUN64
ATOMIC_ASM = sparc_atomic64.s

include sunpro-common.mak
