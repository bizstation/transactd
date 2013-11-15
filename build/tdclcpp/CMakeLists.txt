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
## Transactd C++ client
##
## *** original file is ../tdclcpp/CMakeLists.txt ***
##
cmake_minimum_required(VERSION 2.6)
include(../common/transactd_required.cmake)
include(../common/transactd_common.cmake)
include(../common/transactd_cl_common.cmake)
include(../common/options.cmake)
include(../common/system.cmake)
include(../common/get_boost_libs.cmake)
include(../common/check_for_link_iconv.cmake)
check_for_link_iconv()


get_filename_component(this_target "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
string(REPLACE tdclcpp "" TDCL_CPP_CHARSET "${this_target}")
project(${this_target})


# ==========================================================
# options
# ==========================================================
if(NOT TRANSACTD_CLIENTS_PREFIX)
  set(TRANSACTD_CLIENTS_PREFIX "" CACHE STRING "Prefix for transactd client libraries")
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
# add source / include dir , set compiler / linker options
# ==========================================================
tdcl_add_source_files("${TRANSACTD_ROOT}")
tdcl_add_include_directory("${TRANSACTD_ROOT}")
tdcl_add_compiler_options()
if(MSVC)
  foreach(build_type "" "_RELEASE" "_DEBUG" "_RELWITHDEBINFO" "_MINSIZEREL")
    if("${TDCL_CPP_CHARSET}" STREQUAL "u")
      bz_set_UNICODE_cxx_flag("${build_type}")
    elseif("${TDCL_CPP_CHARSET}" STREQUAL "m")
      bz_set_MBCS_cxx_flag("${build_type}")
    endif()
  endforeach()
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
    P_NAME  "Transactd CPP Client"
    F_DESC  "Transactd CPP Client module"
  )
  transactd_add_rc_file()
endif()


# ==========================================================
# add_library
# ==========================================================
add_library(${this_target} SHARED ${${this_target}_SOURCE_FILES})


# ==========================================================
# add link / include
# ==========================================================
transactd_link_boost_libraries("chrono;filesystem;system;thread")
if(CAN_LINK_ICONV)
  target_link_libraries(${this_target} iconv)
endif()


# ==========================================================
# set binary name
# ==========================================================
bz_get_compiler_bitness()
if(WIN32)
  ## Windows tdclcpp[_Compiler]_[32|64][m|u]_[MAJOR]_[MINOR].dll
  bz_get_compiler()
  set_target_properties(${this_target} PROPERTIES PREFIX "")
  set_target_properties(${this_target} PROPERTIES OUTPUT_NAME
    "tdclcpp${BZ_COMPILER}_${BZ_BITNESS}${TDCL_CPP_CHARSET}_${TDVER_CPP_INTERFACE_VER_MAJOR}_${TDVER_CPP_INTERFACE_VER_MINOR}")
  set_target_properties(${this_target} PROPERTIES SUFFIX ".dll")
else()
  ## Linux   libtdclcpp_[32|64][m|u].so.[MAJOR].[MINOR]
  set_target_properties(${this_target} PROPERTIES PREFIX "lib")
  set_target_properties(${this_target} PROPERTIES OUTPUT_NAME
    "tdclcpp_${BZ_BITNESS}${TDCL_CPP_CHARSET}")
  set_target_properties(${this_target} PROPERTIES SUFFIX ".so")
  set_target_properties(${this_target} PROPERTIES
    SOVERSION ${TDVER_CPP_INTERFACE_VER_MAJOR}.${TDVER_CPP_INTERFACE_VER_MINOR}
    VERSION ${TDVER_CPP_INTERFACE_VER_MAJOR}.${TDVER_CPP_INTERFACE_VER_MINOR}.${TDVER_CPP_INTERFACE_VER_RELEASE})
endif()


# ==========================================================
# set output directory
# ==========================================================
if(NOT COMMAND tdcl_set_output)
  include(../common/transactd_cl_output.cmake)
endif()
tdcl_set_output("${TRANSACTD_BINARY_ROOT}" "${TRANSACTD_CLIENTS_PREFIX}")
