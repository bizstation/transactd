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
## Transactd PHP client with swig
##
cmake_minimum_required(VERSION 2.6)
include(../common/transactd_required.cmake)
include(../common/transactd_common.cmake)
include(../common/transactd_cl_common.cmake)
include(../common/options.cmake)
include(../common/system.cmake)
include(../common/get_boost_libs.cmake)


set(this_target tdclphp)
project(${this_target})


# ==========================================================
# options
# ==========================================================
if(NOT TRANSACTD_CLIENTS_PREFIX)
  set(TRANSACTD_CLIENTS_PREFIX "" CACHE STRING "Prefix for transactd client libraries")
endif()
if(NOT TRANSACTD_PHP_NONTHREAD)
  set(TRANSACTD_PHP_NONTHREAD OFF)
endif()


# ==========================================================
# set variables
# ==========================================================
if("${CMAKE_CURRENT_BINARY_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  get_filename_component(TRANSACTD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
  set(TRANSACTD_BINARY_ROOT "${CMAKE_BINARY_DIR}")
endif()
file(TO_CMAKE_PATH "${TRANSACTD_PHP_SOURCE_ROOT}" TRANSACTD_PHP_SOURCE_ROOT)
file(TO_CMAKE_PATH "${TRANSACTD_PHP_LIBRARY_PATH}" TRANSACTD_PHP_LIBRARY_PATH)
message(STATUS "  TRANSACTD_PHP_SOURCE_ROOT  ${TRANSACTD_PHP_SOURCE_ROOT}")
message(STATUS "  TRANSACTD_PHP_LIBRARY_PATH ${TRANSACTD_PHP_LIBRARY_PATH}")


# ==========================================================
# version info
# ==========================================================
transactd_read_file_to_int("${TRANSACTD_ROOT}/build/tdclphp/RELEASE_VERSION")
set(TDVER_PHP_RELEASE "${TRANSACTD_READ_FILE_TO_INT_RETURN}")


# ==========================================================
# read PHP version from header
# ==========================================================
file(TO_CMAKE_PATH "${TRANSACTD_PHP_SOURCE_ROOT}/main/php_version.h"
  TRANSACTD_PHP_VERSION_H_PATH)
if(EXISTS "${TRANSACTD_PHP_VERSION_H_PATH}")
  file(READ "${TRANSACTD_PHP_VERSION_H_PATH}" TRANSACTD_PHP_VERSION_H)
  string(REGEX REPLACE ".*define +PHP_VERSION +\"(.+)\".*" "\\1"
    TRANSACTD_PHP_VERSION "${TRANSACTD_PHP_VERSION_H}" )
  if("${TRANSACTD_PHP_VERSION_H}" STREQUAL "${TRANSACTD_PHP_VERSION}")
    set(TRANSACTD_PHP_VERSION "")
  endif()
else()
  set(TRANSACTD_PHP_VERSION "")
endif()
if("${TRANSACTD_PHP_VERSION}" STREQUAL "")
  message(STATUS "NOTE: PHP_VERSION not found in [${TRANSACTD_PHP_VERSION_H_PATH}].")
  set(TRANSACTD_PHP_VERSION "5.5.0-(detect)")
endif()
message(STATUS "PHP_VERSION : [${TRANSACTD_PHP_VERSION}]")
string(REGEX REPLACE "([0-9]+).[0-9]+.[0-9]+.*" "\\1"
  TRANSACTD_PHP_VERSION_MAJOR "${TRANSACTD_PHP_VERSION}" )
MATH(EXPR TRANSACTD_PHP_VERSION_MAJOR "${TRANSACTD_PHP_VERSION_MAJOR} + 0")
string(REGEX REPLACE "[0-9]+.([0-9]+).[0-9]+.*" "\\1"
  TRANSACTD_PHP_VERSION_MINOR "${TRANSACTD_PHP_VERSION}" )
MATH(EXPR TRANSACTD_PHP_VERSION_MINOR "${TRANSACTD_PHP_VERSION_MINOR} + 0")
set(TRANSACTD_PHP_HAVE_YIELD OFF)
if(${TRANSACTD_PHP_VERSION_MAJOR} GREATER 5)
  set(TRANSACTD_PHP_HAVE_YIELD ON)
elseif(${TRANSACTD_PHP_VERSION_MAJOR} EQUAL 5)
  if(${TRANSACTD_PHP_VERSION_MINOR} GREATER 4)
    set(TRANSACTD_PHP_HAVE_YIELD ON)
  endif()
endif()
message(STATUS "TRANSACTD_PHP_HAVE_YIELD : [${TRANSACTD_PHP_HAVE_YIELD}]")


# ==========================================================
# add custom command for generate swig wrapper
# ==========================================================
set(PHP_SWIG_WRAPPER_CPP "${CMAKE_CURRENT_BINARY_DIR}/tdclphp_wrap.cpp")
file(TO_NATIVE_PATH "${TRANSACTD_ROOT}/build/swig/tdcl.i" SWIG_INTERFACE_FILE)
file(TO_NATIVE_PATH "${PHP_SWIG_WRAPPER_CPP}"             SWIG_NATIVE_OUTFILE)
file(TO_NATIVE_PATH "${TRANSACTD_ROOT}"                   SWIG_INC_PATH1)
file(TO_NATIVE_PATH "${TRANSACTD_ROOT}/source"            SWIG_INC_PATH2)
set(SWIG_ADDITONAL_OPTIONS "")
# configure generate command file
if(WIN32)
  set(SWIG_GEN_CMD_WIN "${CMAKE_CURRENT_BINARY_DIR}/generate.cmd")
  configure_file("${TRANSACTD_ROOT}/build/swig/php/generate.cmd.in" "${SWIG_GEN_CMD_WIN}" @ONLY)
endif()
set(SWIG_GEN_CMD "${CMAKE_CURRENT_BINARY_DIR}/generate.cmake")
configure_file("${TRANSACTD_ROOT}/build/swig/php/generate.cmake.in" "${SWIG_GEN_CMD}" @ONLY)
# add generate command to build time
add_custom_command (
  OUTPUT  "${PHP_SWIG_WRAPPER_CPP}"
  COMMAND ${CMAKE_COMMAND} -P ${SWIG_GEN_CMD}
  COMMENT "Generating tdclphp_wrap.cpp"
  DEPENDS "${TRANSACTD_ROOT}/build/swig/tdcl.i"
          "${TRANSACTD_ROOT}/build/swig/php/php.swg")


# ==========================================================
# add swig wrapper source file
# ==========================================================
set(${this_target}_SOURCE_FILES
  ${${this_target}_SOURCE_FILES}
  ${PHP_SWIG_WRAPPER_CPP}
  ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/connectionPool.cpp
  ${TRANSACTD_ROOT}/source/bzs/rtl/benchmark.cpp
)


# ==========================================================
# add include path
# ==========================================================
tdcl_add_include_directory("${TRANSACTD_ROOT}")
include_directories(${TRANSACTD_ROOT}
  ${TRANSACTD_PHP_SOURCE_ROOT}
  ${TRANSACTD_PHP_SOURCE_ROOT}/main
  ${TRANSACTD_PHP_SOURCE_ROOT}/TSRM
  ${TRANSACTD_PHP_SOURCE_ROOT}/Zend
  ${TRANSACTD_PHP_SOURCE_ROOT}/ext
  ${TRANSACTD_PHP_SOURCE_ROOT}/ext/date/lib
  ${TRANSACTD_PHP_INCLUDE_PATH})


# ==========================================================
# add resource file (for Windows Visual Studio)
# ==========================================================
set(${this_target}_RC_FILE "${TRANSACTD_ROOT}/build/tdclphp/tdclphp.rc")
transactd_read_rc("${${this_target}_RC_FILE}")
if(WIN32)
  transactd_add_rc_file()
endif()


# ==========================================================
# add_library
# ==========================================================
if(APPLE)
  add_library(${this_target} MODULE ${${this_target}_SOURCE_FILES})
  set_target_properties(${this_target} PROPERTIES LINK_FLAGS "-bundle -undefined dynamic_lookup")
else()
  add_library(${this_target} SHARED ${${this_target}_SOURCE_FILES})
endif()


# ==========================================================
# Add compile options
# ==========================================================
transactd_set_MTMD("MD")


# ==========================================================
# Add compile definitions
# ==========================================================
set_property(TARGET ${this_target} PROPERTY COMPILE_DEFINITIONS
  SWIGPHP SWIG_BUILDING USE_DBM_CONNECTION_POOL BOOST_ALL_NO_LIB)
bz_get_compiler_bitness()
if(MSVC90)
  set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
    _SECURE_SCL=0)
elseif(MSVC11)
  set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
    _ALLOW_KEYWORD_MACROS)
endif()
if(TRANSACTD_PHP_ZTS)
  set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
    ZTS=1)
endif()
set_property(TARGET ${this_target} PROPERTY COMPILE_DEFINITIONS_DEBUG
  ZEND_DEBUG=1)
set_property(TARGET ${this_target} PROPERTY COMPILE_DEFINITIONS_RELWITHDEBINFO
  ZEND_DEBUG=1)
set_property(TARGET ${this_target} PROPERTY COMPILE_DEFINITIONS_RELEASE
  ZEND_DEBUG=0)
set_property(TARGET ${this_target} PROPERTY COMPILE_DEFINITIONS_MINSIZEREL
  ZEND_DEBUG=0)
if(MSVC)
  set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
    WIN32 _WIN32 _WINDOWS ZEND_WIN32 PHP_WIN32 ZEND_WIN32_FORCE_INLINE)
  if(TRANSACTD_PHP_USE_32BIT_TIME_T AND ("${BZ_BITNESS}" STREQUAL "32"))
    set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
      _USE_32BIT_TIME_T)
  endif()
else()
  if(NOT MINGW)
    set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
      LINUX)
  else()
    set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
      WIN32 _WIN32)
  endif()
endif()


# ==========================================================
# add link / include
# ==========================================================
target_link_libraries(${this_target} "${TRANSACTD_PHP_LIBRARY_PATH}")
transactd_link_boost_libraries("timer")
if(MSVC)
  target_link_libraries(${this_target} "tdclcppmr")
else()
  target_link_libraries(${this_target} "tdclcppm")
endif()


# ==========================================================
# set binary name
# ==========================================================
set_target_properties(${this_target} PROPERTIES PREFIX "")
set_target_properties(${this_target} PROPERTIES OUTPUT_NAME php_transactd)
if(NOT WIN32)
  set_target_properties(${this_target} PROPERTIES SUFFIX ".so")
endif()
if(UNIX AND NOT APPLE)
  set_target_properties(${this_target} PROPERTIES
    SOVERSION ${TD_RC_VER_MAJOR}.${TD_RC_VER_MINOR}
    VERSION ${TD_RC_VER_MAJOR}.${TD_RC_VER_MINOR}.${TDVER_PHP_RELEASE})
endif()


# ==========================================================
# set output directory
# ==========================================================
if("${TRANSACTD_PHP_HAVE_YIELD}" STREQUAL "ON")
  set(tdcl_yield ".yield")
else()
  set(tdcl_yield ".no_yield")
endif()
set(TRANSACTD_PHP_FILENAME "transactd${tdcl_yield}.php")
if(WIN32)
  if(NOT COMMAND tdcl_set_output_win)
    include(../common/transactd_cl_output.cmake)
  endif()
  tdcl_set_output_win(${TRANSACTD_BINARY_ROOT})
else()
  set_target_properties(${this_target} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${TRANSACTD_BINARY_ROOT}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${TRANSACTD_BINARY_ROOT}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${TRANSACTD_BINARY_ROOT}/bin")
endif()
add_custom_command(TARGET ${this_target} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  ${TRANSACTD_ROOT}/build/swig/php/${TRANSACTD_PHP_FILENAME}
  ${TRANSACTD_BINARY_ROOT}/bin/transactd.php)
