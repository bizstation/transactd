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
include(../common/check_for_link_iconv.cmake)
check_for_link_iconv()


set(this_target tdclc)
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
if(MSVC)
  set(${this_target}_SOURCE_FILES ${${this_target}_SOURCE_FILES}
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/myDateTime.cpp
  )
endif()
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
# add resource file (for Windows Visual Studio)
# ==========================================================
set(${this_target}_RC_FILE "${TRANSACTD_ROOT}/build/tdclc/tdclc.rc")
transactd_read_rc("${${this_target}_RC_FILE}")
if(WIN32)
  transactd_add_rc_file()
endif()


# ==========================================================
# add_library
# ==========================================================
if(APPLE)
  add_library(${this_target} MODULE ${${this_target}_SOURCE_FILES})
else()
  add_library(${this_target} SHARED ${${this_target}_SOURCE_FILES})
endif()


# ==========================================================
# Add compile definitions
# ==========================================================
if(MSVC)
  set_property(TARGET ${this_target} PROPERTY COMPILE_DEFINITIONS
    _WIN32_WINNT=0x0501 PACKAGE_NO_EXPORT BOOST_DATE_TIME_NO_LIB BOOST_REGEX_NO_LIB _USRDLL _MBCS)
else()
  set_property(TARGET ${this_target} PROPERTY COMPILE_DEFINITIONS
    PIC BOOST_ALL_NO_LIB)
  if(NOT MINGW)
    set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
      LINUX)
  endif()
endif()


# ==========================================================
# Add compile options and linker options
# ==========================================================
transactd_reset_MTMD()
if (MSVC)
  set(CMAKE_CXX_FLAGS "/W3 /wd4068 /Gy /GL /Zi /Oi /EHa")
  bz_add_cxx_flag("/MT  /Ot /Ox /Ob2" "_RELEASE")
  bz_add_cxx_flag("/MTd             " "_DEBUG")
  bz_add_cxx_flag("/MT  /Ot /Ox /Ob2" "_RELWITHDEBINFO")
  bz_add_cxx_flag("/MT  /Ot /Ox /Ob2" "_MINSIZEREL")
  set(CMAKE_LINKER_FLAGS "")
  set(CMAKE_SHARED_LINKER_FLAGS "")
  if(MSVC_VERSION LESS 1800)
    set_target_properties(${this_target} PROPERTIES LINK_FLAGS
      "/MANIFEST /OPT:REF /OPT:ICF /DEF:\"${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/trnsctcl.def\"")
  else()
    set_target_properties(${this_target} PROPERTIES LINK_FLAGS
      "/MANIFEST /LTCG:incremental /OPT:REF /OPT:ICF /DEF:\"${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/trnsctcl.def\"")
  endif()
else()
  set(CMAKE_CXX_FLAGS "-fexceptions -frtti -fpermissive -fno-strict-aliasing -fno-omit-frame-pointer")
endif()


# ==========================================================
# add link / include
# ==========================================================
transactd_link_boost_libraries("chrono;filesystem;system;thread")
if(CAN_LINK_ICONV)
  target_link_libraries(${this_target} "${ICONV_LIBRARY}")
endif()
if(NOT MSVC)
  target_link_libraries(${this_target} dl)
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    ## target_link_libraries(${this_target} "${TRANSACTD_ROOT}/build/tdclc/libtdclcm.map")
  elseif(APPLE)
    target_link_libraries(${this_target} -Wl,-map,"${TRANSACTD_ROOT}/build/tdclc/libtdclcm.map")
  else()
    target_link_libraries(${this_target} -Wl,--version-script,"${TRANSACTD_ROOT}/build/tdclc/libtdclcm.map")
  endif()
endif()


# ==========================================================
# set binary name
# ==========================================================
bz_get_compiler_bitness()
if(WIN32)
  ## Windows tdclc_[32|64]_[MAJOR]_[MINOR].dll
  set_target_properties(${this_target} PROPERTIES PREFIX "")
  set_target_properties(${this_target} PROPERTIES OUTPUT_NAME
      "tdclc_${BZ_BITNESS}_${TD_RC_VER_MAJOR}_${TD_RC_VER_MINOR}")
  set_target_properties(${this_target} PROPERTIES SUFFIX ".dll")
else()
  ## Linux libtdclc_[32|64].so.[MAJOR].[MINOR]
  set_target_properties(${this_target} PROPERTIES PREFIX "lib")
  set_target_properties(${this_target} PROPERTIES OUTPUT_NAME "tdclc_${BZ_BITNESS}")
  if(APPLE)
    if(NOT COMMAND tdcl_set_outputnames_osx)
      include(../common/transactd_cl_output.cmake)
    endif()
    tdcl_set_outputnames_osx(
      "libtdclc_${BZ_BITNESS}" "${TD_RC_VER_MAJOR}"
      "${TD_RC_VER_MINOR}" "${TD_RC_VER_RELEASE}"
      "${TRANSACTD_ROOT}")
  else()
    set_target_properties(${this_target} PROPERTIES SUFFIX ".so")
    set_target_properties(${this_target} PROPERTIES
      SOVERSION ${TD_RC_VER_MAJOR}.${TD_RC_VER_MINOR}
      VERSION   ${TD_RC_VER_MAJOR}.${TD_RC_VER_MINOR}.${TD_RC_VER_RELEASE})
  endif()
endif()


# ==========================================================
# set output directory
# ==========================================================
if(NOT COMMAND tdcl_set_output)
  include(../common/transactd_cl_output.cmake)
endif()
tdcl_set_output("${TRANSACTD_BINARY_ROOT}" "${TRANSACTD_CLIENTS_PREFIX}")
