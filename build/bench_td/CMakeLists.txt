##=================================================================
#   Copyright (C) 2012 2013 BizStation Corp All rights reserved.
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
## Transactd benchmark program
##
## *** original file is ../bench_td/CMakeLists.txt ***
##
cmake_minimum_required(VERSION 2.6)
include(../common/transactd_required.cmake)
include(../common/transactd_common.cmake)
include(../common/options.cmake)
include(../common/system.cmake)
include(../common/get_boost_libs.cmake)


get_filename_component(this_target "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
string(REPLACE bench_td "" TDCL_CPP_CHARSET "${this_target}")
project(${this_target})


# ==========================================================
# options
# ==========================================================
if(NOT TRANSACTD_PREFIX)
  set(TRANSACTD_PREFIX "/usr/local/transactd" CACHE STRING "Prefix for transactd executables(test,benchmark)")
endif()


# ==========================================================
# set variables
# ==========================================================
if("${CMAKE_CURRENT_BINARY_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  get_filename_component(TRANSACTD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
  set(TRANSACTD_BINARY_ROOT "${CMAKE_BINARY_DIR}")
endif()


# ==========================================================
# version info
# ==========================================================
transactd_read_version("${TRANSACTD_ROOT}")
transactd_read_build_number("${TRANSACTD_ROOT}")


# ==========================================================
# source files
# ==========================================================
set(${this_target}_SOURCE_FILES
  ${TRANSACTD_ROOT}/source/bzs/test/transactdBench/transactdBench.cpp
  ${TRANSACTD_ROOT}/source/bzs/rtl/benchmark.cpp)


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
if(WIN32)
  set(${this_target}_RC_FILE "${CMAKE_CURRENT_BINARY_DIR}/${this_target}.rc")
  transactd_generate_rc_file(
    TRANSACTD_ROOT "${TRANSACTD_ROOT}"
    OUTFILE "${${this_target}_RC_FILE}"
    F_VER   "${TDVER_CPP_INTERFACE_VER_MAJOR}.${TDVER_CPP_INTERFACE_VER_MINOR}.${TDVER_CPP_INTERFACE_VER_RELEASE}.${TRANSACTD_BUILD_NUMBER}"
    P_VER   "${TDVER_CPP_INTERFACE_VER_MAJOR}.${TDVER_CPP_INTERFACE_VER_MINOR}.${TDVER_CPP_INTERFACE_VER_RELEASE}.${TRANSACTD_BUILD_NUMBER}"
    P_NAME  "Transactd CPP Client benchmark program"
    F_DESC  "Transactd CPP Client benchmark program"
  )
  transactd_add_rc_file()
endif()


# ==========================================================
# add_executable and set output name
# ==========================================================
add_executable(${this_target} ${${this_target}_SOURCE_FILES})
bz_get_compiler_bitness()
set_target_properties(${this_target} PROPERTIES OUTPUT_NAME "bench_td_${BZ_BITNESS}${TDCL_CPP_CHARSET}")


# ==========================================================
# Add compile options
# ==========================================================
if(NOT MSVC)
  set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_FLAGS
    " -finput-charset=utf-8 ")
endif()


# ==========================================================
# Add compile definitions
# ==========================================================
if(MSVC)
  set_property(TARGET ${this_target} PROPERTY COMPILE_DEFINITIONS
    WIN32 _WINDOWS TRDCL_AUTOLINK BOOST_ALL_NO_LIB)
else()
  set_property(TARGET ${this_target} PROPERTY COMPILE_DEFINITIONS
    PIC BOOST_ALL_NO_LIB)
  if(NOT MINGW)
    set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
      LINUX)
  endif()
endif()


# ==========================================================
# add link / include
# ==========================================================
if(TARGET tdclcpp${TDCL_CPP_CHARSET})
  transactd_link_boost_libraries("timer")
  target_link_libraries(${this_target} "tdclcpp${TDCL_CPP_CHARSET}")
else()
  transactd_link_boost_libraries("timer;chrono")
  bz_get_compiler_bitness()
  target_link_libraries(${this_target} "tdclcpp_${BZ_BITNESS}${TDCL_CPP_CHARSET}")
endif()
if(UNIX)
  target_link_libraries(${this_target} dl)
endif()


# ==========================================================
# set output directory
# ==========================================================
if(NOT COMMAND transactd_set_exe_output)
  include(../common/transactd_cl_output.cmake)
endif()
transactd_set_exe_output("${TRANSACTD_BINARY_ROOT}" "${TRANSACTD_PREFIX}")
