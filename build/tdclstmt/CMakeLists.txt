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
## Transactd Statement client
##
## *** original file is ../tdclstmt/CMakeLists.txt ***
##
cmake_minimum_required(VERSION 2.6)
include(../common/transactd_required.cmake)
include(../common/transactd_common.cmake)
include(../common/transactd_cl_common.cmake)
include(../common/options.cmake)
include(../common/system.cmake)
include(../common/get_boost_libs.cmake)


get_filename_component(this_target "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
string(REPLACE tdclstmt "" TDCL_STMT_CHARSET "${this_target}")
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
# add source / include dir , set compiler / linker options
# ==========================================================
set(${this_target}_SOURCE_FILES
  ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/serializer.cpp
)
tdcl_add_include_directory("${TRANSACTD_ROOT}")


# ==========================================================
# add resource file (for Windows Visual Studio)
# ==========================================================
set(${this_target}_RC_FILE "${TRANSACTD_ROOT}/build/tdclstmt/tdclstmt.rc")
transactd_read_rc("${${this_target}_RC_FILE}")
if(WIN32)
  transactd_add_rc_file()
endif()


# ==========================================================
# add_library
# ==========================================================
add_library(${this_target} SHARED ${${this_target}_SOURCE_FILES})


# ==========================================================
# Add compile options
# ==========================================================
transactd_set_MTMD("MT")


# ==========================================================
# Add compile definitions
# ==========================================================
if(MSVC)
  set_property(TARGET ${this_target} PROPERTY COMPILE_DEFINITIONS
    WIN32 _WIN32 _WINDOWS _USRDLL LIB_TDCLSTMT TRDCL_AUTOLINK)
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
if(TARGET tdclcpp${TDCL_CPP_CHARSET})
  target_link_libraries(${this_target} "tdclcpp${TDCL_CPP_CHARSET}")
else()
  bz_get_compiler_bitness()
  target_link_libraries(${this_target} "tdclcpp_${BZ_BITNESS}${TDCL_CPP_CHARSET}")
endif()
transactd_link_boost_libraries("serialization")
if(NOT WIN32)
  target_link_libraries(${this_target} dl)
endif()


# ==========================================================
# set binary name
# ==========================================================
bz_get_compiler_bitness()
if(WIN32)
  ## Windows tdclstmt[_Compiler]_[32|64][m|u]_[MAJOR]_[MINOR].dll
  bz_get_compiler()
  set_target_properties(${this_target} PROPERTIES PREFIX "")
  set_target_properties(${this_target} PROPERTIES OUTPUT_NAME
    "tdclstmt${BZ_COMPILER}_${BZ_BITNESS}${TDCL_CPP_CHARSET}_${TD_RC_VER_MAJOR}_${TD_RC_VER_MINOR}")
  set_target_properties(${this_target} PROPERTIES SUFFIX ".dll")
else()
  ## Linux   libtdclstmt_[32|64][m|u].so.[MAJOR].[MINOR].[RELEASE]
  set_target_properties(${this_target} PROPERTIES PREFIX "lib")
  set_target_properties(${this_target} PROPERTIES OUTPUT_NAME
    "tdclstmt_${BZ_BITNESS}${TDCL_CPP_CHARSET}")
  if(APPLE)
  ## Apple   libtdclstmt_[32|64][m|u].[MAJOR].[MINOR].[RELEASE].dylib
    set_target_properties(${this_target} PROPERTIES SUFFIX ".dylib")
  else()
    set_target_properties(${this_target} PROPERTIES SUFFIX ".so")
  endif()
  set_target_properties(${this_target} PROPERTIES
    SOVERSION ${TD_RC_VER_MAJOR}.${TD_RC_VER_MINOR}
    VERSION ${TD_RC_VER_MAJOR}.${TD_RC_VER_MINOR}.${TD_RC_VER_RELEASE})
endif()


# ==========================================================
# set output directory
# ==========================================================
if(NOT COMMAND tdcl_set_output)
  include(../common/transactd_cl_output.cmake)
endif()
tdcl_set_output("${TRANSACTD_BINARY_ROOT}" "${TRANSACTD_CLIENTS_PREFIX}")
