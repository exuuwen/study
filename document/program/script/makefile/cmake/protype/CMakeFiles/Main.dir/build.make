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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/local/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/cr7/tmp/protype

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/cr7/tmp/protype

# Include any dependencies generated for this target.
include CMakeFiles/Main.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/Main.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Main.dir/flags.make

CMakeFiles/Main.dir/main.cxx.o: CMakeFiles/Main.dir/flags.make
CMakeFiles/Main.dir/main.cxx.o: main.cxx
	$(CMAKE_COMMAND) -E cmake_progress_report /home/cr7/tmp/protype/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/Main.dir/main.cxx.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/Main.dir/main.cxx.o -c /home/cr7/tmp/protype/main.cxx

CMakeFiles/Main.dir/main.cxx.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Main.dir/main.cxx.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/cr7/tmp/protype/main.cxx > CMakeFiles/Main.dir/main.cxx.i

CMakeFiles/Main.dir/main.cxx.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Main.dir/main.cxx.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/cr7/tmp/protype/main.cxx -o CMakeFiles/Main.dir/main.cxx.s

CMakeFiles/Main.dir/main.cxx.o.requires:
.PHONY : CMakeFiles/Main.dir/main.cxx.o.requires

CMakeFiles/Main.dir/main.cxx.o.provides: CMakeFiles/Main.dir/main.cxx.o.requires
	$(MAKE) -f CMakeFiles/Main.dir/build.make CMakeFiles/Main.dir/main.cxx.o.provides.build
.PHONY : CMakeFiles/Main.dir/main.cxx.o.provides

CMakeFiles/Main.dir/main.cxx.o.provides.build: CMakeFiles/Main.dir/main.cxx.o

# Object files for target Main
Main_OBJECTS = \
"CMakeFiles/Main.dir/main.cxx.o"

# External object files for target Main
Main_EXTERNAL_OBJECTS =

Main: CMakeFiles/Main.dir/main.cxx.o
Main: CMakeFiles/Main.dir/build.make
Main: Hello/libHello.a
Main: Demo/libDemo.a
Main: CMakeFiles/Main.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable Main"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Main.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Main.dir/build: Main
.PHONY : CMakeFiles/Main.dir/build

CMakeFiles/Main.dir/requires: CMakeFiles/Main.dir/main.cxx.o.requires
.PHONY : CMakeFiles/Main.dir/requires

CMakeFiles/Main.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Main.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Main.dir/clean

CMakeFiles/Main.dir/depend:
	cd /home/cr7/tmp/protype && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/cr7/tmp/protype /home/cr7/tmp/protype /home/cr7/tmp/protype /home/cr7/tmp/protype /home/cr7/tmp/protype/CMakeFiles/Main.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Main.dir/depend

