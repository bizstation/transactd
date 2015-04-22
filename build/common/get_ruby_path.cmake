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
#   get ruby path
#     message(STATUS "Found ${RUBY_EXECITABLE_PATH}")
#     message(STATUS "  RUBY_VERSION          : ${RUBY_VERSION}")
#     message(STATUS "  RUBY_VERSION_STRING   : ${RUBY_VERSION_STRING}")
#     message(STATUS "  RUBY_INCLUDE_DIRS     : ${RUBY_INCLUDE_DIRS}")
#     message(STATUS "  RUBY_LIB_PATH         : ${RUBY_LIB_PATH}")
#     message(STATUS "  RUBY_STATIC_LIB_PATH  : ${RUBY_STATIC_LIB_PATH}")
#     message(STATUS "  RUBY_VERSION_FOR_GEM  : ${RUBY_VERSION_FOR_GEM}")
if(NOT COMMAND get_ruby_path)
macro(get_ruby_path RUBY_EXE_PATH)
  if("${RUBY_EXE_PATH}" STREQUAL "")
    set(tmp_ruby "ruby")
  else()
    set(tmp_ruby "${RUBY_EXE_PATH}")
  endif()
  # find ruby executable
  execute_process(COMMAND ${tmp_ruby} -v OUTPUT_VARIABLE RUBY_VERSION_STRING)
  string(STRIP "${RUBY_VERSION_STRING}" RUBY_VERSION_STRING)
  set(RUBY_EXECITABLE_PATH "${tmp_ruby}")
  if("${RUBY_VERSION_STRING}" STREQUAL "")
    set(RUBY_EXECITABLE_PATH "")
  endif()
  # RUBY_VERSION
  execute_process(COMMAND ${tmp_ruby} -e "puts RUBY_VERSION"
                  OUTPUT_VARIABLE RUBY_VERSION)
  string(STRIP "${RUBY_VERSION}" RUBY_VERSION)
  # RUBY_VERSION_GEM
  execute_process(COMMAND ${tmp_ruby} -e "puts RUBY_VERSION.sub(/\\.\\d+$/, '')"
                  OUTPUT_VARIABLE RUBY_VERSION_FOR_GEM)
  string(STRIP "${RUBY_VERSION_FOR_GEM}" RUBY_VERSION_FOR_GEM)
  # excec ruby command (rbconfig)
  execute_process(COMMAND ${tmp_ruby} -r rbconfig -e 
    "keys = ['LIBRUBY', 'LIBRUBY_A', 'rubyarchhdrdir', 'rubyhdrdir', 'libdir', 'archlibdir', 'arch', 'bindir', 'sbindir', 'ruby_install_name'];RbConfig::CONFIG.each{|key, value| puts ':' + key + '=' + value if keys.include?(key)}"
    OUTPUT_VARIABLE RBCONFIG_RETURN_VALUES)
    #message(STATUS "${RBCONFIG_RETURN_VALUES}")
  # parse result
  foreach(keyword LIBRUBY LIBRUBY_A rubyarchhdrdir rubyhdrdir libdir archlibdir arch bindir sbindir ruby_install_name)
    set(RBCONFIG_${keyword} "")
    string(REGEX MATCH ":${keyword}=[^\n]*" tmp_str "${RBCONFIG_RETURN_VALUES}")
    string(STRIP "${tmp_str}" tmp_str)
    string(REPLACE ":${keyword}=" "" tmp_str "${tmp_str}")
    set(RBCONFIG_${keyword} "${tmp_str}")
  endforeach()
  # overwrite executable path to fullpath
  if(NOT "${RBCONFIG_bindir}" STREQUAL "")
    if("${RBCONFIG_ruby_install_name}" STREQUAL "")
      set(RUBY_EXECITABLE_PATH "${RBCONFIG_bindir}/ruby")
    else()
      set(RUBY_EXECITABLE_PATH "${RBCONFIG_bindir}/${RBCONFIG_ruby_install_name}")
    endif()
    file(TO_CMAKE_PATH "${RUBY_EXECITABLE_PATH}" RUBY_EXECITABLE_PATH)
  endif()
  # find library path
  find_path(RUBY_LIB_PATH NAMES ${RBCONFIG_LIBRUBY}
    HINTS ${RBCONFIG_libdir} ${RBCONFIG_archlibdir}
          ${RBCONFIG_bindir} ${RBCONFIG_sbindir} NO_DEFAULT_PATH)
  if("${RUBY_LIB_PATH}" STREQUAL "RUBY_LIB_PATH-NOTFOUND")
    set(RUBY_LIB_PATH "")
  else()
    set(RUBY_LIB_PATH "${RUBY_LIB_PATH}/${RBCONFIG_LIBRUBY}")
    file(TO_NATIVE_PATH "${RUBY_LIB_PATH}" RUBY_LIB_PATH)
  endif()
  # find static library path
  find_path(RUBY_STATIC_LIB_PATH NAMES ${RBCONFIG_LIBRUBY_A}
    HINTS ${RBCONFIG_libdir} ${RBCONFIG_archlibdir}
          ${RBCONFIG_bindir} ${RBCONFIG_sbindir} NO_DEFAULT_PATH)
  if("${RUBY_STATIC_LIB_PATH}" STREQUAL "RUBY_STATIC_LIB_PATH-NOTFOUND")
    set(RUBY_STATIC_LIB_PATH "")
  else()
    set(RUBY_STATIC_LIB_PATH "${RUBY_STATIC_LIB_PATH}/${RBCONFIG_LIBRUBY_A}")
    file(TO_NATIVE_PATH "${RUBY_STATIC_LIB_PATH}" RUBY_STATIC_LIB_PATH)
  endif()
  # find include path (ruby.h)
  find_path(RUBY_INCLUDE_DIR_1 NAMES ruby.h
    HINTS ${RBCONFIG_rubyhdrdir} ${RBCONFIG_rubyarchhdrdir} NO_DEFAULT_PATH)
  if("${RUBY_INCLUDE_DIR_1}" STREQUAL "RUBY_INCLUDE_DIR_1-NOTFOUND")
    set(RUBY_INCLUDE_DIR_1 "")
  endif()
  # find include path (ruby/config.h)
  set(tmp_str "${RBCONFIG_rubyhdrdir}/${RBCONFIG_arch}")
  file(TO_NATIVE_PATH "${tmp_str}" tmp_str)
  find_path(RUBY_INCLUDE_DIR_2 NAMES ruby/config.h
    HINTS "${RBCONFIG_rubyhdrdir}" "${RBCONFIG_rubyarchhdrdir}" "${tmp_str}" NO_DEFAULT_PATH)
  if("${RUBY_INCLUDE_DIR_2}" STREQUAL "RUBY_INCLUDE_DIR_2-NOTFOUND")
    set(RUBY_INCLUDE_DIR_2 "")
  endif()
  set(RUBY_INCLUDE_DIRS "${RUBY_INCLUDE_DIR_1};${RUBY_INCLUDE_DIR_2}")
  # print
  message(STATUS "Found ${RUBY_EXECITABLE_PATH}")
  message(STATUS "  RUBY_VERSION          : ${RUBY_VERSION}")
  message(STATUS "  RUBY_VERSION_STRING   : ${RUBY_VERSION_STRING}")
  message(STATUS "  RUBY_INCLUDE_DIRS     : ${RUBY_INCLUDE_DIRS}")
  message(STATUS "  RUBY_LIB_PATH         : ${RUBY_LIB_PATH}")
  message(STATUS "  RUBY_STATIC_LIB_PATH  : ${RUBY_STATIC_LIB_PATH}")
  message(STATUS "  RUBY_VERSION_FOR_GEM  : ${RUBY_VERSION_FOR_GEM}")
endmacro()
endif()
