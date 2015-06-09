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
## Build Transactd server plugin and client libraries
##
cmake_minimum_required(VERSION 2.6)
include(build/common/transactd_required.cmake)
include(build/common/transactd_common.cmake)
include(build/common/transactd_cl_common.cmake)
include(build/common/options.cmake)

# ==========================================================
# options
# ==========================================================
option(WITH_TRANSACTD_SERVER    "Build Transactd server plugin"                   ON)
option(WITH_TRANSACTD_CLIENTS   "Build Transactd client libraries"                ON)
option(TRANSACTD_HANDLERSOCKET  "Add HandlerSocket protocol to Transactd plugin"  ON)
set(TRANSACTD_PREFIX  "/usr/local/transactd"  CACHE STRING "Prefix for transactd executables")
set(TRANSACTD_CLIENTS_PREFIX    ""            CACHE STRING "Prefix for transactd client libraries")
  # options for Ruby client
option(TRANSACTD_RUBY_GEM       "Build Transactd ruby gem"                        OFF)
  # options for PHP client
option(TRANSACTD_PHP            "Build Transactd client PHP extension"            OFF)
set(TRANSACTD_PHP_SOURCE_ROOT   "" CACHE STRING "Path of php source directory")
set(TRANSACTD_PHP_LIBRARY_PATH  "" CACHE STRING "Library path for php")
option(TRANSACTD_PHP_ZTS        "Add ZTS flag for build Transactd client PHP extension" OFF)

#   The following options are set automatically if not specified.
#   Usually you do not have to set these values.
option(TRANSACTD_WITHOUT_C_CPP_CLIENTS "Not build C/C++ clients" OFF)
  # options for Ruby client
set(TRANSACTD_RUBY_GEM_ROOT_PATH    "" CACHE STRING "Transactd gem root path")
set(TRANSACTD_RUBY_EXECUTABLE_PATH  "" CACHE STRING "Ruby executable path")
set(TRANSACTD_RUBY_INCLUDE_PATH     "" CACHE STRING "Include path for ruby")
set(TRANSACTD_RUBY_LIBRARY_PATH     "" CACHE STRING "Library path for ruby")
  # options for PHP client
set(TRANSACTD_PHP_INCLUDE_PATH      "" CACHE STRING "Include path for PHP")
option(TRANSACTD_PHP_USE_32BIT_TIME_T 
  "Add _USE_32BIT_TIME_T option for building Transactd client PHP extension"      ON)
set(TDCL_CPP_CHARSET  "" CACHE STRING "The charset of Transactd clients [m]ultibyte [u]nicode")
option(TRANSACTD_HAVE_RB_THREAD_CALL_WITHOUT_GVL 
  "Add HAVE_RB_THREAD_CALL_WITHOUT_GVL option for building Transactd client Ruby gem" ON)
option(TRANSACTD_HAVE_RB_THREAD_BLOCKING_REGION 
  "Add HAVE_RB_THREAD_BLOCKING_REGION option for building Transactd client Ruby gem" OFF)


# ==========================================================
# set variables
# ==========================================================
get_filename_component(TRANSACTD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
set(TRANSACTD_BINARY_ROOT       "${CMAKE_CURRENT_BINARY_DIR}")
if(TRANSACTD_RUBY_GEM)
  set(WITH_TRANSACTD_SERVER   OFF)
  set(WITH_TRANSACTD_CLIENTS  OFF)
endif()
if(TRANSACTD_PHP)
  set(WITH_TRANSACTD_SERVER   OFF)
  set(WITH_TRANSACTD_CLIENTS  OFF)
endif()
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Type of build")
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


# ==========================================================
# build server plugin
# ==========================================================
if(WITH_TRANSACTD_SERVER)
  message(STATUS "BUILD Transactd Server Plugin")
  subdirs("build/transactd")
endif()


# ==========================================================
# build client libraries
# ==========================================================
if(WITH_TRANSACTD_CLIENTS)
  message(STATUS "BUILD Transactd Client Libraries")
  if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    # build without server plugin
    project(tdcl)
  endif()
  tdcl_set_compiler_flags()
  subdirs("build/tdclc")
  if(MSVC AND (NOT ("${TDCL_CPP_CHARSET}" STREQUAL "m")))
    set(TDCL_CPP_CHARSET u)
  else()
    set(TDCL_CPP_CHARSET m)
  endif()
  foreach(srcname tdclcpp test_tdcl bench_td bench_query tdclstmt querystmts ormsrcgen)
    transactd_copy_subdir(
      "${TRANSACTD_ROOT}" "${srcname}" "${srcname}${TDCL_CPP_CHARSET}")
    subdirs("build/${srcname}${TDCL_CPP_CHARSET}")
  endforeach()
endif()


# ==========================================================
# build Ruby Gem
# ==========================================================
if(TRANSACTD_RUBY_GEM)
  message(STATUS "BUILD Transactd Ruby Gem")
  set(TDCL_CPP_CHARSET m)
  project(tdcl)
  tdcl_set_compiler_flags()
  if(NOT TRANSACTD_WITHOUT_C_CPP_CLIENTS)
    if(WIN32)
      if(MSVC)
        transactd_copy_subdir("${TRANSACTD_ROOT}" tdclcpp tdclcppmr)
        subdirs("build/tdclcppmr")
      endif()
    else()
      transactd_copy_subdir("${TRANSACTD_ROOT}" tdclcpp tdclcppm)
      subdirs("build/tdclc")
      subdirs("build/tdclcppm")
    endif()
  endif()
  subdirs("build/tdclrb")
endif()


# ==========================================================
# build PHP extension
# ==========================================================
if(TRANSACTD_PHP)
  message(STATUS "BUILD Transactd client PHP extension")
  set(TDCL_CPP_CHARSET m)
  project(tdcl)
  tdcl_set_compiler_flags()
  if(NOT TRANSACTD_WITHOUT_C_CPP_CLIENTS)
    subdirs("build/tdclc")
    if(MSVC)
      transactd_copy_subdir("${TRANSACTD_ROOT}" tdclcpp tdclcppmr)
      subdirs("build/tdclcppmr")
    else()
      transactd_copy_subdir("${TRANSACTD_ROOT}" tdclcpp tdclcppm)
      subdirs("build/tdclcppm")
    endif()
  endif()
  subdirs("build/tdclphp")
endif()
