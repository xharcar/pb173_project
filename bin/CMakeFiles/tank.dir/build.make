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
include CMakeFiles/tank.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/tank.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/tank.dir/flags.make

CMakeFiles/tank.dir/src/tankclient.cpp.o: CMakeFiles/tank.dir/flags.make
CMakeFiles/tank.dir/src/tankclient.cpp.o: ../src/tankclient.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/xharcar/pb173/pb173_project/bin/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/tank.dir/src/tankclient.cpp.o"
	/packages/run.64/gcc-4.8.2/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/tank.dir/src/tankclient.cpp.o -c /home/xharcar/pb173/pb173_project/src/tankclient.cpp

CMakeFiles/tank.dir/src/tankclient.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tank.dir/src/tankclient.cpp.i"
	/packages/run.64/gcc-4.8.2/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/xharcar/pb173/pb173_project/src/tankclient.cpp > CMakeFiles/tank.dir/src/tankclient.cpp.i

CMakeFiles/tank.dir/src/tankclient.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tank.dir/src/tankclient.cpp.s"
	/packages/run.64/gcc-4.8.2/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/xharcar/pb173/pb173_project/src/tankclient.cpp -o CMakeFiles/tank.dir/src/tankclient.cpp.s

CMakeFiles/tank.dir/src/tankclient.cpp.o.requires:
.PHONY : CMakeFiles/tank.dir/src/tankclient.cpp.o.requires

CMakeFiles/tank.dir/src/tankclient.cpp.o.provides: CMakeFiles/tank.dir/src/tankclient.cpp.o.requires
	$(MAKE) -f CMakeFiles/tank.dir/build.make CMakeFiles/tank.dir/src/tankclient.cpp.o.provides.build
.PHONY : CMakeFiles/tank.dir/src/tankclient.cpp.o.provides

CMakeFiles/tank.dir/src/tankclient.cpp.o.provides.build: CMakeFiles/tank.dir/src/tankclient.cpp.o

# Object files for target tank
tank_OBJECTS = \
"CMakeFiles/tank.dir/src/tankclient.cpp.o"

# External object files for target tank
tank_EXTERNAL_OBJECTS =

tank: CMakeFiles/tank.dir/src/tankclient.cpp.o
tank: CMakeFiles/tank.dir/build.make
tank: CMakeFiles/tank.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable tank"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/tank.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/tank.dir/build: tank
.PHONY : CMakeFiles/tank.dir/build

CMakeFiles/tank.dir/requires: CMakeFiles/tank.dir/src/tankclient.cpp.o.requires
.PHONY : CMakeFiles/tank.dir/requires

CMakeFiles/tank.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/tank.dir/cmake_clean.cmake
.PHONY : CMakeFiles/tank.dir/clean

CMakeFiles/tank.dir/depend:
	cd /home/xharcar/pb173/pb173_project/bin && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/xharcar/pb173/pb173_project /home/xharcar/pb173/pb173_project /home/xharcar/pb173/pb173_project/bin /home/xharcar/pb173/pb173_project/bin /home/xharcar/pb173/pb173_project/bin/CMakeFiles/tank.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/tank.dir/depend

