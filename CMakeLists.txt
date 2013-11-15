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

# ==========================================================
# options
# ==========================================================
option(WITH_TRANSACTD_SERVER    "Build Transactd server plugin"                   ON)
option(WITH_TRANSACTD_CLIENTS   "Build Transactd client libraries"                ON)
option(TRANSACTD_HANDLERSOCKET  "Add HandlerSocket protocol to Transactd plugin"  ON)
option(TRANSACTD_RUBY_GEM       "Build Transactd ruby gem"                        OFF)
set(TRANSACTD_PREFIX  "/usr/local/transactd"  CACHE STRING "Prefix for transactd executables(test,benchmark)")
set(TRANSACTD_CLIENTS_PREFIX    ""            CACHE STRING "Prefix for transactd client libraries")

#   The following options are set automatically if not specified.
#   Usually you do not have to set these values.
set(TRANSACTD_RUBY_GEM_ROOT_PATH    "" CACHE STRING "Transactd gem root path")
set(TRANSACTD_RUBY_EXECUTABLE_PATH  "" CACHE STRING "Ruby executable path")
set(TRANSACTD_RUBY_INCLUDE_PATH     "" CACHE STRING "Include path for ruby-swig")
set(TRANSACTD_RUBY_LIBRARY_PATH     "" CACHE STRING "Include path for ruby")


# ==========================================================
# set variables
# ==========================================================
get_filename_component(TRANSACTD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
set(TRANSACTD_BINARY_ROOT       "${CMAKE_CURRENT_BINARY_DIR}")
if(TRANSACTD_RUBY_GEM)
  set(WITH_TRANSACTD_SERVER   OFF)
  set(WITH_TRANSACTD_CLIENTS  OFF)
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
    project(tdcl)
  endif()
  subdirs("build/tdclc")
  if(WIN32 AND NOT MINGW)
    set(TDCL_CPP_CHARSET u)
  else()
    set(TDCL_CPP_CHARSET m)
  endif()
  foreach(srcname tdclcpp test_tdcl bench_td)
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
  if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    project(tdcl)
  endif()
  transactd_copy_subdir("${TRANSACTD_ROOT}" tdclcpp tdclcppm)
  subdirs("build/tdclrb")
endif()
