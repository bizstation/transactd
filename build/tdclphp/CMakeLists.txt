##=================================================================
#   Copyright (C) 2012 - 2016 BizStation Corp All rights reserved.
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

if(APPLE)
  transactd_set_rpath("${TRANSACTD_CLIENTS_PREFIX}")
endif()


# ==========================================================
# find php package
# ==========================================================
set(PHPLIBS_FOUND "FALSE")
if (NOT TRANSACTD_PHP_SOURCE_ROOT)
  set(PHPLibs_DIR "./")
  find_package(PHPLibs)

  if (${PHPLIBS_FOUND} STREQUAL "TRUE")
     set(TRANSACTD_PHP_SOURCE_ROOT ${PHP_INCLUDE_DIR})
     set(TRANSACTD_PHP_ZTS ${PHP_ZTS})
  endif()

  message(STATUS  "PHPLIBS_FOUND = ${PHPLIBS_FOUND}")
  message(STATUS  "PHP_VERSION_STRING = ${PHP_VERSION_STRING}")
  message(STATUS  "PHP_LIBRARIES = ${PHP_LIBRARIES}")
  message(STATUS  "PHP_INCLUDE_DIRS = ${PHP_INCLUDE_DIRS}")
  message(STATUS  "PHP_INCLUDE_DIR = ${PHP_INCLUDE_DIR}")
  message(STATUS  "PHP_EXTENSION_DIR = ${PHP_EXTENSION_DIR}")
  message(STATUS  "PHP_EXECUTABLE = ${PHP_EXECUTABLE}")
  message(STATUS  "PHP_INSTALL_PREFIX = ${PHP_INSTALL_PREFIX}")
  message(STATUS  "PHP_ZTS = ${PHP_ZTS}")
  message(STATUS  "PHP_INFO_INCLUDE = ${PHP_INFO_INCLUDE}")
  message(STATUS  "PHP_INFO_FILE = ${PHP_INFO_FILE}")
  message(STATUS  "PHP_TRANSACTD_ENABLED = ${PHP_TRANSACTD_ENABLED}")

endif()



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
#message(STATUS "  TRANSACTD_PHP_SOURCE_ROOT  ${TRANSACTD_PHP_SOURCE_ROOT}")
#message(STATUS "  TRANSACTD_PHP_LIBRARY_PATH ${TRANSACTD_PHP_LIBRARY_PATH}")

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
# read PHP version from header
# ==========================================================
#file(TO_CMAKE_PATH "${TRANSACTD_PHP_SOURCE_ROOT}/main/php_version.h"
#  TRANSACTD_PHP_VERSION_H_PATH)
#if(EXISTS "${TRANSACTD_PHP_VERSION_H_PATH}")
#  file(READ "${TRANSACTD_PHP_VERSION_H_PATH}" TRANSACTD_PHP_VERSION_H)
#  string(REGEX REPLACE ".*define +PHP_VERSION +\"(.+)\".*" "\\1"
#    TRANSACTD_PHP_VERSION "${TRANSACTD_PHP_VERSION_H}" )
#  if("${TRANSACTD_PHP_VERSION_H}" STREQUAL "${TRANSACTD_PHP_VERSION}")
#    set(TRANSACTD_PHP_VERSION "")
#  endif()
#else()
#  set(TRANSACTD_PHP_VERSION "")
#endif()
#if("${TRANSACTD_PHP_VERSION}" STREQUAL "")
#  message(STATUS "NOTE: PHP_VERSION not found in [${TRANSACTD_PHP_VERSION_H_PATH}].")
#  set(TRANSACTD_PHP_VERSION "5.5.0-(detect)")
#endif()
#message(STATUS "PHP_VERSION : [${TRANSACTD_PHP_VERSION}]")
#string(REGEX REPLACE "([0-9]+).[0-9]+.[0-9]+.*" "\\1"
#  TRANSACTD_PHP_VERSION_MAJOR "${TRANSACTD_PHP_VERSION}" )
#MATH(EXPR TRANSACTD_PHP_VERSION_MAJOR "${TRANSACTD_PHP_VERSION_MAJOR} + 0")
#string(REGEX REPLACE "[0-9]+.([0-9]+).[0-9]+.*" "\\1"
#  TRANSACTD_PHP_VERSION_MINOR "${TRANSACTD_PHP_VERSION}" )
#MATH(EXPR TRANSACTD_PHP_VERSION_MINOR "${TRANSACTD_PHP_VERSION_MINOR} + 0")
#set(TRANSACTD_PHP_HAVE_YIELD OFF)
#if(${TRANSACTD_PHP_VERSION_MAJOR} GREATER 5)
#  set(TRANSACTD_PHP_HAVE_YIELD ON)
#elseif(${TRANSACTD_PHP_VERSION_MAJOR} EQUAL 5)
#  if(${TRANSACTD_PHP_VERSION_MINOR} GREATER 4)
#    set(TRANSACTD_PHP_HAVE_YIELD ON)
#  endif()
#endif()
#message(STATUS "TRANSACTD_PHP_HAVE_YIELD : [${TRANSACTD_PHP_HAVE_YIELD}]")


# ==========================================================
# add swig wrapper source file
# ==========================================================
set(${this_target}_SOURCE_FILES
  ${${this_target}_SOURCE_FILES}
  ${TRANSACTD_ROOT}/build/swig/php/tdclphp_wrap.cpp
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
# read and add resource file
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
# add dependencie projects
# ==========================================================
add_dependencies(${this_target} tdclc tdclcpp)

# ==========================================================
# Add compile options
# ==========================================================
transactd_set_MTMD("MD")
if (MSVC)
  foreach(build_type "" "_RELEASE" "_DEBUG" "_RELWITHDEBINFO" "_MINSIZEREL")
    bz_add_cxx_flag("/Ox /Ob2 /Ot /wd4267 /wd4102 /wd4800" "${build_type}")
  endforeach()
endif()

# ==========================================================
# Add compile definitions
# ==========================================================
set_property(TARGET ${this_target} PROPERTY COMPILE_DEFINITIONS
  SWIG_BUILDING USE_DBM_CONNECTION_POOL BOOST_ALL_NO_LIB)
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
    WIN32 _WIN32 _WINDOWS ZEND_WIN32 PHP_WIN32 ZEND_WIN32_FORCE_INLINE TRDCL_AUTOLINK)
  if(TRANSACTD_PHP_USE_32BIT_TIME_T AND ("${BZ_BITNESS}" STREQUAL "32"))
    set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
      _USE_32BIT_TIME_T)
  endif()
else()
  set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
    _REENTRANT)
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
transactd_link_boost_libraries("timer;filesystem;thread;chrono;system")
#tdclcpp_x on windows is auto link
if(NOT MSVC)
  target_link_libraries(${this_target} "tdclcpp_64m")
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
    VERSION ${TD_RC_VER_MAJOR}.${TD_RC_VER_MINOR}.${TD_RC_VER_RELEASE})
endif()


# ==========================================================
# set output directory
# ==========================================================
#if("${TRANSACTD_PHP_HAVE_YIELD}" STREQUAL "ON")
#  set(tdcl_yield ".yield")
#else()
#  set(tdcl_yield ".no_yield")
#endif()
#set(TRANSACTD_PHP_FILENAME "transactd${tdcl_yield}.php")

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
  ${TRANSACTD_ROOT}/build/swig/php/transactd.php
  ${TRANSACTD_BINARY_ROOT}/bin/transactd.php)

# ==========================================================
# install 
# ==========================================================
if (${PHPLIBS_FOUND} STREQUAL "TRUE")
  install(FILES  "${TRANSACTD_BINARY_ROOT}/bin/php_transactd.so" DESTINATION
    "${PHP_EXTENSION_DIR}/")
  if (NOT PHP_INFO_INCLUDE)
    install(CODE "MESSAGE(STATUS \"Warning! php include path is not found. ${TRANSACTD_BINARY_ROOT}/bin/transactd.php will not be installed automatically.\")")
  else()  
    install(FILES  "${TRANSACTD_BINARY_ROOT}/bin/transactd.php" DESTINATION
      "${PHP_INFO_INCLUDE}/")
  endif()
  if(${PHP_TRANSACTD_ENABLED} STREQUAL "NO")
    install(CODE "file(APPEND ${PHP_INFO_FILE} \"extension=php_transactd.so\\n\")")
  endif()
endif()
