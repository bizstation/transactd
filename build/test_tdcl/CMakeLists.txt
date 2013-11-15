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
## Build Transactd client library test program
##
## *** original file is ../test_tdcl/CMakeLists.txt ***
##
cmake_minimum_required(VERSION 2.6)
include(../common/transactd_required.cmake)
include(../common/transactd_common.cmake)
include(../common/options.cmake)
include(../common/system.cmake)
include(../common/get_boost_libs.cmake)


get_filename_component(this_target "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
string(REPLACE test_tdcl "" TDCL_CPP_CHARSET "${this_target}")
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
  ${TRANSACTD_ROOT}/source/bzs/test/trdclengn/test_trdclengn.cpp)


# ==========================================================
# include directories
# ==========================================================
include_directories(${TRANSACTD_ROOT}/source;)
if(UNIX)
  include_directories(${TRANSACTD_ROOT}/source/linux;)
endif()


# ==========================================================
# compiler and linker options
# ==========================================================
foreach(build_type "" "_RELEASE" "_DEBUG" "_RELWITHDEBINFO" "_MINSIZEREL")
  bz_add_cxx_flag("-DBOOST_ALL_NO_LIB -DPIC" "${build_type}")
  bz_remove_cxx_flag("-fno-exceptions" "${build_type}")
  bz_remove_cxx_flag("-fno-rtti" "${build_type}")
  bz_remove_cxx_flag("-fno-implicit-templates" "${build_type}")
  bz_remove_cxx_flag("/EHsc" "${build_type}")
  bz_remove_cxx_flag("/EHs" "${build_type}")
  bz_remove_cxx_flag("-DPACKAGE_NO_EXPORT" "${build_type}")
  if(MSVC)
    bz_add_cxx_flag("/EHa /wd4068 /DTRDCL_AUTOLINK" "${build_type}")
    bz_set_MTMTd_cxx_flag("${build_type}")
    if("${TDCL_CPP_CHARSET}" STREQUAL "u")
      bz_set_UNICODE_cxx_flag("${build_type}")
    elseif("${TDCL_CPP_CHARSET}" STREQUAL "m")
      bz_set_MBCS_cxx_flag("${build_type}")
    endif()
  else()
    bz_add_cxx_flag("-fPIC -fabi-version=2 -fexceptions -finput-charset=utf-8" "${build_type}")
    bz_add_cxx_flag("-fno-omit-frame-pointer -fno-strict-aliasing -fpermissive -frtti" "${build_type}")
    if(NOT MINGW)
      bz_add_cxx_flag("-DLINUX" "${build_type}")
    endif()
  endif()
endforeach()


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
    P_NAME  "Transactd CPP Client test program"
    F_DESC  "Transactd CPP Client test program"
  )
  transactd_add_rc_file()
endif()


# ==========================================================
# add_executable and set output name
# ==========================================================
add_executable(${this_target} ${${this_target}_SOURCE_FILES})
bz_get_compiler_bitness()
set_target_properties(${this_target} PROPERTIES OUTPUT_NAME "test_tdcl_${BZ_BITNESS}${TDCL_CPP_CHARSET}")


# ==========================================================
# add link / include
# ==========================================================
transactd_link_boost_libraries("")
if(TARGET tdclcpp${TDCL_CPP_CHARSET})
  target_link_libraries(${this_target} "tdclcpp${TDCL_CPP_CHARSET}")
else()
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
