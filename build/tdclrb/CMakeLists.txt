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
include(./gem_output.cmake)


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
message(STATUS "  TRANSACTD_RUBY_INCLUDE_PATH ${TRANSACTD_RUBY_INCLUDE_PATH}")
message(STATUS "  TRANSACTD_RUBY_LIBRARY_PATH ${TRANSACTD_RUBY_LIBRARY_PATH}")
set(TDCL_GEM_PREFIX "${RUBY_VERSION_FOR_GEM}")
if("${TRANSACTD_RUBY_GEM_ROOT_PATH}" STREQUAL "")
  set(TRANSACTD_RUBY_GEM_ROOT_PATH "${TRANSACTD_BINARY_ROOT}")
endif()


# ==========================================================
# version info
# ==========================================================
transactd_read_version("${TRANSACTD_ROOT}")
transactd_read_build_number("${TRANSACTD_ROOT}")
set(TDVER_RUBY_BUILD "${TRANSACTD_BUILD_NUMBER}")


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
set(RUBY_SWIG_WRAPPER_CPP "${CMAKE_CURRENT_BINARY_DIR}/tdclrb_wrap.cpp")
file(TO_NATIVE_PATH "${TRANSACTD_ROOT}/build/swig/tdcl.i" SWIG_INTERFACE_FILE)
file(TO_NATIVE_PATH "${RUBY_SWIG_WRAPPER_CPP}"            SWIG_NATIVE_OUTFILE)
file(TO_NATIVE_PATH "${TRANSACTD_ROOT}"                   SWIG_INC_PATH1)
file(TO_NATIVE_PATH "${TRANSACTD_ROOT}/source"            SWIG_INC_PATH2)
# configure generate command file
if(WIN32)
  set(SWIG_GEN_CMD_WIN "${CMAKE_CURRENT_BINARY_DIR}/generate.cmd")
  configure_file("${TRANSACTD_ROOT}/build/swig/ruby/generate.cmd.in" "${SWIG_GEN_CMD_WIN}" @ONLY)
endif()
set(SWIG_GEN_CMD "${CMAKE_CURRENT_BINARY_DIR}/generate.cmake")
configure_file("${TRANSACTD_ROOT}/build/swig/ruby/generate.cmake.in" "${SWIG_GEN_CMD}" @ONLY)
# add generate command to build time
add_custom_command (
  OUTPUT  "${RUBY_SWIG_WRAPPER_CPP}"
  COMMAND ${CMAKE_COMMAND} -P ${SWIG_GEN_CMD}
  COMMENT "Generating tdclrb_wrap.cpp"
  DEPENDS "${TRANSACTD_ROOT}/build/swig/tdcl.i"
          "${TRANSACTD_ROOT}/build/swig/ruby/ruby.swg")


# ==========================================================
# add swig wrapper source file
# ==========================================================
set(${this_target}_SOURCE_FILES
  ${${this_target}_SOURCE_FILES}
  ${RUBY_SWIG_WRAPPER_CPP}
  ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/connectionPool.cpp
  ${TRANSACTD_ROOT}/source/bzs/rtl/benchmark.cpp
)


# ==========================================================
# add include path
# ==========================================================
tdcl_add_include_directory("${TRANSACTD_ROOT}")
include_directories(${TRANSACTD_ROOT};${TRANSACTD_RUBY_INCLUDE_PATH})


# ==========================================================
# add compiler / linker options
# ==========================================================
tdcl_add_compiler_options()
foreach(build_type "" "_RELEASE" "_DEBUG" "_RELWITHDEBINFO" "_MINSIZEREL")
  bz_add_cxx_flag("-DSWIGRUBY -DSWIG_BUILDING --std=c++0x -U__STRICT_ANSI__ -DUSE_DBM_CONNECTION_POOL" "${build_type}")
  if(MSVC)
    bz_set_MBCS_cxx_flag("${build_type}")
  endif()
endforeach()


# ==========================================================
# add resource file (for Windows Visual Studio)
# ==========================================================
if(WIN32)
  transactd_ver_info_lic()
  set(${this_target}_RC_FILE "${CMAKE_CURRENT_BINARY_DIR}/${this_target}.rc")
  transactd_generate_rc_file(
    TRANSACTD_ROOT "${TRANSACTD_ROOT}"
    OUTFILE "${${this_target}_RC_FILE}"
    F_VER   "${TDVER_CPP_INTERFACE_VER_MAJOR}.${TDVER_CPP_INTERFACE_VER_MINOR}.${TDVER_CPP_INTERFACE_VER_RELEASE}.${TDVER_RUBY_BUILD}"
    P_VER   "${TDVER_CPP_INTERFACE_VER_MAJOR}.${TDVER_CPP_INTERFACE_VER_MINOR}.${TDVER_CPP_INTERFACE_VER_RELEASE}.${TDVER_RUBY_BUILD}"
    P_NAME  "Transactd Ruby Client ${TRANSACTD_VER_POSTFIX}"
    F_DESC  "Transactd Ruby Client module ${TRANSACTD_VER_POSTFIX}"
  )
  transactd_add_rc_file()
endif()


# ==========================================================
# add_library
# ==========================================================
if(APPLE)
  add_library(${this_target} MODULE ${${this_target}_SOURCE_FILES})
  set_target_properties(${this_target} PROPERTIES LINK_FLAGS "-flat_namespace")
else()
  add_library(${this_target} SHARED ${${this_target}_SOURCE_FILES})
endif()

# ==========================================================
# add link / include
# ==========================================================
if(MINGW)
  transactd_link_boost_libraries("timer;filesystem;thread;chrono;system")
else()
  transactd_link_boost_libraries("timer")
endif()
target_link_libraries(${this_target} "tdclcpp${TDCL_CPP_CHARSET}")
target_link_libraries(${this_target} ${TRANSACTD_RUBY_LIBRARY_PATH})


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
if(UNIX AND NOT APPLE)
  set_target_properties(${this_target} PROPERTIES
    SOVERSION ${TDVER_CPP_INTERFACE_VER_MAJOR}.${TDVER_CPP_INTERFACE_VER_MINOR}
    VERSION ${TDVER_CPP_INTERFACE_VER_MAJOR}.${TDVER_CPP_INTERFACE_VER_MINOR}.${TDVER_CPP_INTERFACE_VER_RELEASE})
endif()


# ==========================================================
# find .dll if MINGW
# ==========================================================
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
  # find tdclc dll
  bz_get_compiler_bitness()
  file(GLOB tmp_paths "${TRANSACTD_RUBY_GEM_ROOT_PATH}/bin/common/tdclc_${BZ_BITNESS}*.dll")
  if("${tmp_paths}" STREQUAL "")
    set(TDCL_CLIB_PATH "")
  else()
    list(GET tmp_paths 0 TDCL_CLIB_PATH)
  endif()
  message(STATUS "MinGW bin path  : ${MINGW_BIN_PATH}")
  message(STATUS "libgcc_*.dll    : ${MINGW_LIBGCC_PATH}")
  message(STATUS "libstdc++-*.dll : ${MINGW_LIBSTDCPP_PATH}")
  message(STATUS "tdclc_${BZ_BITNESS}*.dll   : ${TDCL_CLIB_PATH}")
else()
  set(MINGW_LIBGCC_PATH "")
  set(MINGW_LIBSTDCPP_PATH "")
  set(TDCL_CLIB_PATH "")
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
## install runtime and prebuilt tdclc_*.dll if mingw on win32
if(MINGW AND WIN32)
  # libgcc*.dll and libstdc++*.dll -> ruby\bin
  get_filename_component(RUBY_BIN_PATH "${TRANSACTD_RUBY_EXECUTABLE_PATH}" PATH)
  bz_smart_install(DEST "${RUBY_BIN_PATH}"
    SOURCES "${MINGW_LIBGCC_PATH}" "${MINGW_LIBSTDCPP_PATH}")
  # tdclc_*.dll -> system dir
  bz_smart_install(TO_WIN_SYSTEMDIR SOURCES "${TDCL_CLIB_PATH}")
endif()
