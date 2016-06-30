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
# add library path
# ==========================================================
if (MSVC)
   link_directories("${TRANSACTD_BINARY_ROOT}/lib")
else()
   link_directories("../tdclcpp")
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
# add dependencie projects
# ==========================================================
add_dependencies(${this_target} tdclc tdclcpp)


# ==========================================================
# Add compile definitions
# ==========================================================
if(MSVC)
  set_property(TARGET ${this_target} PROPERTY COMPILE_DEFINITIONS
    TRDCL_AUTOLINK LIB_TDCLSTMT BOOST_DATE_TIME_NO_LIB _USRDLL TEDCLSTMT1_EXPORTS _WINDLL)
else()
  set_property(TARGET ${this_target} PROPERTY COMPILE_DEFINITIONS
    PIC _REENTRANT LIB_TDCLSTMT BOOST_DATE_TIME_NO_LIB TEDCLSTMT1_EXPORTS)
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
# Add compile options and linker options
# ==========================================================
transactd_reset_MTMD()
if (MSVC)
  set(CMAKE_CXX_FLAGS "/W3 /wd4068 /wd4819 /Gy /GL /Zi /Oi /EHsc")
  bz_add_cxx_flag("/MT  /GR" "_RELEASE")
  bz_add_cxx_flag("/MTd    " "_DEBUG")
  bz_add_cxx_flag("/MT  /GR" "_RELWITHDEBINFO")
  bz_add_cxx_flag("/MD  /GR" "_MINSIZEREL")
  set(CMAKE_LINKER_FLAGS "")
  set(CMAKE_SHARED_LINKER_FLAGS "")
  set_target_properties(${this_target} PROPERTIES LINK_FLAGS
    "/MANIFEST /LTCG:incremental /OPT:REF /OPT:ICF")
else()
  bz_add_cxx_flag("-Wno-unknown-pragmas -Wno-ignored-qualifiers" "")
endif()


# ==========================================================
# add link / include
# ==========================================================
transactd_link_boost_libraries("serialization")
#tdclcpp_x on windows is auto link
if(NOT MSVC)
  bz_get_compiler_bitness()
  target_link_libraries(${this_target} "tdclcpp_${BZ_BITNESS}${TDCL_CPP_CHARSET}")
  target_link_libraries(${this_target} dl)
endif()


# ==========================================================
# set binary name
# ==========================================================
bz_get_compiler_bitness()
if(WIN32)
  ## Windows tdclstmt[_Compiler]_[32|64][m|u]_[MAJOR]_[MINOR].dll
  bz_get_compiler()
  set(TDCL_CPP_NAME_TMP "tdclstmt${BZ_COMPILER}_${BZ_BITNESS}${TDCL_CPP_CHARSET}")
  set_target_properties(${this_target} PROPERTIES OUTPUT_NAME_DEBUG "${TDCL_CPP_NAME_TMP}_${TD_RC_VER_MAJOR}_${TD_RC_VER_MINOR}")
  set_target_properties(${this_target} PROPERTIES OUTPUT_NAME_RELEASE "${TDCL_CPP_NAME_TMP}_${TD_RC_VER_MAJOR}_${TD_RC_VER_MINOR}")
  set_target_properties(${this_target} PROPERTIES OUTPUT_NAME_RELWITHDEBINFO "${TDCL_CPP_NAME_TMP}_${TD_RC_VER_MAJOR}_${TD_RC_VER_MINOR}")
  set_target_properties(${this_target} PROPERTIES OUTPUT_NAME_MINSIZEREL "${TDCL_CPP_NAME_TMP}r_${TD_RC_VER_MAJOR}_${TD_RC_VER_MINOR}")

  set_target_properties(${this_target} PROPERTIES PREFIX "")
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
