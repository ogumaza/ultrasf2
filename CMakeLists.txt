# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.20)
# ultrasf2 --version reports this version. The file properties of the Windows program and the Info.plist of the macOS
# app show it too.
project(ultrasf2 VERSION 1.3.0 LANGUAGES CXX)

# The submodules: seqlib writes the MIDI, SoundFont and SFZ files. ultralib contains the conversion of the Nintendo
# 64's sound data (ultralib::sequence_export_n64). ntdlib contains the converters' command-line frame
# (ntdlib::sequence_export). xsflib contains the common code that ntdlib and ultralib build on, and the CMake helpers.
# xsflib's XsflibProject.cmake sets the C++ standard, a release build by default, the static C runtime and the compiler
# options that keep the output the same on every platform. XSFLIB_ZLIB builds xsflib's reader of ROM sets with zlib.
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/externals/xsflib/CMakeLists.txt")
  message(FATAL_ERROR "externals/xsflib is missing. Run: git submodule update --init")
endif()
include(externals/xsflib/cmake/XsflibProject.cmake)
xsflib_project(WARNINGS_ON_EVERY_TARGET)
set(NTDLIB_SEQUENCE_EXPORT ON)
set(ULTRALIB_SOUND ON)
set(ULTRALIB_SEQUENCE_EXPORT ON)
set(XSFLIB_ZLIB ON)
xsflib_add_libraries(seqlib xsflib ntdlib ultralib)

add_executable(ultrasf2 src/main.cpp)
target_link_libraries(ultrasf2 PRIVATE ultralib::sequence_export_n64)
xsflib_add_version(ultrasf2 VERSION ${PROJECT_VERSION} DEFINE ULTRASF2_VERSION
                   DESCRIPTION "Nintendo 64 music converter" PRODUCT ultrasf2
                   COPYRIGHT "Copyright (c) 2026 the ultrasf2 authors. MIT licence.")

# Windows: UTF-8 as the code page. An image, a ROM set or a folder whose name the system's code page can't represent
# then opens.
xsflib_add_utf8_manifest(ultrasf2)

# Finder cannot drop files onto a command-line executable. Package it in a macOS droplet app, with the version in its
# Info.plist.
xsflib_add_droplet_tool(ultrasf2 APP_NAME Ultrasf2 VERSION ${PROJECT_VERSION}
                        DESCRIPTION "Drop Nintendo 64 images (.z64, .n64, .v64) or Aleck64 ROM sets (.zip) here to convert their music to MIDI, SoundFont and SFZ. Output goes in a folder beside each one."
                        PROMPT "Choose Nintendo 64 images (.z64, .n64, .v64) or Aleck64 ROM sets (.zip) to convert:")

include(CTest)
if(BUILD_TESTING)
  # The program reports the version it was built as.
  xsflib_add_version_test(ultrasf2 VERSION ${PROJECT_VERSION})

  # Load local_tests.cmake from each immediate subdirectory to register tests that use locally supplied files.
  xsflib_include_local_tests()
endif()
