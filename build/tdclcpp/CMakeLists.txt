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


# ==========================================================
# check project name and options
# ==========================================================
get_filename_component(this_target "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
string(TOLOWER "${this_target}" this_target)
project(${this_target})
# m:multibyte u:unicode mr:multibyte-use_rtm
if("${this_target}" STREQUAL "tdclcppm")
  set(TDCL_CPP_CHARSET m)
  set(TDCL_CPP_USE_RUNTIME_LIBS OFF)
elseif("${this_target}" STREQUAL "tdclcppu")
  set(TDCL_CPP_CHARSET u)
  set(TDCL_CPP_USE_RUNTIME_LIBS OFF)
elseif("${this_target}" STREQUAL "tdclcppmr")
  set(TDCL_CPP_CHARSET m)
  set(TDCL_CPP_USE_RUNTIME_LIBS ON)
else()
  message(ERROR "Invalid project name ${this_target}")
endif()


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


# ==========================================================
# add resource file (for Windows Visual Studio)
# ==========================================================
if(WIN32)
  transactd_ver_info_lic()
  set(${this_target}_RC_FILE "${CMAKE_CURRENT_BINARY_DIR}/tdclcpp.rc")
  transactd_generate_rc_file(
    TRANSACTD_ROOT "${TRANSACTD_ROOT}"
    OUTFILE "${${this_target}_RC_FILE}"
    F_VER   "${TDVER_CPP_INTERFACE_VER_MAJOR}.${TDVER_CPP_INTERFACE_VER_MINOR}.${TDVER_CPP_INTERFACE_VER_RELEASE}.${TRANSACTD_BUILD_NUMBER}"
    P_VER   "${TDVER_CPP_INTERFACE_VER_MAJOR}.${TDVER_CPP_INTERFACE_VER_MINOR}.${TDVER_CPP_INTERFACE_VER_RELEASE}.${TRANSACTD_BUILD_NUMBER}"
    P_NAME  "Transactd Client ${TRANSACTD_VER_POSTFIX}"
    F_DESC  "Transactd C++ client"
  )
  transactd_add_rc_file()
endif()


# ==========================================================
# add_library
# ==========================================================
add_library(${this_target} SHARED ${${this_target}_SOURCE_FILES})


# ==========================================================
# Add compile options
# ==========================================================
if(MSVC)
  if(TDCL_CPP_USE_RUNTIME_LIBS)
    transactd_set_MTMD("MD")
  else()
    transactd_set_MTMD("MT")
  endif()
else()
  set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_FLAGS
    " -finput-charset=utf-8 ")
endif()


# ==========================================================
# Add compile definitions
# ==========================================================
if(MSVC)
  set_property(TARGET ${this_target} PROPERTY COMPILE_DEFINITIONS
    WIN32 _WIN32 _WINDOWS _USRDLL BOOST_ALL_NO_LIB 
    LIB_TDCLCPP TRDCL_AUTOLINK_OFF TRDCLENGN_EXPORTS _CRT_SECURE_NO_WARNINGS)
else()
  set_property(TARGET ${this_target} PROPERTY COMPILE_DEFINITIONS
    PIC BOOST_ALL_NO_LIB)
  if(NOT MINGW)
    set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
      LINUX)
  else()
    set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
      WIN32 _WIN32)
  endif()
endif()
if(TRANSACTD_RUBY_GEM OR TRANSACTD_PHP)
  set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
    SWIG_BUILDING)
endif()
if("${TDCL_CPP_CHARSET}" STREQUAL "u")
  set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
    UNICODE _UNICODE)
else()
  set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
    _MBCS)
endif()


# ==========================================================
# add link / include
# ==========================================================
transactd_link_boost_libraries("filesystem;thread;chrono;system")
if(CAN_LINK_ICONV)
  target_link_libraries(${this_target} "${ICONV_LIBRARY}")
endif()
if(NOT WIN32)
  target_link_libraries(${this_target} dl)
endif()


# ==========================================================
# set binary name
# ==========================================================
bz_get_compiler_bitness()
if(WIN32)
  ## Windows tdclcpp[_Compiler]_[32|64][m|u|mr]_[MAJOR]_[MINOR].dll
  bz_get_compiler()
  set(TDCL_CPP_NAME_TMP "tdclcpp${BZ_COMPILER}_${BZ_BITNESS}${TDCL_CPP_CHARSET}")
  if(TDCL_CPP_USE_RUNTIME_LIBS)
    set(TDCL_CPP_NAME_TMP "${TDCL_CPP_NAME_TMP}r")
  endif()
  set_target_properties(${this_target} PROPERTIES PREFIX "")
  set_target_properties(${this_target} PROPERTIES OUTPUT_NAME
    "${TDCL_CPP_NAME_TMP}_${TDVER_CPP_INTERFACE_VER_MAJOR}_${TDVER_CPP_INTERFACE_VER_MINOR}")
  set_target_properties(${this_target} PROPERTIES SUFFIX ".dll")
else()
  ## Linux   libtdclcpp_[32|64][m|u].so.[MAJOR].[MINOR].[RELEASE]
  set_target_properties(${this_target} PROPERTIES PREFIX "lib")
  set_target_properties(${this_target} PROPERTIES OUTPUT_NAME
    "tdclcpp_${BZ_BITNESS}${TDCL_CPP_CHARSET}")
  if(APPLE)
  ## Apple   libtdclcpp_[32|64][m|u].[MAJOR].[MINOR].[RELEASE].dylib
    set_target_properties(${this_target} PROPERTIES SUFFIX ".dylib")
  else()
    set_target_properties(${this_target} PROPERTIES SUFFIX ".so")
  endif()
  set_target_properties(${this_target} PROPERTIES
    SOVERSION ${TDVER_CPP_INTERFACE_VER_MAJOR}.${TDVER_CPP_INTERFACE_VER_MINOR}
    VERSION ${TDVER_CPP_INTERFACE_VER_MAJOR}.${TDVER_CPP_INTERFACE_VER_MINOR}.${TDVER_CPP_INTERFACE_VER_RELEASE})
endif()


# ==========================================================
# set output directory
# ==========================================================
if(NOT COMMAND tdcl_set_output)
  if(TRANSACTD_RUBY_GEM)
    include(../tdclrb/gem_output.cmake)
  else()
    include(../common/transactd_cl_output.cmake)
  endif()
endif()
tdcl_set_output("${TRANSACTD_BINARY_ROOT}" "${TRANSACTD_CLIENTS_PREFIX}")
