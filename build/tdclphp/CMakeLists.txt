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
include(../common/check_for_link_iconv.cmake)
check_for_link_iconv()


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
transactd_read_version("${TRANSACTD_ROOT}")
transactd_read_build_number("${TRANSACTD_ROOT}")
set(TDVER_PHP_BUILD "${TRANSACTD_BUILD_NUMBER}")


# ==========================================================
# add tdclc/tdclcppm if need
# ==========================================================
if((NOT TARGET tdclc) AND (NOT MINGW))
  subdirs("../tdclc")
endif()
if(NOT TARGET tdclcppm)
  set(TDCL_CPP_CHARSET "m")
  subdirs("../tdclcppm")
endif()


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
if(WIN32)
  transactd_ver_info_lic()
  set(${this_target}_RC_FILE "${CMAKE_CURRENT_BINARY_DIR}/${this_target}.rc")
  transactd_generate_rc_file(
    TRANSACTD_ROOT "${TRANSACTD_ROOT}"
    OUTFILE "${${this_target}_RC_FILE}"
    F_VER   "${TDVER_CPP_INTERFACE_VER_MAJOR}.${TDVER_CPP_INTERFACE_VER_MINOR}.${TDVER_CPP_INTERFACE_VER_RELEASE}.${TDVER_PHP_BUILD}"
    P_VER   "${TDVER_CPP_INTERFACE_VER_MAJOR}.${TDVER_CPP_INTERFACE_VER_MINOR}.${TDVER_CPP_INTERFACE_VER_RELEASE}.${TDVER_PHP_BUILD}"
    P_NAME  "Transactd PHP Client ${TRANSACTD_VER_POSTFIX}"
    F_DESC  "Transactd PHP Client module ${TRANSACTD_VER_POSTFIX}"
  )
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
transactd_link_boost_libraries("timer;filesystem;thread;chrono;system")
if(CAN_LINK_ICONV)
  target_link_libraries(${this_target} "${ICONV_LIBRARY}")
endif()
target_link_libraries(${this_target} "tdclcpp${TDCL_CPP_CHARSET}")
target_link_libraries(${this_target} "${TRANSACTD_PHP_LIBRARY_PATH}")
if(NOT WIN32)
  target_link_libraries(${this_target} dl)
endif()


# ==========================================================
# set binary name
# ==========================================================
set_target_properties(${this_target} PROPERTIES PREFIX "")
set_target_properties(${this_target} PROPERTIES OUTPUT_NAME php_transactd)
if(UNIX AND NOT APPLE)
  set_target_properties(${this_target} PROPERTIES
    SOVERSION ${TDVER_CPP_INTERFACE_VER_MAJOR}.${TDVER_CPP_INTERFACE_VER_MINOR}
    VERSION ${TDVER_CPP_INTERFACE_VER_MAJOR}.${TDVER_CPP_INTERFACE_VER_MINOR}.${TDVER_CPP_INTERFACE_VER_RELEASE})
endif()
