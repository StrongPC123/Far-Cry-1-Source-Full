
.SUFFIXES: .cpp .c .o .so .a

$(RELEASE_OBJDIR_static)/%.o: %.cpp
	$(CXX) $(CXXFLAGS_RELEASE_static) $< -c -o $@
$(RELEASE_OBJDIR_static)/%.o: %.c
	$(CC) $(CXXFLAGS_RELEASE_static) $< -c -o $@
$(RELEASE_OBJDIR_static)/%.i : %.cpp
	$(CXX) $(CXXFLAGS_RELEASE_static) $< -E  $@

$(RELEASE_OBJDIR_dynamic)/%.o : %.cpp
	$(CXX) $(CXXFLAGS_RELEASE_dynamic) $< -c -o $@
$(RELEASE_OBJDIR_dynamic)/%.o : %.c
	$(CC) $(CXXFLAGS_RELEASE_dynamic) $< -c -o $@
$(RELEASE_OBJDIR_dynamic)/%.i : %.cpp
	$(CXX) $(CXXFLAGS_RELEASE_dynamic) $< -E  $@

$(DEBUG_OBJDIR_static)/%.o : %.cpp
	$(CXX) $(CXXFLAGS_DEBUG_static) $< -c -o $@
$(DEBUG_OBJDIR_static)/%.o : %.c
	$(CC) $(CXXFLAGS_DEBUG_static) $< -c -o $@
$(DEBUG_OBJDIR_static)/%.i : %.cpp
	$(CXX) $(CXXFLAGS_DEBUG_static) $< -E  $@

$(DEBUG_OBJDIR_dynamic)/%.o : %.cpp
	$(CXX) $(CXXFLAGS_DEBUG_dynamic) $< -c -o $@
$(DEBUG_OBJDIR_dynamic)/%.o : %.c
	$(CC) $(CXXFLAGS_DEBUG_dynamic) $< -c -o $@
$(DEBUG_OBJDIR_dynamic)/%.i : %.cpp
	$(CXX) $(CXXFLAGS_DEBUG_dynamic) $< -E  $@

$(STLDEBUG_OBJDIR_static)/%.o : %.cpp
	$(CXX) $(CXXFLAGS_STLDEBUG_static) $< -c -o $@
$(STLDEBUG_OBJDIR_static)/%.o : %.c
	$(CC) $(CXXFLAGS_STLDEBUG_static) $< -c -o $@
$(STLDEBUG_OBJDIR_static)/%.i : %.cpp
	$(CXX) $(CXXFLAGS_STLDEBUG_static) $< -E  $@

$(STLDEBUG_OBJDIR_dynamic)/%.o : %.cpp
	$(CXX) $(CXXFLAGS_STLDEBUG_dynamic) $< -c -o $@
$(STLDEBUG_OBJDIR_dynamic)/%.o : %.c
	$(CC) $(CXXFLAGS_STLDEBUG_dynamic) $< -c -o $@
$(STLDEBUG_OBJDIR_dynamic)/%.i : %.cpp
	$(CXX) $(CXXFLAGS_STLDEBUG_dynamic) $< -E  $@


