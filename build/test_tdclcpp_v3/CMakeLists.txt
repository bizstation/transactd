##=================================================================
#   Copyright (C) 2012-2015 BizStation Corp All rights reserved.
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
## Build Transactd client library test program (version 3.0)
##
## *** original file is ../test_tdclcpp_v3/CMakeLists.txt ***
##
cmake_minimum_required(VERSION 2.6)
include(../common/transactd_required.cmake)
include(../common/transactd_common.cmake)
include(../common/options.cmake)
include(../common/system.cmake)
include(../common/get_boost_libs.cmake)
check_for_link_iconv()


get_filename_component(this_target "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
project(${this_target})
bz_force_gcc() # set compiler to gcc instead of g++


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
  ${TRANSACTD_ROOT}/source/bzs/test/trdclengn/test_tdclcpp_v3.cpp
  ${TRANSACTD_ROOT}/source/bzs/test/trdclengn/testField.h)


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
set(${this_target}_RC_FILE "${TRANSACTD_ROOT}/build/test_tdclcpp_v3/test_tdclcpp_v3.rc")
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
  # test_tdclcpp_v3_[compiler]_[32|64][m|u].exe
  set_target_properties(${this_target} PROPERTIES
    OUTPUT_NAME "test_tdclcpp_v3${BZ_COMPILER}_${BZ_BITNESS}${TDCL_CPP_CHARSET}")
else()
  # test_tdclcpp_v3_[32|64]m
  set_target_properties(${this_target} PROPERTIES
    OUTPUT_NAME "test_tdclcpp_v3_${BZ_BITNESS}${TDCL_CPP_CHARSET}")
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
    TRDCL_AUTOLINK USE_DBM_CONNECTION_POOL TEST_FIELDDEFS)
else()
  set_property(TARGET ${this_target} PROPERTY COMPILE_DEFINITIONS
    PIC BOOST_ALL_NO_LIB USE_DBM_CONNECTION_POOL)
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
  set(CMAKE_CXX_FLAGS "/W3 /wd4068 /wd4819 /wd4231 /wd4251 /Gy /GL /Zi /Oi /EHa")
  bz_add_cxx_flag("/MT"   "_RELEASE")
  bz_add_cxx_flag("/MTd"  "_DEBUG")
  bz_add_cxx_flag("/MT"   "_RELWITHDEBINFO")
  bz_add_cxx_flag("/MD"   "_MINSIZEREL")
  set(CMAKE_LINKER_FLAGS "")
  set(CMAKE_EXE_LINKER_FLAGS "")
  set_target_properties(${this_target} PROPERTIES LINK_FLAGS
    "/MANIFEST:NO /LTCG:incremental /OPT:REF /OPT:ICF")
else()
  set(CMAKE_CXX_FLAGS "-fexceptions -frtti -fpermissive -fno-strict-aliasing -fno-omit-frame-pointer")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -lstdc++")
  endif()
endif()


# ==========================================================
# add link / include
# ==========================================================
transactd_link_boost_libraries("")
if(CAN_LINK_ICONV)
  target_link_libraries(${this_target} "${ICONV_LIBRARY}")
endif()

#tdclcpp_x on windows is auto link
if(NOT MSVC)
  bz_get_compiler_bitness()
  target_link_libraries(${this_target} "tdclcpp_${BZ_BITNESS}${TDCL_CPP_CHARSET}")
  target_link_libraries(${this_target} dl pthread)
endif()


# ==========================================================
# set output directory
# ==========================================================
if(NOT COMMAND transactd_set_exe_output)
  include(../common/transactd_cl_output.cmake)
endif()
transactd_set_exe_output("${TRANSACTD_BINARY_ROOT}" "${TRANSACTD_PREFIX}")
