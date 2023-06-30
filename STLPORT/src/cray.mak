#
# Note : This makefile is for Cray C++ 3.4 and 3.5
#
#        The GNU version of make must be used to execute the
#	 makefile.  The UNICOS version of make does not support
#	 the "%.suffix1: %.suffix2" feature.  
#
#	 GNU version of make is available from the Cray Open
#	 Source release, or can be downloaded from various of
#	 open source sites.

#
# compiler
#
CC = cc 
CXX = CC

#
# Basename for libraries
#
LIB_BASENAME = libstlport

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
COMP=CRAY$(ARCH)

all:  all_static all_staticx

include common_macros.mak

CXXFLAGS_COMMON = -I${STLPORT_DIR}
CXXFLAGS_RELEASE_static = $(CXXFLAGS_COMMON) -D_STLP_HAS_NO_EXCEPTIONS
CXXFLAGS_RELEASE_staticx = $(CXXFLAGS_COMMON) -hexceptions
CXXFLAGS_DEBUG_static = $(CXXFLAGS_COMMON) -g
CXXFLAGS_STLDEBUG_static = $(CXXFLAGS_DEBUG_static) -D_STLP_DEBUG -D_STLP_HAS_NO_EXCEPTIONS
CXXFLAGS_STLDEBUG_staticx = $(CXXFLAGS_DEBUG_static) -D_STLP_DEBUG -hexceptions

include common_percent_rules.mak

# common_percent_rules.mak does not contain makerules for staticx source
$(RELEASE_OBJDIR_staticx)/%.o: %.cpp
	$(CXX) $(CXXFLAGS_RELEASE_staticx) $< -c -o $@
$(RELEASE_OBJDIR_staticx)/%.o: %.c
	$(CC) $(CXXFLAGS_RELEASE_staticx) $< -c -o $@
$(RELEASE_OBJDIR_staticx)/%.i : %.cpp
	$(CXX) $(CXXFLAGS_RELEASE_staticx) $< -E  $@

$(STLDEBUG_OBJDIR_staticx)/%.o : %.cpp
	$(CXX) $(CXXFLAGS_STLDEBUG_staticx) $< -c -o $@
$(STLDEBUG_OBJDIR_staticx)/%.o : %.c
	$(CC) $(CXXFLAGS_STLDEBUG_staticx) $< -c -o $@
$(STLDEBUG_OBJDIR_staticx)/%.i : %.cpp
	$(CXX) $(CXXFLAGS_STLDEBUG_staticx) $< -E  $@

#include common_rules.mak
#
#  Target directories
#
#

$(OUTDIR) :
	$(MKDIR) $(OUTDIR)
$(RELEASE_OBJDIR_static) :
	$(MKDIR) $(RELEASE_OBJDIR_static)
$(DEBUG_OBJDIR_static) :
	$(MKDIR) $(DEBUG_OBJDIR_static)
$(STLDEBUG_OBJDIR_static) :
	$(MKDIR) $(STLDEBUG_OBJDIR_static)
$(RELEASE_OBJDIR_staticx) :
	$(MKDIR) $(RELEASE_OBJDIR_staticx)
$(DEBUG_OBJDIR_staticx) :
	$(MKDIR) $(DEBUG_OBJDIR_staticx)
$(STLDEBUG_OBJDIR_staticx) :
	$(MKDIR) $(STLDEBUG_OBJDIR_staticx)

#create a compiler platform directory
platform: $(PREPARE_STEP)
	-@$(MKDIR) $(OUTDIR)
	-@$(MKDIR) $(OBJDIR_COMMON)
	-@$(MKDIR) $(OBJDIR)

clean_all_obj:
	-$(RM) $(OUTDIR)$(PATH_SEP)obj

######   Targets ##################

all_static :   platform $(ALL_STATIC_LIBS)

all_staticx :	platform $(ALL_STATICX_LIBS) 

release_static :  platform $(OUTDIR)$(PATH_SEP)$(RELEASE_LIB)

debug_static :  platform $(OUTDIR)$(PATH_SEP)$(DEBUG_LIB)

stldebug_static :  platform $(OUTDIR)$(PATH_SEP)$(STLDEBUG_LIB)

release_staticx :  platform $(OUTDIR)$(PATH_SEP)$(RELEASEX_LIB)
 
debug_staticx :  platform $(OUTDIR)$(PATH_SEP)$(DEBUGX_LIB)

stldebug_staticx :  platform $(OUTDIR)$(PATH_SEP)$(STLDEBUGX_LIB)

install :  all $(INSTALL_STEP)

clean : $(CLEAN_SPECIFIC)
	-$(RM) $(RELEASE_OBJDIR_static) $(DEBUG_OBJDIR_static) $(STLDEBUG_OBJDIR_static) \
        $(RELEASE_OBJDIR_staticx) $(DEBUG_OBJDIR_staticx) $(STLDEBUG_OBJDIR_staticx) 

clobber : clean clean_all_obj
	-$(RM) $(OUTDIR)$(PATH_SEP)$(DEBUG_NAME).* \
               $(OUTDIR)$(PATH_SEP)$(RELEASE_NAME).* $(OUTDIR)$(PATH_SEP)$(STLDEBUG_NAME).*

# Need to link with a dummy main process to fully instantiate object files.

$(OUTDIR)$(PATH_SEP)$(RELEASE_LIB) : $(OUTDIR) $(RELEASE_OBJDIR_static) $(DEF_FILE) $(RELEASE_OBJECTS_static)
	echo 'main() { }' >dummy_main.C
	$(CXX) -o junk dummy_main.C $(RELEASE_OBJECTS_static) -lpthread
	rm dummy_main.C
	$(LINK) $(LINK_OUT)$(OUTDIR)$(PATH_SEP)$(RELEASE_LIB) $(LDFLAGS_RELEASE_static) $(RELEASE_OBJECTS_static) $(LDLIBS_RELEASE_static)
	
$(OUTDIR)$(PATH_SEP)$(RELEASEX_LIB) : $(OUTDIR) $(RELEASE_OBJDIR_staticx) $(DEF_FILE) $(RELEASE_OBJECTS_staticx)
	echo 'main() { }' >dummy_main.C
	$(CXX) -o junk dummy_main.C $(RELEASE_OBJECTS_staticx) -lpthread
	rm dummy_main.C
	$(LINK) $(LINK_OUT)$(OUTDIR)$(PATH_SEP)$(RELEASEX_LIB) $(LDFLAGS_RELEASE_static) $(RELEASE_OBJECTS_staticx) $(LDLIBS_RELEASE_static)

$(OUTDIR)$(PATH_SEP)$(STLDEBUG_LIB) : $(OUTDIR) $(STLDEBUG_OBJDIR_static) $(DEF_FILE) $(STLDEBUG_OBJECTS_static)
	echo 'main() { }' >dummy_main.C
	$(CXX) -o junk dummy_main.C $(STLDEBUG_OBJECTS_static) -lpthread
	rm dummy_main.C
	$(LINK) $(LINK_OUT)$(OUTDIR)$(PATH_SEP)$(STLDEBUG_LIB) $(LDFLAGS_STLDEBUG_static) $(STLDEBUG_OBJECTS_static)  $(LDLIBS_STLDEBUG_static)

$(OUTDIR)$(PATH_SEP)$(STLDEBUGX_LIB) : $(OUTDIR) $(STLDEBUG_OBJDIR_staticx) $(DEF_FILE) $(STLDEBUG_OBJECTS_staticx)
	echo 'main() { }' >dummy_main.C
	$(CXX) -o junk dummy_main.C $(STLDEBUG_OBJECTS_staticx) -lpthread
	rm dummy_main.C
	$(LINK) $(LINK_OUT)$(OUTDIR)$(PATH_SEP)$(STLDEBUGX_LIB) $(LDFLAGS_STLDEBUG_static) $(STLDEBUG_OBJECTS_staticx)  $(LDLIBS_STLDEBUG_static)


#install: all
#	cp -p $(LIB_TARGET) ${D_LIB_TARGET} ../lib

%.s: %.cpp
	$(CXX) $(CXXFLAGS) -S  $<  -o $@


