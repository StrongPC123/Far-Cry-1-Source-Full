
install_dlls:
	@echo Copying STLport DLLs to Windows system directory...
	@if "%OS%" == "Windows_NT" copy $(OUTDIR)\$(LIB_BASENAME)*.dll $(WINDIR)\System32
	@if "%OS%" == "" copy $(OUTDIR)\$(LIB_BASENAME)*.dll $(WINDIR)\System
	@echo STLport DLL libraries are installed on your system.

install_libs_vc:
	@echo Copying STLport .lib files to VC lib directory : %MSVCDir%\lib...
	@copy $(OUTDIR)\$(LIB_BASENAME)*.lib "%MSVCDir%\lib"
	@echo Done copying .lib files.

install_headers_vc:
	@echo Copying STLport headers files to VC include directory : %MSVCDir%\include\stlport...
	@xcopy /Q /R /K /I /S ..\stlport "%MSVCDir%\include\stlport\"
	@echo Done copying STLport headers.

check_env_vc:
	@if not exist "%MSVCDir%\lib\*.*" echo "Please set up MSVCDir environment variable. Did you run vcvars32.bat ?"
	@if not exist "%MSVCDir%\lib\*.*" dir "%MSVCDir%\lib"

install_vc : all check_env_vc install_libs_vc install_dlls install_headers_vc report_dirs_vc

install_bc : all install_dlls

report_dirs_vc :
	@echo STLport installation complete.
	@echo Please find STLport headers in %MSVCDir%\include\stlport.
	@echo Please find STLport .lib files in %MSVCDir%\lib.
	@echo Please find STLport DLLs in Windows system directory.

{$(SRCDIR)}.cpp{$(RELEASE_OBJDIR_static)}.obj:
   $(CXX) $(CXXFLAGS_RELEASE_static) $< 

{$(SRCDIR)}.c{$(RELEASE_OBJDIR_static)}.obj:
   $(CC) $(CXXFLAGS_RELEASE_static) $< 

{$(SRCDIR)}.c{$(RELEASE_OBJDIR_static)}.sbr:
   $(CC) $(CXXFLAGS_RELEASE_static) $< 

{$(SRCDIR)}.cpp{$(RELEASE_OBJDIR_static)}.sbr:
   $(CXX) $(CXXFLAGS_RELEASE_static) $<


{$(SRCDIR)}.cpp{$(RELEASE_OBJDIR_staticx)}.obj:
   $(CXX) $(CXXFLAGS_RELEASE_staticx) $< 

{$(SRCDIR)}.c{$(RELEASE_OBJDIR_staticx)}.obj:
   $(CC) $(CXXFLAGS_RELEASE_staticx) $< 

{$(SRCDIR)}.c{$(RELEASE_OBJDIR_staticx)}.sbr:
   $(CC) $(CXXFLAGS_RELEASE_staticx) $< 

{$(SRCDIR)}.cpp{$(RELEASE_OBJDIR_staticx)}.sbr:
   $(CXX) $(CXXFLAGS_RELEASE_staticx) $<


{$(SRCDIR)}.cpp{$(RELEASE_OBJDIR_dynamic)}.obj:
   $(CXX) $(CXXFLAGS_RELEASE_dynamic) $< 

{$(SRCDIR)}.c{$(RELEASE_OBJDIR_dynamic)}.obj:
   $(CC) $(CXXFLAGS_RELEASE_dynamic) $< 

{$(SRCDIR)}.c{$(RELEASE_OBJDIR_dynamic)}.sbr:
   $(CC) $(CXXFLAGS_RELEASE_dynamic) $< 

{$(SRCDIR)}.cpp{$(RELEASE_OBJDIR_dynamic)}.sbr:
   $(CXX) $(CXXFLAGS_RELEASE_dynamic) $<

#########################################

{$(SRCDIR)}.cpp{$(DEBUG_OBJDIR_static)}.obj:
   $(CXX) $(CXXFLAGS_DEBUG_static) $<

{$(SRCDIR)}.c{$(DEBUG_OBJDIR_static)}.obj:
   $(CC) $(CXXFLAGS_DEBUG_static) $<

{$(SRCDIR)}.c{$(DEBUG_OBJDIR_static)}.sbr:
   $(CC) $(CXXFLAGS_DEBUG_static) $<

{$(SRCDIR)}.cpp{$(DEBUG_OBJDIR_static)}.sbr:
   $(CXX) $(CXXFLAGS_DEBUG_static) $<

{$(SRCDIR)}.cpp{$(DEBUG_OBJDIR_staticx)}.obj:
   $(CXX) $(CXXFLAGS_DEBUG_staticx) $<

{$(SRCDIR)}.c{$(DEBUG_OBJDIR_staticx)}.obj:
   $(CC) $(CXXFLAGS_DEBUG_staticx) $<

{$(SRCDIR)}.c{$(DEBUG_OBJDIR_staticx)}.sbr:
   $(CC) $(CXXFLAGS_DEBUG_staticx) $<

{$(SRCDIR)}.cpp{$(DEBUG_OBJDIR_staticx)}.sbr:
   $(CXX) $(CXXFLAGS_DEBUG_staticx) $<

{$(SRCDIR)}.cpp{$(DEBUG_OBJDIR_dynamic)}.obj:
   $(CXX) $(CXXFLAGS_DEBUG_dynamic) $<

{$(SRCDIR)}.c{$(DEBUG_OBJDIR_dynamic)}.obj:
   $(CC) $(CXXFLAGS_DEBUG_dynamic) $<

{$(SRCDIR)}.c{$(DEBUG_OBJDIR_dynamic)}.sbr:
   $(CC) $(CXXFLAGS_DEBUG_dynamic) $<

{$(SRCDIR)}.cpp{$(DEBUG_OBJDIR_dynamic)}.sbr:
   $(CXX) $(CXXFLAGS_DEBUG_dynamic) $<

######################################### 

{$(SRCDIR)}.cpp{$(STLDEBUG_OBJDIR_static)}.obj:
   $(CXX) $(CXXFLAGS_STLDEBUG_static) $<

{$(SRCDIR)}.c{$(STLDEBUG_OBJDIR_static)}.obj:
   $(CC) $(CXXFLAGS_STLDEBUG_static) $<

{$(SRCDIR)}.c{$(STLDEBUG_OBJDIR_static)}.sbr:
   $(CC) $(CXXFLAGS_STLDEBUG_static) $<

{$(SRCDIR)}.cpp{$(STLDEBUG_OBJDIR_static)}.sbr:
   $(CXX) $(CXXFLAGS_STLDEBUG_static) $<

{$(SRCDIR)}.cpp{$(STLDEBUG_OBJDIR_staticx)}.obj:
   $(CXX) $(CXXFLAGS_STLDEBUG_staticx) $<

{$(SRCDIR)}.c{$(STLDEBUG_OBJDIR_staticx)}.obj:
   $(CC) $(CXXFLAGS_STLDEBUG_staticx) $<

{$(SRCDIR)}.c{$(STLDEBUG_OBJDIR_staticx)}.sbr:
   $(CC) $(CXXFLAGS_STLDEBUG_staticx) $<

{$(SRCDIR)}.cpp{$(STLDEBUG_OBJDIR_staticx)}.sbr:
   $(CXX) $(CXXFLAGS_STLDEBUG_staticx) $<

{$(SRCDIR)}.cpp{$(STLDEBUG_OBJDIR_dynamic)}.obj:
   $(CXX) $(CXXFLAGS_STLDEBUG_dynamic) $<

{$(SRCDIR)}.c{$(STLDEBUG_OBJDIR_dynamic)}.obj:
   $(CC) $(CXXFLAGS_STLDEBUG_dynamic) $<

{$(SRCDIR)}.c{$(STLDEBUG_OBJDIR_dynamic)}.sbr:
   $(CC) $(CXXFLAGS_STLDEBUG_dynamic) $<

{$(SRCDIR)}.cpp{$(STLDEBUG_OBJDIR_dynamic)}.sbr:
   $(CXX) $(CXXFLAGS_STLDEBUG_dynamic) $<

############################################

{$(SRCDIR)}.rc{$(RELEASE_OBJDIR_dynamic)}.res:
	$(RC) /d COMP=$(COMP) /fo $(RELEASE_OBJDIR_dynamic)\stlport.res $<

{$(SRCDIR)}.rc{$(DEBUG_OBJDIR_dynamic)}.res:
	$(RC) /d COMP=$(COMP) /d BUILD=_DEBUG /fo $(DEBUG_OBJDIR_dynamic)\stlport.res $<

{$(SRCDIR)}.rc{$(STLDEBUG_OBJDIR_dynamic)}.res:
	$(RC) /d COMP=$(COMP) /d BUILD=_STLDEBUG /fo $(STLDEBUG_OBJDIR_dynamic)\stlport.res $<


