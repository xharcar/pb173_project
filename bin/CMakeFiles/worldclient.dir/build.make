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
CMAKE_COMMAND = /packages/run.64/cmake-2.8.8/bin/cmake

# The command to remove a file.
RM = /packages/run.64/cmake-2.8.8/bin/cmake -E remove -f

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /packages/run.64/cmake-2.8.8/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/xharcar/pb173/pb173_project

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/xharcar/pb173/pb173_project/bin

# Include any dependencies generated for this target.
include CMakeFiles/worldclient.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/worldclient.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/worldclient.dir/flags.make

CMakeFiles/worldclient.dir/src/worldclient.cpp.o: CMakeFiles/worldclient.dir/flags.make
CMakeFiles/worldclient.dir/src/worldclient.cpp.o: ../src/worldclient.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/xharcar/pb173/pb173_project/bin/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/worldclient.dir/src/worldclient.cpp.o"
	/packages/run.64/gcc-4.8.2/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/worldclient.dir/src/worldclient.cpp.o -c /home/xharcar/pb173/pb173_project/src/worldclient.cpp

CMakeFiles/worldclient.dir/src/worldclient.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/worldclient.dir/src/worldclient.cpp.i"
	/packages/run.64/gcc-4.8.2/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/xharcar/pb173/pb173_project/src/worldclient.cpp > CMakeFiles/worldclient.dir/src/worldclient.cpp.i

CMakeFiles/worldclient.dir/src/worldclient.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/worldclient.dir/src/worldclient.cpp.s"
	/packages/run.64/gcc-4.8.2/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/xharcar/pb173/pb173_project/src/worldclient.cpp -o CMakeFiles/worldclient.dir/src/worldclient.cpp.s

CMakeFiles/worldclient.dir/src/worldclient.cpp.o.requires:
.PHONY : CMakeFiles/worldclient.dir/src/worldclient.cpp.o.requires

CMakeFiles/worldclient.dir/src/worldclient.cpp.o.provides: CMakeFiles/worldclient.dir/src/worldclient.cpp.o.requires
	$(MAKE) -f CMakeFiles/worldclient.dir/build.make CMakeFiles/worldclient.dir/src/worldclient.cpp.o.provides.build
.PHONY : CMakeFiles/worldclient.dir/src/worldclient.cpp.o.provides

CMakeFiles/worldclient.dir/src/worldclient.cpp.o.provides.build: CMakeFiles/worldclient.dir/src/worldclient.cpp.o

# Object files for target worldclient
worldclient_OBJECTS = \
"CMakeFiles/worldclient.dir/src/worldclient.cpp.o"

# External object files for target worldclient
worldclient_EXTERNAL_OBJECTS =

worldclient: CMakeFiles/worldclient.dir/src/worldclient.cpp.o
worldclient: CMakeFiles/worldclient.dir/build.make
worldclient: CMakeFiles/worldclient.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable worldclient"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/worldclient.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/worldclient.dir/build: worldclient
.PHONY : CMakeFiles/worldclient.dir/build

CMakeFiles/worldclient.dir/requires: CMakeFiles/worldclient.dir/src/worldclient.cpp.o.requires
.PHONY : CMakeFiles/worldclient.dir/requires

CMakeFiles/worldclient.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/worldclient.dir/cmake_clean.cmake
.PHONY : CMakeFiles/worldclient.dir/clean

CMakeFiles/worldclient.dir/depend:
	cd /home/xharcar/pb173/pb173_project/bin && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/xharcar/pb173/pb173_project /home/xharcar/pb173/pb173_project /home/xharcar/pb173/pb173_project/bin /home/xharcar/pb173/pb173_project/bin /home/xharcar/pb173/pb173_project/bin/CMakeFiles/worldclient.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/worldclient.dir/depend

