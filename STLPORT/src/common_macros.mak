SHELL=/bin/sh

#
# Versioning
#
VERSION_MAJOR=4
VERSION_MINOR=6

# This one is not always present; it should be empty for release versions.
# We do not assume any binary compatibility for betas.
BETA_SUFFIX=

VERSION_SUFFIX= .$(VERSION_MAJOR).$(VERSION_MINOR)$(BETA_SUFFIX)
WIN_VERSION_SUFFIX= $(VERSION_MAJOR)$(VERSION_MINOR)$(BETA_SUFFIX)

# DYNAMIC_SUFFIX= $(VERSION_SUFFIX)$(PATCH_SUFFIX)
DYNAMIC_SUFFIX= $(VERSION_SUFFIX)
WIN_DYNAMIC_SUFFIX= $(WIN_VERSION_SUFFIX)

#
#  Directories
#
#

SRCDIR=.
STLPORT_DIR=$(CUR_DIR)..$(PATH_SEP)stlport
OUTDIR=$(CUR_DIR)..$(PATH_SEP)lib
OBJDIR_COMMON=$(OUTDIR)$(PATH_SEP)obj
OBJDIR=$(OUTDIR)$(PATH_SEP)obj$(PATH_SEP)$(COMP)

RELEASE_OBJDIR_static=$(OBJDIR)$(PATH_SEP)Release
DEBUG_OBJDIR_static=$(OBJDIR)$(PATH_SEP)Debug
STLDEBUG_OBJDIR_static=$(OBJDIR)$(PATH_SEP)DebugSTL
RELEASE_OBJDIR_staticx=$(OBJDIR)$(PATH_SEP)Releasex
DEBUG_OBJDIR_staticx=$(OBJDIR)$(PATH_SEP)Debugx
STLDEBUG_OBJDIR_staticx=$(OBJDIR)$(PATH_SEP)DebugSTLx
RELEASE_OBJDIR_dynamic=$(OBJDIR)$(PATH_SEP)ReleaseD
DEBUG_OBJDIR_dynamic=$(OBJDIR)$(PATH_SEP)DebugD
STLDEBUG_OBJDIR_dynamic=$(OBJDIR)$(PATH_SEP)DebugSTLD

#
# By default on UNIX, STLport headers go into /usr/local/include/stlport,
# and libraries go into /usr/local/lib. Please override any of them if desired. 
#

INSTALLDIR=/usr/local
# INSTALLDIR=/tmp
INSTALLDIR_INC=$(INSTALLDIR)/include/stlport
INSTALLDIR_LIB=$(INSTALLDIR)/lib

RM = rm -fr
INSTALL = ./install.sh -c
INSTALL_LIB = $(INSTALL)
INSTALL_H = $(INSTALL) -m 444

#
#
# Targets
#

RELEASE_NAME=$(LIB_BASENAME)
DEBUG_NAME=$(LIB_BASENAME)_debug
STLDEBUG_NAME=$(LIB_BASENAME)_stldebug

RELEASE_DYNLIB=$(RELEASE_NAME).$(DYNEXT)$(DYNAMIC_SUFFIX)
DEBUG_DYNLIB=$(DEBUG_NAME).$(DYNEXT)$(DYNAMIC_SUFFIX)
STLDEBUG_DYNLIB=$(STLDEBUG_NAME).$(DYNEXT)$(DYNAMIC_SUFFIX)

RELEASE_DYNLIB_SONAME=$(RELEASE_NAME).$(DYNEXT)$(VERSION_SUFFIX)
DEBUG_DYNLIB_SONAME=$(DEBUG_NAME).$(DYNEXT)$(VERSION_SUFFIX)
STLDEBUG_DYNLIB_SONAME=$(STLDEBUG_NAME).$(DYNEXT)$(VERSION_SUFFIX)

RELEASE_LIB=$(RELEASE_NAME)$(STATIC_SUFFIX).$(STEXT)
DEBUG_LIB=$(DEBUG_NAME)$(STATIC_SUFFIX).$(STEXT)
STLDEBUG_LIB=$(STLDEBUG_NAME)$(STATIC_SUFFIX).$(STEXT)

RELEASEX_LIB=$(RELEASE_NAME)$(STATIC_SUFFIX)x.$(STEXT)
DEBUGX_LIB=$(DEBUG_NAME)$(STATIC_SUFFIX)x.$(STEXT)
STLDEBUGX_LIB=$(STLDEBUG_NAME)$(STATIC_SUFFIX)x.$(STEXT)

ALL_STATIC_LIBS=$(OUTDIR)$(PATH_SEP)$(RELEASE_LIB) $(OUTDIR)$(PATH_SEP)$(STLDEBUG_LIB)
DEBUG_STATIC_LIBS=$(ALL_STATIC_LIBS) $(OUTDIR)$(PATH_SEP)$(DEBUG_LIB)

ALL_STATICX_LIBS=$(OUTDIR)$(PATH_SEP)$(RELEASEX_LIB) $(OUTDIR)$(PATH_SEP)$(STLDEBUGX_LIB)
DEBUG_STATICX_LIBS=$(ALL_STATICX_LIBS) $(OUTDIR)$(PATH_SEP)$(DEBUGX_LIB)

ALL_DYNAMIC_LIBS=$(OUTDIR)$(PATH_SEP)$(RELEASE_DYNLIB) $(OUTDIR)$(PATH_SEP)$(STLDEBUG_DYNLIB)
DEBUG_DYNAMIC_LIBS=$(OUTDIR)$(PATH_SEP)$(RELEASE_DYNLIB) $(OUTDIR)$(PATH_SEP)$(STLDEBUG_DYNLIB)


RELEASE_OBJECTS_static= \
$(RELEASE_OBJDIR_static)$(PATH_SEP)dll_main.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)fstream.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)strstream.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)sstream.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)ios.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)streambuf.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)stdio_streambuf.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)istream.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)ostream.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)iostream.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)codecvt.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)collate.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)ctype.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)monetary.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)num_get.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)num_put.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)num_get_float.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)num_put_float.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)numpunct.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)time_facets.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)messages.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)locale_impl.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)locale.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)locale_catalog.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)facets_byname.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)c_locale.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)c_locale_stub.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)complex.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)complex_exp.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)complex_io.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)complex_trig.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)complex_io_w.$(OBJEXT) \
$(RELEASE_OBJDIR_static)$(PATH_SEP)string_w.$(OBJEXT)


DEBUG_OBJECTS_static= \
$(DEBUG_OBJDIR_static)$(PATH_SEP)dll_main.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)fstream.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)strstream.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)sstream.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)ios.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)streambuf.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)stdio_streambuf.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)istream.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)ostream.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)iostream.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)codecvt.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)collate.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)ctype.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)monetary.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)num_get.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)num_put.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)num_get_float.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)num_put_float.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)numpunct.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)time_facets.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)messages.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)locale_impl.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)locale.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)locale_catalog.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)facets_byname.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)c_locale.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)c_locale_stub.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)complex.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)complex_exp.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)complex_io.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)complex_trig.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)complex_io_w.$(OBJEXT) \
$(DEBUG_OBJDIR_static)$(PATH_SEP)string_w.$(OBJEXT)

STLDEBUG_OBJECTS_static= \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)dll_main.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)fstream.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)strstream.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)sstream.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)ios.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)streambuf.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)stdio_streambuf.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)istream.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)ostream.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)iostream.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)codecvt.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)collate.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)ctype.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)monetary.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)num_get.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)num_put.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)num_get_float.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)num_put_float.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)numpunct.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)time_facets.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)messages.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)locale_impl.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)locale.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)locale_catalog.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)facets_byname.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)c_locale.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)c_locale_stub.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)complex.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)complex_exp.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)complex_io.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)complex_trig.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)complex_io_w.$(OBJEXT) \
$(STLDEBUG_OBJDIR_static)$(PATH_SEP)string_w.$(OBJEXT)

###########################################################

RELEASE_OBJECTS_staticx= \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)dll_main.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)fstream.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)strstream.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)sstream.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)ios.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)streambuf.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)stdio_streambuf.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)istream.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)ostream.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)iostream.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)codecvt.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)collate.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)ctype.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)monetary.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)num_get.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)num_put.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)num_get_float.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)num_put_float.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)numpunct.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)time_facets.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)messages.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)locale_impl.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)locale.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)locale_catalog.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)facets_byname.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)c_locale.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)c_locale_stub.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)complex.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)complex_exp.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)complex_io.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)complex_trig.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)complex_io_w.$(OBJEXT) \
$(RELEASE_OBJDIR_staticx)$(PATH_SEP)string_w.$(OBJEXT)


DEBUG_OBJECTS_staticx= \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)dll_main.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)fstream.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)strstream.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)sstream.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)ios.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)streambuf.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)stdio_streambuf.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)istream.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)ostream.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)iostream.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)codecvt.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)collate.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)ctype.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)monetary.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)num_get.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)num_put.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)num_get_float.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)num_put_float.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)numpunct.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)time_facets.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)messages.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)locale_impl.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)locale.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)locale_catalog.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)facets_byname.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)c_locale.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)c_locale_stub.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)complex.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)complex_exp.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)complex_io.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)complex_trig.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)complex_io_w.$(OBJEXT) \
$(DEBUG_OBJDIR_staticx)$(PATH_SEP)string_w.$(OBJEXT)

STLDEBUG_OBJECTS_staticx= \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)dll_main.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)fstream.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)strstream.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)sstream.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)ios.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)streambuf.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)stdio_streambuf.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)istream.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)ostream.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)iostream.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)codecvt.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)collate.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)ctype.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)monetary.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)num_get.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)num_put.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)num_get_float.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)num_put_float.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)numpunct.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)time_facets.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)messages.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)locale_impl.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)locale.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)locale_catalog.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)facets_byname.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)c_locale.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)c_locale_stub.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)complex.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)complex_exp.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)complex_io.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)complex_trig.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)complex_io_w.$(OBJEXT) \
$(STLDEBUG_OBJDIR_staticx)$(PATH_SEP)string_w.$(OBJEXT)


###########################################################

RELEASE_OBJECTS_dynamic= \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)dll_main.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)fstream.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)strstream.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)sstream.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)ios.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)streambuf.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)stdio_streambuf.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)istream.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)ostream.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)iostream.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)codecvt.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)collate.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)ctype.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)monetary.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)num_get.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)num_put.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)num_get_float.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)num_put_float.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)numpunct.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)time_facets.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)messages.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)locale_impl.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)locale.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)locale_catalog.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)facets_byname.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)c_locale.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)c_locale_stub.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)complex.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)complex_exp.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)complex_io.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)complex_trig.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)complex_io_w.$(OBJEXT) \
$(RELEASE_OBJDIR_dynamic)$(PATH_SEP)string_w.$(OBJEXT) \
$(RESFILE)

DEBUG_OBJECTS_dynamic= \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)dll_main.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)fstream.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)strstream.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)sstream.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)ios.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)streambuf.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)stdio_streambuf.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)istream.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)ostream.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)iostream.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)codecvt.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)collate.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)ctype.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)monetary.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)num_get.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)num_put.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)num_get_float.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)num_put_float.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)numpunct.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)time_facets.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)messages.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)locale_impl.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)locale.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)locale_catalog.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)facets_byname.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)c_locale.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)c_locale_stub.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)complex.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)complex_exp.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)complex_io.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)complex_trig.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)complex_io_w.$(OBJEXT) \
$(DEBUG_OBJDIR_dynamic)$(PATH_SEP)string_w.$(OBJEXT) \
$(RESFILE_debug)

STLDEBUG_OBJECTS_dynamic= \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)dll_main.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)fstream.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)strstream.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)sstream.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)ios.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)streambuf.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)stdio_streambuf.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)istream.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)ostream.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)iostream.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)codecvt.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)collate.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)ctype.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)monetary.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)num_get.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)num_put.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)num_get_float.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)num_put_float.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)numpunct.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)time_facets.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)messages.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)locale_impl.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)locale.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)locale_catalog.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)facets_byname.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)c_locale.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)c_locale_stub.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)complex.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)complex_exp.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)complex_io.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)complex_trig.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)complex_io_w.$(OBJEXT) \
$(STLDEBUG_OBJDIR_dynamic)$(PATH_SEP)string_w.$(OBJEXT) \
$(RESFILE_stldebug)


