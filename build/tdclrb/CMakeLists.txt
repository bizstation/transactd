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
## Transactd ruby client with swig
##
cmake_minimum_required(VERSION 2.6)
include(../common/transactd_required.cmake)
include(../common/transactd_common.cmake)
include(../common/transactd_cl_common.cmake)
include(../common/options.cmake)
include(../common/system.cmake)
include(../common/get_boost_libs.cmake)
include(../common/get_ruby_path.cmake)
include(../common/smart_install.cmake)


set(this_target tdclrb)
project(${this_target})


# ==========================================================
# options
# ==========================================================
if(NOT TRANSACTD_CLIENTS_PREFIX)
  set(TRANSACTD_CLIENTS_PREFIX "" CACHE STRING "Prefix for transactd client libraries")
endif()
if(NOT TRANSACTD_RUBY_GEM_ROOT_PATH)
  set(TRANSACTD_RUBY_GEM_ROOT_PATH "" CACHE STRING "Transactd gem root path")
endif()


# ==========================================================
# set variables
# ==========================================================
if("${CMAKE_CURRENT_BINARY_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  get_filename_component(TRANSACTD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
  set(TRANSACTD_BINARY_ROOT "${CMAKE_BINARY_DIR}")
endif()
get_ruby_path("${TRANSACTD_RUBY_EXECUTABLE_PATH}")
if("${TRANSACTD_RUBY_INCLUDE_PATH}" STREQUAL "")
  set(TRANSACTD_RUBY_INCLUDE_PATH "${RUBY_INCLUDE_DIRS}")
endif()
if("${TRANSACTD_RUBY_LIBRARY_PATH}" STREQUAL "")
  set(TRANSACTD_RUBY_LIBRARY_PATH "${RUBY_LIB_PATH}")
endif()
#message(STATUS "  TRANSACTD_RUBY_INCLUDE_PATH ${TRANSACTD_RUBY_INCLUDE_PATH}")
#message(STATUS "  TRANSACTD_RUBY_LIBRARY_PATH ${TRANSACTD_RUBY_LIBRARY_PATH}")
set(TDCL_GEM_PREFIX "${RUBY_VERSION_FOR_GEM}")
if("${TRANSACTD_RUBY_GEM_ROOT_PATH}" STREQUAL "")
  set(TRANSACTD_RUBY_GEM_ROOT_PATH "${TRANSACTD_BINARY_ROOT}")
endif()

# ==========================================================
# add library path
# ==========================================================
if(WIN32)
  link_directories("${TRANSACTD_RUBY_GEM_ROOT_PATH}/bin/common")
else()
 link_directories("../tdclcpp")
endif()


# ==========================================================
# add swig wrapper source file
# ==========================================================
if(WIN32 AND (NOT MSVC))
  # Add source files of C++ client to contain it inside.
  tdcl_add_source_files("${TRANSACTD_ROOT}")
endif()
set(${this_target}_SOURCE_FILES
  ${${this_target}_SOURCE_FILES}
  ${TRANSACTD_ROOT}/build/swig/ruby/tdclrb_wrap.cpp
  ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/connectionPool.cpp
  ${TRANSACTD_ROOT}/source/bzs/rtl/benchmark.cpp
)


# ==========================================================
# add include path
# ==========================================================
tdcl_add_include_directory("${TRANSACTD_ROOT}")
include_directories(${TRANSACTD_ROOT};${TRANSACTD_RUBY_INCLUDE_PATH})


# ==========================================================
# read and add resource file
# ==========================================================
set(${this_target}_RC_FILE "${TRANSACTD_ROOT}/build/tdclrb/tdclrb.rc")
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
if(WIN32)
  if(MSVC)
    add_dependencies(${this_target} tdclcpp)
  endif()
else()
  add_dependencies(${this_target} tdclc tdclcpp)
endif()

# ==========================================================
# Add compile options
# ==========================================================
transactd_set_MTMD("MD")


# ==========================================================
# Add compile definitions
# ==========================================================
set_property(TARGET ${this_target} PROPERTY COMPILE_DEFINITIONS
  SWIGRUBY SWIG_BUILDING USE_DBM_CONNECTION_POOL BOOST_ALL_NO_LIB _MBCS)
if(TRANSACTD_HAVE_RB_THREAD_BLOCKING_REGION)
  set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
    HAVE_RB_THREAD_BLOCKING_REGION)
endif()
if(TRANSACTD_HAVE_RB_THREAD_CALL_WITHOUT_GVL)
  set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
    HAVE_RB_THREAD_CALL_WITHOUT_GVL)
endif()
if(MSVC)
  set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
    WIN32 _WIN32 _WINDOWS _USRDLL
    TRDCL_AUTOLINK TRDCLENGN_EXPORTS _CRT_SECURE_NO_WARNINGS)
else()
  set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
    PIC _REENTRANT)
  if(NOT MINGW)
    set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
      LINUX)
  else()
    set_property(TARGET ${this_target} APPEND PROPERTY COMPILE_DEFINITIONS
      WIN32 _WIN32 _SYS_TIME_H_ _GETTIMEOFDAY_DEFINED)
  endif()
endif()


# ==========================================================
# add link / include
# ==========================================================
target_link_libraries(${this_target} "${TRANSACTD_RUBY_LIBRARY_PATH}")
transactd_link_boost_libraries("timer;filesystem;thread;chrono;system")
#tdclcpp_x on windows is auto link
if(NOT WIN32)
  target_link_libraries(${this_target} "tdclcpp_64m")
endif()


# ==========================================================
# set binary name
# ==========================================================
set_target_properties(${this_target} PROPERTIES PREFIX "")
set_target_properties(${this_target} PROPERTIES OUTPUT_NAME transactd)
if(APPLE)
  set_target_properties(${this_target} PROPERTIES SUFFIX ".bundle")
else()
  set_target_properties(${this_target} PROPERTIES SUFFIX ".so")
endif()
if(UNIX)
  set_target_properties(${this_target} PROPERTIES
    SOVERSION ${TD_RC_VER_MAJOR}.${TD_RC_VER_MINOR}
    VERSION ${TD_RC_VER_MAJOR}.${TD_RC_VER_MINOR}.${TD_RC_VER_RELEASE})
endif()


# ==========================================================
# find .dll to install (prebuilt tdclc_*.dll and MinGW runtime)
# ==========================================================
# find tdclc_*.dll
if(WIN32)
  bz_get_compiler_bitness()
  file(GLOB tmp_paths "${TRANSACTD_RUBY_GEM_ROOT_PATH}/bin/common/tdclc_${BZ_BITNESS}*.dll")
  if("${tmp_paths}" STREQUAL "")
    set(TDCL_CLIB_PATH "")
  else()
    list(GET tmp_paths 0 TDCL_CLIB_PATH)
  endif()
  file(TO_CMAKE_PATH "${TDCL_CLIB_PATH}" TDCL_CLIB_PATH)
  message(STATUS "tdclc_${BZ_BITNESS}*.dll   : ${TDCL_CLIB_PATH}")
else()
  set(TDCL_CLIB_PATH "")
endif()
# find MinGW runtime
if(MINGW AND WIN32)
  get_filename_component(MINGW_BIN_PATH ${CMAKE_CXX_COMPILER} PATH)
  # find libgcc_*.dll
  file(GLOB tmp_paths "${MINGW_BIN_PATH}/libgcc_*.dll")
  if("${tmp_paths}" STREQUAL "")
    set(MINGW_LIBGCC_PATH "")
  else()
    list(GET tmp_paths 0 MINGW_LIBGCC_PATH)
  endif()
  # find libstdc++-*.dll
  file(GLOB tmp_paths "${MINGW_BIN_PATH}/libstdc++-*.dll")
  if("${tmp_paths}" STREQUAL "")
    set(MINGW_LIBSTDCPP_PATH "")
  else()
    list(GET tmp_paths 0 MINGW_LIBSTDCPP_PATH)
  endif()
  message(STATUS "MinGW bin path  : ${MINGW_BIN_PATH}")
  message(STATUS "libgcc_*.dll    : ${MINGW_LIBGCC_PATH}")
  message(STATUS "libstdc++-*.dll : ${MINGW_LIBSTDCPP_PATH}")
else()
  set(MINGW_LIBGCC_PATH "")
  set(MINGW_LIBSTDCPP_PATH "")
endif()


# ==========================================================
# set output directory
# ==========================================================
## install rubyclient .so to transactd-gem dir
install(TARGETS ${this_target}
  LIBRARY DESTINATION "${TRANSACTD_RUBY_GEM_ROOT_PATH}/bin/${TDCL_GEM_PREFIX}"
  RUNTIME DESTINATION "${TRANSACTD_RUBY_GEM_ROOT_PATH}/bin/${TDCL_GEM_PREFIX}"
  ARCHIVE DESTINATION "${TRANSACTD_RUBY_GEM_ROOT_PATH}/bin/${TDCL_GEM_PREFIX}"
)
## install *.rb files
file(GLOB rb_files "${TRANSACTD_ROOT}/build/tdclrb/gem/*.rb")
install(FILES ${rb_files} DESTINATION "${TRANSACTD_RUBY_GEM_ROOT_PATH}/lib/")
file(GLOB rb_test_files "${TRANSACTD_ROOT}/source/bzs/test/tdclrb/*.rb")
install(FILES ${rb_test_files} DESTINATION "${TRANSACTD_RUBY_GEM_ROOT_PATH}/test/")
## install runtime and prebuilt tdclc_*.dll on win32
if(WIN32)
  if(MINGW)
    # libgcc*.dll and libstdc++*.dll -> ruby\bin
    get_filename_component(RUBY_BIN_PATH "${TRANSACTD_RUBY_EXECUTABLE_PATH}" PATH)
    bz_smart_install(DEST "${RUBY_BIN_PATH}"
      SOURCES "${MINGW_LIBGCC_PATH}" "${MINGW_LIBSTDCPP_PATH}")
  endif()
  # tdclc_*.dll -> system dir
  bz_smart_install(SOURCES "${TDCL_CLIB_PATH}" TO_WIN_SYSTEMDIR)
endif()
