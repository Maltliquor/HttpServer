# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/http_server

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/http_server/build/Debug

# Include any dependencies generated for this target.
include src/test/CMakeFiles/demo_server.dir/depend.make

# Include the progress variables for this target.
include src/test/CMakeFiles/demo_server.dir/progress.make

# Include the compile flags for this target's objects.
include src/test/CMakeFiles/demo_server.dir/flags.make

src/test/CMakeFiles/demo_server.dir/demo_http_server.cpp.o: src/test/CMakeFiles/demo_server.dir/flags.make
src/test/CMakeFiles/demo_server.dir/demo_http_server.cpp.o: ../../src/test/demo_http_server.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/http_server/build/Debug/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/test/CMakeFiles/demo_server.dir/demo_http_server.cpp.o"
	cd /home/http_server/build/Debug/src/test && g++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/demo_server.dir/demo_http_server.cpp.o -c /home/http_server/src/test/demo_http_server.cpp

src/test/CMakeFiles/demo_server.dir/demo_http_server.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/demo_server.dir/demo_http_server.cpp.i"
	cd /home/http_server/build/Debug/src/test && g++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/http_server/src/test/demo_http_server.cpp > CMakeFiles/demo_server.dir/demo_http_server.cpp.i

src/test/CMakeFiles/demo_server.dir/demo_http_server.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/demo_server.dir/demo_http_server.cpp.s"
	cd /home/http_server/build/Debug/src/test && g++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/http_server/src/test/demo_http_server.cpp -o CMakeFiles/demo_server.dir/demo_http_server.cpp.s

src/test/CMakeFiles/demo_server.dir/demo_http_server.cpp.o.requires:
.PHONY : src/test/CMakeFiles/demo_server.dir/demo_http_server.cpp.o.requires

src/test/CMakeFiles/demo_server.dir/demo_http_server.cpp.o.provides: src/test/CMakeFiles/demo_server.dir/demo_http_server.cpp.o.requires
	$(MAKE) -f src/test/CMakeFiles/demo_server.dir/build.make src/test/CMakeFiles/demo_server.dir/demo_http_server.cpp.o.provides.build
.PHONY : src/test/CMakeFiles/demo_server.dir/demo_http_server.cpp.o.provides

src/test/CMakeFiles/demo_server.dir/demo_http_server.cpp.o.provides.build: src/test/CMakeFiles/demo_server.dir/demo_http_server.cpp.o

# Object files for target demo_server
demo_server_OBJECTS = \
"CMakeFiles/demo_server.dir/demo_http_server.cpp.o"

# External object files for target demo_server
demo_server_EXTERNAL_OBJECTS =

bin/demo_server: src/test/CMakeFiles/demo_server.dir/demo_http_server.cpp.o
bin/demo_server: src/test/CMakeFiles/demo_server.dir/build.make
bin/demo_server: lib/libserverlib_net.a
bin/demo_server: lib/libserverlib_base.a
bin/demo_server: src/test/CMakeFiles/demo_server.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable ../../bin/demo_server"
	cd /home/http_server/build/Debug/src/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/demo_server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/test/CMakeFiles/demo_server.dir/build: bin/demo_server
.PHONY : src/test/CMakeFiles/demo_server.dir/build

src/test/CMakeFiles/demo_server.dir/requires: src/test/CMakeFiles/demo_server.dir/demo_http_server.cpp.o.requires
.PHONY : src/test/CMakeFiles/demo_server.dir/requires

src/test/CMakeFiles/demo_server.dir/clean:
	cd /home/http_server/build/Debug/src/test && $(CMAKE_COMMAND) -P CMakeFiles/demo_server.dir/cmake_clean.cmake
.PHONY : src/test/CMakeFiles/demo_server.dir/clean

src/test/CMakeFiles/demo_server.dir/depend:
	cd /home/http_server/build/Debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/http_server /home/http_server/src/test /home/http_server/build/Debug /home/http_server/build/Debug/src/test /home/http_server/build/Debug/src/test/CMakeFiles/demo_server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/test/CMakeFiles/demo_server.dir/depend
