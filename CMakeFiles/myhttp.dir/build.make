# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Produce verbose output by default.
VERBOSE = 1

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

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ubuntu/HttpPractice

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ubuntu/HttpPractice

# Include any dependencies generated for this target.
include CMakeFiles/myhttp.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/myhttp.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/myhttp.dir/flags.make

CMakeFiles/myhttp.dir/myhttp/log.cpp.o: CMakeFiles/myhttp.dir/flags.make
CMakeFiles/myhttp.dir/myhttp/log.cpp.o: myhttp/log.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/HttpPractice/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/myhttp.dir/myhttp/log.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/myhttp.dir/myhttp/log.cpp.o -c /home/ubuntu/HttpPractice/myhttp/log.cpp

CMakeFiles/myhttp.dir/myhttp/log.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/myhttp.dir/myhttp/log.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/HttpPractice/myhttp/log.cpp > CMakeFiles/myhttp.dir/myhttp/log.cpp.i

CMakeFiles/myhttp.dir/myhttp/log.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/myhttp.dir/myhttp/log.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/HttpPractice/myhttp/log.cpp -o CMakeFiles/myhttp.dir/myhttp/log.cpp.s

# Object files for target myhttp
myhttp_OBJECTS = \
"CMakeFiles/myhttp.dir/myhttp/log.cpp.o"

# External object files for target myhttp
myhttp_EXTERNAL_OBJECTS =

lib/libmyhttp.so: CMakeFiles/myhttp.dir/myhttp/log.cpp.o
lib/libmyhttp.so: CMakeFiles/myhttp.dir/build.make
lib/libmyhttp.so: CMakeFiles/myhttp.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ubuntu/HttpPractice/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX shared library lib/libmyhttp.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/myhttp.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/myhttp.dir/build: lib/libmyhttp.so

.PHONY : CMakeFiles/myhttp.dir/build

CMakeFiles/myhttp.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/myhttp.dir/cmake_clean.cmake
.PHONY : CMakeFiles/myhttp.dir/clean

CMakeFiles/myhttp.dir/depend:
	cd /home/ubuntu/HttpPractice && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ubuntu/HttpPractice /home/ubuntu/HttpPractice /home/ubuntu/HttpPractice /home/ubuntu/HttpPractice /home/ubuntu/HttpPractice/CMakeFiles/myhttp.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/myhttp.dir/depend

