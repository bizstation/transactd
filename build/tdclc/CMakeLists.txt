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
## Transactd C client
##
cmake_minimum_required(VERSION 2.6)
include(../common/transactd_required.cmake)
include(../common/transactd_common.cmake)
include(../common/options.cmake)
include(../common/system.cmake)
include(../common/get_boost_libs.cmake)


set(this_target tdclc)
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
# source files
# ==========================================================
set(${this_target}_SOURCE_FILES
  ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/tdapSchema.cpp
  ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/client.cpp
  ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/dllmain.cpp
  ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/sqlBuilder.cpp
  ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/mysql/characterset.cpp
  ${TRANSACTD_ROOT}/source/bzs/netsvc/client/tcpClient.cpp
  ${TRANSACTD_ROOT}/source/bzs/rtl/stl_uty.cpp
)
if(APPLE)
  set(${this_target}_SOURCE_FILES ${${this_target}_SOURCE_FILES}
    ${TRANSACTD_ROOT}/source/bzs/env/crosscompile.cpp
    ${TRANSACTD_ROOT}/source/bzs/env/mbcswchrLinux.cpp
  )
endif()


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
  if(MSVC)
    bz_add_cxx_flag("/EHa /wd4068 /D_WIN32_WINNT=0x0501 /DPACKAGE_NO_EXPORT" "${build_type}")
    bz_set_MTMTd_cxx_flag("${build_type}")
    bz_set_MBCS_cxx_flag("${build_type}")
  else()
    bz_add_cxx_flag("-fPIC -fabi-version=2 -fexceptions -fno-omit-frame-pointer" "${build_type}")
    bz_add_cxx_flag("-fno-strict-aliasing -fpermissive -frtti" "${build_type}")
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
    F_VER   "${TDVER_C_INTERFACE_VER_MAJOR}.${TDVER_C_INTERFACE_VER_MINOR}.${TDVER_C_INTERFACE_VER_RELEASE}.${TRANSACTD_BUILD_NUMBER}"
    P_VER   "${TDVER_C_INTERFACE_VER_MAJOR}.${TDVER_C_INTERFACE_VER_MINOR}.${TDVER_C_INTERFACE_VER_RELEASE}.${TRANSACTD_BUILD_NUMBER}"
    P_NAME  "Transactd C Client"
    F_DESC  "Transactd C Client module"
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
transactd_link_boost_libraries("filesystem;system;thread")
if(APPLE)
  target_link_libraries(${this_target} iconv)
endif()


# ==========================================================
# set binary name
# ==========================================================
bz_get_compiler_bitness()
if(WIN32)
  ## Windows tdclc_[32|64]_[MAJOR]_[MINOR].dll
  set_target_properties(${this_target} PROPERTIES PREFIX "")
  set_target_properties(${this_target} PROPERTIES OUTPUT_NAME
      "tdclc_${BZ_BITNESS}_${TDVER_C_INTERFACE_VER_MAJOR}_${TDVER_C_INTERFACE_VER_MINOR}")
  set_target_properties(${this_target} PROPERTIES SUFFIX ".dll")
else()
  ## Linux libtdclc_[32|64].so.[MAJOR].[MINOR]
  set_target_properties(${this_target} PROPERTIES PREFIX "lib")
  set_target_properties(${this_target} PROPERTIES OUTPUT_NAME "tdclc_${BZ_BITNESS}")
  set_target_properties(${this_target} PROPERTIES SUFFIX ".so")
  set_target_properties(${this_target} PROPERTIES
    SOVERSION ${TDVER_C_INTERFACE_VER_MAJOR}.${TDVER_C_INTERFACE_VER_MINOR}
    VERSION   ${TDVER_C_INTERFACE_VER_MAJOR}.${TDVER_C_INTERFACE_VER_MINOR}.${TDVER_C_INTERFACE_VER_RELEASE})
endif()


# ==========================================================
# add .def file (for MSVC)
# ==========================================================
if(MSVC)
  set_target_properties(${this_target} PROPERTIES LINK_FLAGS
    "/DEF:\"${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/trnsctcl.def\"")
endif()


# ==========================================================
# set output directory
# ==========================================================
if(NOT COMMAND tdcl_set_output)
  include(../common/transactd_cl_output.cmake)
endif()
tdcl_set_output("${TRANSACTD_BINARY_ROOT}" "${TRANSACTD_CLIENTS_PREFIX}")
