##=================================================================
#   Copyright (C) 2012-2014 BizStation Corp All rights reserved.
#
#   This program is free software; you can redistribute it and/or
#   modify it under the terms of the GNU General Public License
#   as published by the Free Software Foundation; either version 2
#   of the License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software 
#   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
#   02111-1307, USA.
##=================================================================
##
## Transactd query benchmark program
##
## *** original file is ../bench_query/CMakeLists.txt ***
##
cmake_minimum_required(VERSION 2.6)

include(../common/transactd_required.cmake)
include(../common/transactd_common.cmake)
include(../common/options.cmake)
include(../common/system.cmake)
include(../common/get_boost_libs.cmake)


get_filename_component(this_target "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
project(${this_target})
bz_force_gcc() # set compiler to gcc instead of g++

if(APPLE)
  transactd_set_rpath("${TRANSACTD_CLIENTS_PREFIX}")
endif()


# ==========================================================
# options
# ==========================================================
if(NOT TRANSACTD_PREFIX)
  set(TRANSACTD_PREFIX "/usr/local/transactd" CACHE STRING "Prefix for transactd executables")
endif()


# ==========================================================
# set variables
# ==========================================================
if("${CMAKE_CURRENT_BINARY_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  get_filename_component(TRANSACTD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
  set(TRANSACTD_BINARY_ROOT "${CMAKE_BINARY_DIR}")
endif()


# ==========================================================
# add library path
# ==========================================================
if (MSVC)
  link_directories("${TRANSACTD_BINARY_ROOT}/lib")
else()
  cmake_policy(SET CMP0015 OLD)
  link_directories(../tdclcpp)
endif()


# ==========================================================
# source files
# ==========================================================
set(${this_target}_SOURCE_FILES
  ${TRANSACTD_ROOT}/source/bzs/example/queryData.cpp
  ${TRANSACTD_ROOT}/source/bzs/rtl/benchmark.cpp
  ${TRANSACTD_ROOT}/source/bzs/test/transactdBench/query_bench.cpp)


# ==========================================================
# include directories
# ==========================================================
include_directories(${TRANSACTD_ROOT}/source;)
if(UNIX)
  include_directories(${TRANSACTD_ROOT}/source/linux;)
endif()


# ==========================================================
# add resource file (for Windows Visual Studio)
# ==========================================================
set(${this_target}_RC_FILE "${TRANSACTD_ROOT}/build/bench_td/bench.rc")
transactd_read_rc("${${this_target}_RC_FILE}")
if(WIN32)
  transactd_add_rc_file()
endif()


# ==========================================================
# add_executable and set output name
# ==========================================================
add_executable(${this_target} ${${this_target}_SOURCE_FILES})
bz_get_compiler()
bz_get_compiler_bitness()
if(WIN32)
  # bench_query_[compiler]_[32|64][m|u].exe
  set_target_properties(${this_target} PROPERTIES
    OUTPUT_NAME "bench_query${BZ_COMPILER}_${BZ_BITNESS}${TDCL_CPP_CHARSET}")
else()
  # bench_query_[32|64]m
  set_target_properties(${this_target} PROPERTIES
    OUTPUT_NAME "bench_query_${BZ_BITNESS}${TDCL_CPP_CHARSET}")
endif()


# ==========================================================
# add dependencie projects
# ==========================================================
add_dependencies(${this_target} tdclc tdclcpp)


# ==========================================================
# Add compile definitions
# ==========================================================
if(MSVC)
  set_property(TARGET ${this_target} PROPERTY COMPILE_DEFINITIONS
    TRDCL_AUTOLINK)
else()
  set_property(TARGET ${this_target} PROPERTY COMPILE_DEFINITIONS
    PIC BOOST_ALL_NO_LIB)
  if(NOT MINGW)
    set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
      LINUX)
  endif()
endif()
# Unicode or MultiBytes
if("${TDCL_CPP_CHARSET}" STREQUAL "u")
  set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
    UNICODE _UNICODE)
else()
  set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
    _MBCS)
endif()


# ==========================================================
# Add compile options and linker options
# ==========================================================
transactd_reset_MTMD()
if (MSVC)
  set(CMAKE_CXX_FLAGS "/W3 /wd4068 /wd4996 /wd4819 /Gy /GL /Zi /Oi /EHsc")
  bz_add_cxx_flag("/MT  /Ox" "_RELEASE")
  bz_add_cxx_flag("/MTd    " "_DEBUG")
  bz_add_cxx_flag("/MT  /Ox" "_RELWITHDEBINFO")
  bz_add_cxx_flag("/MD  /Ox" "_MINSIZEREL")
  set(CMAKE_LINKER_FLAGS "")
  set(CMAKE_EXE_LINKER_FLAGS "")
  set_target_properties(${this_target} PROPERTIES LINK_FLAGS
    "/MANIFEST:NO /OPT:REF /OPT:ICF")
else()
  set(CMAKE_CXX_FLAGS "-fexceptions -frtti -fpermissive -fno-strict-aliasing -fno-omit-frame-pointer")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_EXE_LINKER_FLAGS "-lstdc++")
  endif()
endif()


# ==========================================================
# add link / include
# ==========================================================
transactd_link_boost_libraries("timer;system;chrono;thread")

#tdclcpp_x on windows is auto link
if(NOT MSVC)
  bz_get_compiler_bitness()
  target_link_libraries(${this_target} "tdclcpp_${BZ_BITNESS}${TDCL_CPP_CHARSET}")
  target_link_libraries(${this_target} dl)
endif()


# ==========================================================
# set output directory
# ==========================================================
if(NOT COMMAND transactd_set_exe_output)
  include(../common/transactd_cl_output.cmake)
endif()
transactd_set_exe_output("${TRANSACTD_BINARY_ROOT}" "${TRANSACTD_PREFIX}")
