# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


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

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "No interactive CMake dialog available..."
	/usr/bin/cmake -E echo No\ interactive\ CMake\ dialog\ available.
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/ubuntu/HttpPractice/CMakeFiles /home/ubuntu/HttpPractice/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/ubuntu/HttpPractice/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named test_hook

# Build rule for target.
test_hook: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 test_hook
.PHONY : test_hook

# fast build rule for target.
test_hook/fast:
	$(MAKE) -f CMakeFiles/test_hook.dir/build.make CMakeFiles/test_hook.dir/build
.PHONY : test_hook/fast

#=============================================================================
# Target rules for targets named test_iomanager

# Build rule for target.
test_iomanager: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 test_iomanager
.PHONY : test_iomanager

# fast build rule for target.
test_iomanager/fast:
	$(MAKE) -f CMakeFiles/test_iomanager.dir/build.make CMakeFiles/test_iomanager.dir/build
.PHONY : test_iomanager/fast

#=============================================================================
# Target rules for targets named test_scheduler

# Build rule for target.
test_scheduler: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 test_scheduler
.PHONY : test_scheduler

# fast build rule for target.
test_scheduler/fast:
	$(MAKE) -f CMakeFiles/test_scheduler.dir/build.make CMakeFiles/test_scheduler.dir/build
.PHONY : test_scheduler/fast

#=============================================================================
# Target rules for targets named test_fiber

# Build rule for target.
test_fiber: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 test_fiber
.PHONY : test_fiber

# fast build rule for target.
test_fiber/fast:
	$(MAKE) -f CMakeFiles/test_fiber.dir/build.make CMakeFiles/test_fiber.dir/build
.PHONY : test_fiber/fast

#=============================================================================
# Target rules for targets named test_config

# Build rule for target.
test_config: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 test_config
.PHONY : test_config

# fast build rule for target.
test_config/fast:
	$(MAKE) -f CMakeFiles/test_config.dir/build.make CMakeFiles/test_config.dir/build
.PHONY : test_config/fast

#=============================================================================
# Target rules for targets named test_thread

# Build rule for target.
test_thread: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 test_thread
.PHONY : test_thread

# fast build rule for target.
test_thread/fast:
	$(MAKE) -f CMakeFiles/test_thread.dir/build.make CMakeFiles/test_thread.dir/build
.PHONY : test_thread/fast

#=============================================================================
# Target rules for targets named test

# Build rule for target.
test: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 test
.PHONY : test

# fast build rule for target.
test/fast:
	$(MAKE) -f CMakeFiles/test.dir/build.make CMakeFiles/test.dir/build
.PHONY : test/fast

#=============================================================================
# Target rules for targets named test_utils

# Build rule for target.
test_utils: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 test_utils
.PHONY : test_utils

# fast build rule for target.
test_utils/fast:
	$(MAKE) -f CMakeFiles/test_utils.dir/build.make CMakeFiles/test_utils.dir/build
.PHONY : test_utils/fast

#=============================================================================
# Target rules for targets named myhttp

# Build rule for target.
myhttp: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 myhttp
.PHONY : myhttp

# fast build rule for target.
myhttp/fast:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/build
.PHONY : myhttp/fast

myhttp/config.o: myhttp/config.cpp.o

.PHONY : myhttp/config.o

# target to build an object file
myhttp/config.cpp.o:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/config.cpp.o
.PHONY : myhttp/config.cpp.o

myhttp/config.i: myhttp/config.cpp.i

.PHONY : myhttp/config.i

# target to preprocess a source file
myhttp/config.cpp.i:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/config.cpp.i
.PHONY : myhttp/config.cpp.i

myhttp/config.s: myhttp/config.cpp.s

.PHONY : myhttp/config.s

# target to generate assembly for a file
myhttp/config.cpp.s:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/config.cpp.s
.PHONY : myhttp/config.cpp.s

myhttp/fiber.o: myhttp/fiber.cpp.o

.PHONY : myhttp/fiber.o

# target to build an object file
myhttp/fiber.cpp.o:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/fiber.cpp.o
.PHONY : myhttp/fiber.cpp.o

myhttp/fiber.i: myhttp/fiber.cpp.i

.PHONY : myhttp/fiber.i

# target to preprocess a source file
myhttp/fiber.cpp.i:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/fiber.cpp.i
.PHONY : myhttp/fiber.cpp.i

myhttp/fiber.s: myhttp/fiber.cpp.s

.PHONY : myhttp/fiber.s

# target to generate assembly for a file
myhttp/fiber.cpp.s:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/fiber.cpp.s
.PHONY : myhttp/fiber.cpp.s

myhttp/hook.o: myhttp/hook.cpp.o

.PHONY : myhttp/hook.o

# target to build an object file
myhttp/hook.cpp.o:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/hook.cpp.o
.PHONY : myhttp/hook.cpp.o

myhttp/hook.i: myhttp/hook.cpp.i

.PHONY : myhttp/hook.i

# target to preprocess a source file
myhttp/hook.cpp.i:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/hook.cpp.i
.PHONY : myhttp/hook.cpp.i

myhttp/hook.s: myhttp/hook.cpp.s

.PHONY : myhttp/hook.s

# target to generate assembly for a file
myhttp/hook.cpp.s:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/hook.cpp.s
.PHONY : myhttp/hook.cpp.s

myhttp/iomanager.o: myhttp/iomanager.cpp.o

.PHONY : myhttp/iomanager.o

# target to build an object file
myhttp/iomanager.cpp.o:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/iomanager.cpp.o
.PHONY : myhttp/iomanager.cpp.o

myhttp/iomanager.i: myhttp/iomanager.cpp.i

.PHONY : myhttp/iomanager.i

# target to preprocess a source file
myhttp/iomanager.cpp.i:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/iomanager.cpp.i
.PHONY : myhttp/iomanager.cpp.i

myhttp/iomanager.s: myhttp/iomanager.cpp.s

.PHONY : myhttp/iomanager.s

# target to generate assembly for a file
myhttp/iomanager.cpp.s:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/iomanager.cpp.s
.PHONY : myhttp/iomanager.cpp.s

myhttp/log.o: myhttp/log.cpp.o

.PHONY : myhttp/log.o

# target to build an object file
myhttp/log.cpp.o:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/log.cpp.o
.PHONY : myhttp/log.cpp.o

myhttp/log.i: myhttp/log.cpp.i

.PHONY : myhttp/log.i

# target to preprocess a source file
myhttp/log.cpp.i:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/log.cpp.i
.PHONY : myhttp/log.cpp.i

myhttp/log.s: myhttp/log.cpp.s

.PHONY : myhttp/log.s

# target to generate assembly for a file
myhttp/log.cpp.s:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/log.cpp.s
.PHONY : myhttp/log.cpp.s

myhttp/scheduler.o: myhttp/scheduler.cpp.o

.PHONY : myhttp/scheduler.o

# target to build an object file
myhttp/scheduler.cpp.o:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/scheduler.cpp.o
.PHONY : myhttp/scheduler.cpp.o

myhttp/scheduler.i: myhttp/scheduler.cpp.i

.PHONY : myhttp/scheduler.i

# target to preprocess a source file
myhttp/scheduler.cpp.i:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/scheduler.cpp.i
.PHONY : myhttp/scheduler.cpp.i

myhttp/scheduler.s: myhttp/scheduler.cpp.s

.PHONY : myhttp/scheduler.s

# target to generate assembly for a file
myhttp/scheduler.cpp.s:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/scheduler.cpp.s
.PHONY : myhttp/scheduler.cpp.s

myhttp/thread.o: myhttp/thread.cpp.o

.PHONY : myhttp/thread.o

# target to build an object file
myhttp/thread.cpp.o:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/thread.cpp.o
.PHONY : myhttp/thread.cpp.o

myhttp/thread.i: myhttp/thread.cpp.i

.PHONY : myhttp/thread.i

# target to preprocess a source file
myhttp/thread.cpp.i:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/thread.cpp.i
.PHONY : myhttp/thread.cpp.i

myhttp/thread.s: myhttp/thread.cpp.s

.PHONY : myhttp/thread.s

# target to generate assembly for a file
myhttp/thread.cpp.s:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/thread.cpp.s
.PHONY : myhttp/thread.cpp.s

myhttp/timer.o: myhttp/timer.cpp.o

.PHONY : myhttp/timer.o

# target to build an object file
myhttp/timer.cpp.o:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/timer.cpp.o
.PHONY : myhttp/timer.cpp.o

myhttp/timer.i: myhttp/timer.cpp.i

.PHONY : myhttp/timer.i

# target to preprocess a source file
myhttp/timer.cpp.i:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/timer.cpp.i
.PHONY : myhttp/timer.cpp.i

myhttp/timer.s: myhttp/timer.cpp.s

.PHONY : myhttp/timer.s

# target to generate assembly for a file
myhttp/timer.cpp.s:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/timer.cpp.s
.PHONY : myhttp/timer.cpp.s

myhttp/util.o: myhttp/util.cpp.o

.PHONY : myhttp/util.o

# target to build an object file
myhttp/util.cpp.o:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/util.cpp.o
.PHONY : myhttp/util.cpp.o

myhttp/util.i: myhttp/util.cpp.i

.PHONY : myhttp/util.i

# target to preprocess a source file
myhttp/util.cpp.i:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/util.cpp.i
.PHONY : myhttp/util.cpp.i

myhttp/util.s: myhttp/util.cpp.s

.PHONY : myhttp/util.s

# target to generate assembly for a file
myhttp/util.cpp.s:
	$(MAKE) -f CMakeFiles/myhttp.dir/build.make CMakeFiles/myhttp.dir/myhttp/util.cpp.s
.PHONY : myhttp/util.cpp.s

tests/test.o: tests/test.cpp.o

.PHONY : tests/test.o

# target to build an object file
tests/test.cpp.o:
	$(MAKE) -f CMakeFiles/test.dir/build.make CMakeFiles/test.dir/tests/test.cpp.o
.PHONY : tests/test.cpp.o

tests/test.i: tests/test.cpp.i

.PHONY : tests/test.i

# target to preprocess a source file
tests/test.cpp.i:
	$(MAKE) -f CMakeFiles/test.dir/build.make CMakeFiles/test.dir/tests/test.cpp.i
.PHONY : tests/test.cpp.i

tests/test.s: tests/test.cpp.s

.PHONY : tests/test.s

# target to generate assembly for a file
tests/test.cpp.s:
	$(MAKE) -f CMakeFiles/test.dir/build.make CMakeFiles/test.dir/tests/test.cpp.s
.PHONY : tests/test.cpp.s

tests/test_config.o: tests/test_config.cpp.o

.PHONY : tests/test_config.o

# target to build an object file
tests/test_config.cpp.o:
	$(MAKE) -f CMakeFiles/test_config.dir/build.make CMakeFiles/test_config.dir/tests/test_config.cpp.o
.PHONY : tests/test_config.cpp.o

tests/test_config.i: tests/test_config.cpp.i

.PHONY : tests/test_config.i

# target to preprocess a source file
tests/test_config.cpp.i:
	$(MAKE) -f CMakeFiles/test_config.dir/build.make CMakeFiles/test_config.dir/tests/test_config.cpp.i
.PHONY : tests/test_config.cpp.i

tests/test_config.s: tests/test_config.cpp.s

.PHONY : tests/test_config.s

# target to generate assembly for a file
tests/test_config.cpp.s:
	$(MAKE) -f CMakeFiles/test_config.dir/build.make CMakeFiles/test_config.dir/tests/test_config.cpp.s
.PHONY : tests/test_config.cpp.s

tests/test_fiber.o: tests/test_fiber.cpp.o

.PHONY : tests/test_fiber.o

# target to build an object file
tests/test_fiber.cpp.o:
	$(MAKE) -f CMakeFiles/test_fiber.dir/build.make CMakeFiles/test_fiber.dir/tests/test_fiber.cpp.o
.PHONY : tests/test_fiber.cpp.o

tests/test_fiber.i: tests/test_fiber.cpp.i

.PHONY : tests/test_fiber.i

# target to preprocess a source file
tests/test_fiber.cpp.i:
	$(MAKE) -f CMakeFiles/test_fiber.dir/build.make CMakeFiles/test_fiber.dir/tests/test_fiber.cpp.i
.PHONY : tests/test_fiber.cpp.i

tests/test_fiber.s: tests/test_fiber.cpp.s

.PHONY : tests/test_fiber.s

# target to generate assembly for a file
tests/test_fiber.cpp.s:
	$(MAKE) -f CMakeFiles/test_fiber.dir/build.make CMakeFiles/test_fiber.dir/tests/test_fiber.cpp.s
.PHONY : tests/test_fiber.cpp.s

tests/test_hook.o: tests/test_hook.cpp.o

.PHONY : tests/test_hook.o

# target to build an object file
tests/test_hook.cpp.o:
	$(MAKE) -f CMakeFiles/test_hook.dir/build.make CMakeFiles/test_hook.dir/tests/test_hook.cpp.o
.PHONY : tests/test_hook.cpp.o

tests/test_hook.i: tests/test_hook.cpp.i

.PHONY : tests/test_hook.i

# target to preprocess a source file
tests/test_hook.cpp.i:
	$(MAKE) -f CMakeFiles/test_hook.dir/build.make CMakeFiles/test_hook.dir/tests/test_hook.cpp.i
.PHONY : tests/test_hook.cpp.i

tests/test_hook.s: tests/test_hook.cpp.s

.PHONY : tests/test_hook.s

# target to generate assembly for a file
tests/test_hook.cpp.s:
	$(MAKE) -f CMakeFiles/test_hook.dir/build.make CMakeFiles/test_hook.dir/tests/test_hook.cpp.s
.PHONY : tests/test_hook.cpp.s

tests/test_iomanager.o: tests/test_iomanager.cpp.o

.PHONY : tests/test_iomanager.o

# target to build an object file
tests/test_iomanager.cpp.o:
	$(MAKE) -f CMakeFiles/test_iomanager.dir/build.make CMakeFiles/test_iomanager.dir/tests/test_iomanager.cpp.o
.PHONY : tests/test_iomanager.cpp.o

tests/test_iomanager.i: tests/test_iomanager.cpp.i

.PHONY : tests/test_iomanager.i

# target to preprocess a source file
tests/test_iomanager.cpp.i:
	$(MAKE) -f CMakeFiles/test_iomanager.dir/build.make CMakeFiles/test_iomanager.dir/tests/test_iomanager.cpp.i
.PHONY : tests/test_iomanager.cpp.i

tests/test_iomanager.s: tests/test_iomanager.cpp.s

.PHONY : tests/test_iomanager.s

# target to generate assembly for a file
tests/test_iomanager.cpp.s:
	$(MAKE) -f CMakeFiles/test_iomanager.dir/build.make CMakeFiles/test_iomanager.dir/tests/test_iomanager.cpp.s
.PHONY : tests/test_iomanager.cpp.s

tests/test_scheduler.o: tests/test_scheduler.cpp.o

.PHONY : tests/test_scheduler.o

# target to build an object file
tests/test_scheduler.cpp.o:
	$(MAKE) -f CMakeFiles/test_scheduler.dir/build.make CMakeFiles/test_scheduler.dir/tests/test_scheduler.cpp.o
.PHONY : tests/test_scheduler.cpp.o

tests/test_scheduler.i: tests/test_scheduler.cpp.i

.PHONY : tests/test_scheduler.i

# target to preprocess a source file
tests/test_scheduler.cpp.i:
	$(MAKE) -f CMakeFiles/test_scheduler.dir/build.make CMakeFiles/test_scheduler.dir/tests/test_scheduler.cpp.i
.PHONY : tests/test_scheduler.cpp.i

tests/test_scheduler.s: tests/test_scheduler.cpp.s

.PHONY : tests/test_scheduler.s

# target to generate assembly for a file
tests/test_scheduler.cpp.s:
	$(MAKE) -f CMakeFiles/test_scheduler.dir/build.make CMakeFiles/test_scheduler.dir/tests/test_scheduler.cpp.s
.PHONY : tests/test_scheduler.cpp.s

tests/test_thread.o: tests/test_thread.cpp.o

.PHONY : tests/test_thread.o

# target to build an object file
tests/test_thread.cpp.o:
	$(MAKE) -f CMakeFiles/test_thread.dir/build.make CMakeFiles/test_thread.dir/tests/test_thread.cpp.o
.PHONY : tests/test_thread.cpp.o

tests/test_thread.i: tests/test_thread.cpp.i

.PHONY : tests/test_thread.i

# target to preprocess a source file
tests/test_thread.cpp.i:
	$(MAKE) -f CMakeFiles/test_thread.dir/build.make CMakeFiles/test_thread.dir/tests/test_thread.cpp.i
.PHONY : tests/test_thread.cpp.i

tests/test_thread.s: tests/test_thread.cpp.s

.PHONY : tests/test_thread.s

# target to generate assembly for a file
tests/test_thread.cpp.s:
	$(MAKE) -f CMakeFiles/test_thread.dir/build.make CMakeFiles/test_thread.dir/tests/test_thread.cpp.s
.PHONY : tests/test_thread.cpp.s

tests/test_utils.o: tests/test_utils.cpp.o

.PHONY : tests/test_utils.o

# target to build an object file
tests/test_utils.cpp.o:
	$(MAKE) -f CMakeFiles/test_utils.dir/build.make CMakeFiles/test_utils.dir/tests/test_utils.cpp.o
.PHONY : tests/test_utils.cpp.o

tests/test_utils.i: tests/test_utils.cpp.i

.PHONY : tests/test_utils.i

# target to preprocess a source file
tests/test_utils.cpp.i:
	$(MAKE) -f CMakeFiles/test_utils.dir/build.make CMakeFiles/test_utils.dir/tests/test_utils.cpp.i
.PHONY : tests/test_utils.cpp.i

tests/test_utils.s: tests/test_utils.cpp.s

.PHONY : tests/test_utils.s

# target to generate assembly for a file
tests/test_utils.cpp.s:
	$(MAKE) -f CMakeFiles/test_utils.dir/build.make CMakeFiles/test_utils.dir/tests/test_utils.cpp.s
.PHONY : tests/test_utils.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... rebuild_cache"
	@echo "... test_hook"
	@echo "... test_iomanager"
	@echo "... test_scheduler"
	@echo "... test_fiber"
	@echo "... test_config"
	@echo "... edit_cache"
	@echo "... test_thread"
	@echo "... test"
	@echo "... test_utils"
	@echo "... myhttp"
	@echo "... myhttp/config.o"
	@echo "... myhttp/config.i"
	@echo "... myhttp/config.s"
	@echo "... myhttp/fiber.o"
	@echo "... myhttp/fiber.i"
	@echo "... myhttp/fiber.s"
	@echo "... myhttp/hook.o"
	@echo "... myhttp/hook.i"
	@echo "... myhttp/hook.s"
	@echo "... myhttp/iomanager.o"
	@echo "... myhttp/iomanager.i"
	@echo "... myhttp/iomanager.s"
	@echo "... myhttp/log.o"
	@echo "... myhttp/log.i"
	@echo "... myhttp/log.s"
	@echo "... myhttp/scheduler.o"
	@echo "... myhttp/scheduler.i"
	@echo "... myhttp/scheduler.s"
	@echo "... myhttp/thread.o"
	@echo "... myhttp/thread.i"
	@echo "... myhttp/thread.s"
	@echo "... myhttp/timer.o"
	@echo "... myhttp/timer.i"
	@echo "... myhttp/timer.s"
	@echo "... myhttp/util.o"
	@echo "... myhttp/util.i"
	@echo "... myhttp/util.s"
	@echo "... tests/test.o"
	@echo "... tests/test.i"
	@echo "... tests/test.s"
	@echo "... tests/test_config.o"
	@echo "... tests/test_config.i"
	@echo "... tests/test_config.s"
	@echo "... tests/test_fiber.o"
	@echo "... tests/test_fiber.i"
	@echo "... tests/test_fiber.s"
	@echo "... tests/test_hook.o"
	@echo "... tests/test_hook.i"
	@echo "... tests/test_hook.s"
	@echo "... tests/test_iomanager.o"
	@echo "... tests/test_iomanager.i"
	@echo "... tests/test_iomanager.s"
	@echo "... tests/test_scheduler.o"
	@echo "... tests/test_scheduler.i"
	@echo "... tests/test_scheduler.s"
	@echo "... tests/test_thread.o"
	@echo "... tests/test_thread.i"
	@echo "... tests/test_thread.s"
	@echo "... tests/test_utils.o"
	@echo "... tests/test_utils.i"
	@echo "... tests/test_utils.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

