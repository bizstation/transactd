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


set(this_target tdclcpp)
project(${this_target})
bz_force_gcc() # set compiler to gcc instead of g++


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
tdcl_add_source_files("${TRANSACTD_ROOT}")
tdcl_add_include_directory("${TRANSACTD_ROOT}")


# ==========================================================
# add resource file (for Windows Visual Studio)
# ==========================================================
set(${this_target}_RC_FILE "${TRANSACTD_ROOT}/build/tdclcpp/tdclcpp.rc")
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
add_dependencies(${this_target} tdclc)


# ==========================================================
# Add compile definitions
# ==========================================================
if(MSVC)
  set_property(TARGET ${this_target} PROPERTY COMPILE_DEFINITIONS
    _CRT_SECURE_NO_WARNINGS LIB_TDCLCPP TRDCL_AUTOLINK_OFF
    BOOST_DATE_TIME_NO_LIB _USRDLL TRDCLENGN_EXPORTS _WINDLL)
else()
  set_property(TARGET ${this_target} PROPERTY COMPILE_DEFINITIONS
    PIC BOOST_ALL_NO_LIB)
  if(NOT MINGW)
    set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
      LINUX)
  else()
    set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
      WIN32 _WIN32 _SYS_TIME_H_ _GETTIMEOFDAY_DEFINED)
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
# Add compile options and linker options
# ==========================================================
transactd_reset_MTMD()
if (MSVC)
  set(CMAKE_CXX_FLAGS "/W3 /wd4068 /wd4275 /wd4819 /wd4251 /wd4996 /Gy /GL /Zi /Oi /EHa")
  if((TRANSACTD_RUBY_GEM) OR (TRANSACTD_PHP))
    bz_add_cxx_flag("/MD  /Ox /Ob2 /GR" "_RELEASE")
    bz_add_cxx_flag("/MDd             " "_DEBUG")
    bz_add_cxx_flag("/MD  /Ox /Ob2 /GR" "_RELWITHDEBINFO")
    bz_add_cxx_flag("/MD  /Ox /Ob2 /GR" "_MINSIZEREL")
  else()
    bz_add_cxx_flag("/MT  /Ox /Ob2 /GR" "_RELEASE")
    bz_add_cxx_flag("/MTd             " "_DEBUG")
    bz_add_cxx_flag("/MT  /Ox /Ob2 /GR" "_RELWITHDEBINFO")
    bz_add_cxx_flag("/MD  /Ox /Ob2 /GR" "_MINSIZEREL")
  endif()
  set(CMAKE_LINKER_FLAGS "")
  set(CMAKE_SHARED_LINKER_FLAGS "")
  if(MSVC_VERSION LESS 1800)
    set_target_properties(${this_target} PROPERTIES LINK_FLAGS
      "/MANIFEST:NO /OPT:REF /OPT:ICF")
  else()
    set_target_properties(${this_target} PROPERTIES LINK_FLAGS
      "/MANIFEST:NO /LTCG:incremental /OPT:REF /OPT:ICF")
  endif()
else()
  set(CMAKE_CXX_FLAGS "-fexceptions -frtti -fpermissive -fno-strict-aliasing -fno-omit-frame-pointer")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_SHARED_LINKER_FLAGS "-static-libgcc")
  endif()
endif()


# ==========================================================
# add link / include
# ==========================================================
if(MSVC)
  transactd_link_boost_libraries("filesystem;system;thread;chrono")
else()
  transactd_link_boost_libraries("filesystem;system;thread")
endif()
if(CAN_LINK_ICONV)
  target_link_libraries(${this_target} "${ICONV_LIBRARY}")
endif()
if(NOT WIN32)
  target_link_libraries(${this_target} dl pthread)
endif()


# ==========================================================
# set binary name
# ==========================================================
bz_get_compiler_bitness()
if(WIN32)
  ## Windows tdclcpp[_Compiler]_[32|64][m|u|mr]_[MAJOR]_[MINOR].dll
  bz_get_compiler()
  set(TDCL_CPP_NAME_TMP "tdclcpp${BZ_COMPILER}_${BZ_BITNESS}${TDCL_CPP_CHARSET}")
  
  foreach(BUILD_TYPE DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    transactd_has_MD_option("${CMAKE_CXX_FLAGS_${BUILD_TYPE}}")
    if("${IS_MD}" STREQUAL "ON")
      set_target_properties(${this_target} PROPERTIES OUTPUT_NAME_${BUILD_TYPE} "${TDCL_CPP_NAME_TMP}r_${TD_RC_VER_MAJOR}_${TD_RC_VER_MINOR}")
    else()
      set_target_properties(${this_target} PROPERTIES OUTPUT_NAME_${BUILD_TYPE} "${TDCL_CPP_NAME_TMP}_${TD_RC_VER_MAJOR}_${TD_RC_VER_MINOR}")
    endif()
  endforeach()
  set_target_properties(${this_target} PROPERTIES PREFIX "")
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
    SOVERSION ${TD_RC_VER_MAJOR}.${TD_RC_VER_MINOR}
    VERSION ${TD_RC_VER_MAJOR}.${TD_RC_VER_MINOR}.${TD_RC_VER_RELEASE})
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
