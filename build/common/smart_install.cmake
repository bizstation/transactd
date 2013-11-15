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
## need system.cmake
# ==========================================================
#   install so/dll if greater
#     * compare .dll with binary version info
#     * compere .so with file name suffix
# ==========================================================
if(NOT COMMAND bz_smart_install)
set(BZ_SI_SCRIPT "${CMAKE_CURRENT_LIST_FILE}")
get_filename_component(BZ_SI_WINCMD "${CMAKE_CURRENT_LIST_FILE}" PATH)
set(BZ_SI_WINCMD "${BZ_SI_WINCMD}/copyifgreater.cmd")
macro(bz_smart_install)
  cmake_parse_arguments(BZ_SI
    "TO_WIN_SYSTEMDIR;ADD_ME" "DEST;VER_MAJOR;VER_MINOR;VER_RELEASE" "SOURCES" ${ARGN})
  bz_smart_install_set_arguments(
    SOURCES "${BZ_SI_SOURCES}"
    DEST    "${BZ_SI_DEST}"
    ADD_ME  "${BZ_SI_ADD_ME}"
    TO_WIN_SYSTEMDIR "${BZ_SI_TO_WIN_SYSTEMDIR}"
  )
  if(WIN32)
    install(CODE "
  include (CMakeParseArguments)
  if(NOT COMMAND bz_do_smart_install_dll)
    include(\"${BZ_SI_SCRIPT}\")
  endif()
  bz_do_smart_install_dll(
    CMD     \"${BZ_SI_WINCMD}\"
    DEST    \"${BZ_SI_DEST}\"
    SOURCES \"${BZ_SI_SOURCES}\"
  )
    ")
  else()
    install(CODE "
  include (CMakeParseArguments)
  if(NOT COMMAND bz_smart_install_so)
    include(\"${BZ_SI_SCRIPT}\")
  endif()
  bz_smart_install_so(
    DEST      \"${BZ_SI_DEST}\"
    SOURCE    \"${BZ_SI_SOURCES}\"
    VMAJOR    \"${BZ_SI_VER_MAJOR}\"
    VMINOR    \"${BZ_SI_VER_MINOR}\"
    VRELEASE  \"${BZ_SI_VER_RELEASE}\"
  )
    ")
  endif()
endmacro()
endif()

# ==========================================================
#   set arguments for smart_install
# ==========================================================
if(NOT COMMAND bz_smart_install_set_arguments)
macro(bz_smart_install_set_arguments)
  cmake_parse_arguments(BZ_SISA "" "TO_WIN_SYSTEMDIR;ADD_ME;DEST" "SOURCES" ${ARGN})
  # set dest to sysdir if Windows and TO_WIN_SYSTEMDIR flag on
  if(WIN32 AND (${BZ_SISA_TO_WIN_SYSTEMDIR} STREQUAL "TRUE"))
    bz_get_win_sysdir()
    set(BZ_SI_DEST "${BZ_WIN_SYSDIR}")
  endif()
  # set this target file to sources if ADD_ME flag on
  if("${BZ_SISA_ADD_ME}" STREQUAL "TRUE")
    get_target_property(BZ_SISA_tmp_var "${this_target}" LOCATION)
    set(BZ_SI_SOURCES "${BZ_SISA_SOURCES}" "${BZ_SISA_tmp_var}")
    # get .so version info
    if(NOT WIN32)
      get_target_property(BZ_SISA_tmp_var "${this_target}" VERSION)
      string(REPLACE "." ";" BZ_SISA_tmp_list "${BZ_SISA_tmp_var}")
      list(GET BZ_SISA_tmp_list 0 BZ_SI_VER_MAJOR)
      list(GET BZ_SISA_tmp_list 1 BZ_SI_VER_MINOR)
      list(GET BZ_SISA_tmp_list 2 BZ_SI_VER_RELEASE)
      math(EXPR BZ_SI_VER_MAJOR   "${BZ_SI_VER_MAJOR}")
      math(EXPR BZ_SI_VER_MINOR   "${BZ_SI_VER_MINOR}")
      math(EXPR BZ_SI_VER_RELEASE "${BZ_SI_VER_RELEASE}")
    endif()
  endif()
endmacro()
endif()

# ==========================================================
#   install dll if greater (compare with binary version)
# ==========================================================
if(NOT COMMAND bz_do_smart_install_dll)
macro(bz_do_smart_install_dll)
  cmake_parse_arguments(BZ_SID "" "CMD;DEST" "SOURCES" ${ARGN})
  execute_process(COMMAND
    "${BZ_SID_CMD}" "${BZ_SID_SOURCES}" "${BZ_SID_DEST}"
    RESULT_VARIABLE tmp_ret
    OUTPUT_VARIABLE tmp_var
    ERROR_VARIABLE  tmp_err
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
  )
  string(STRIP "${tmp_var}" tmp_var)
  string(REGEX REPLACE "\n" ";" tmp_list "${tmp_var}")
  foreach(tmp_line ${tmp_list})
    message(STATUS "${tmp_line}")
    foreach(tmp_key "Installing-if-grater: " "Up-to-date-if-grater: ")
      string(REGEX MATCH "${tmp_key}(.*)" tmp_filename "${tmp_line}")
      string(STRIP "${tmp_filename}" tmp_filename)
      string(REPLACE "${tmp_key}" "" tmp_filename "${tmp_filename}")
      if(NOT "${tmp_filename}" STREQUAL "")
        file(TO_CMAKE_PATH "${tmp_filename}" tmp_filename)
        set(CMAKE_INSTALL_MANIFEST_FILES "${CMAKE_INSTALL_MANIFEST_FILES}" "${tmp_filename}")
      endif()
    endforeach()
  endforeach()
  string(STRIP "${tmp_ret}" tmp_ret)
  if(NOT "${tmp_err}" STREQUAL "")
    if("${tmp_ret}" STREQUAL "1")
      message(SEND_ERROR "
***********************************************************************
(EROOR MESSAGE)
Failed to Install.
Perhaps it require to run as Administrator, or file is in use.
Return Code: ${tmp_ret}
${tmp_err}
***********************************************************************
")
    else()
      message(WARNING "
***********************************************************************
(WARNING MESSAGE)
Files not copied because it could not read the version from the binary.
There is a possibility that the program does not work properly.
Please check the compatibility of following binaries.
Return Code: ${tmp_ret}
${tmp_err}
***********************************************************************
")
    endif()
  endif()
endmacro()
endif()

# ==========================================================
#   install so if greater (compare with file name)
# ==========================================================
if(NOT COMMAND bz_smart_install_so)
macro(bz_smart_install_so)
  cmake_parse_arguments(BZ_DSI "" "DEST;SOURCE;VMAJOR;VMINOR;VRELEASE" "" ${ARGN})
  get_filename_component(BZ_DSI_BASE "${BZ_DSI_SOURCE}" NAME)
  file(TO_CMAKE_PATH "${BZ_DSI_DEST}/${BZ_DSI_BASE}" BZ_DSI_DEST_BASE)
  #
  # copy file like libfoo.so.1.2.3
  #
  if(NOT EXISTS "${BZ_DSI_DEST}")
    file(MAKE_DIRECTORY "${BZ_DSI_DEST}")
  endif()
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy
    "${BZ_DSI_SOURCE}.${BZ_DSI_VMAJOR}.${BZ_DSI_VMINOR}.${BZ_DSI_VRELEASE}" "${BZ_DSI_DEST}")
  message(STATUS "Installing: ${BZ_DSI_DEST_BASE}.${BZ_DSI_VMAJOR}.${BZ_DSI_VMINOR}.${BZ_DSI_VRELEASE}")
  set(CMAKE_INSTALL_MANIFEST_FILES "${CMAKE_INSTALL_MANIFEST_FILES}"
    "${BZ_DSI_DEST_BASE}.${BZ_DSI_VMAJOR}.${BZ_DSI_VMINOR}.${BZ_DSI_VRELEASE}")
  #
  # create libfoo.so.1.2 symlink
  #
  set(BZ_DSI_INSTALLED_LINK_1 OFF)
  if(NOT EXISTS "${BZ_DSI_DEST_BASE}.${BZ_DSI_VMAJOR}.${BZ_DSI_VMINOR}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E remove
      "${BZ_DSI_DEST_BASE}.${BZ_DSI_VMAJOR}.${BZ_DSI_VMINOR}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
      "${BZ_DSI_BASE}.${BZ_DSI_VMAJOR}.${BZ_DSI_VMINOR}.${BZ_DSI_VRELEASE}"
      "${BZ_DSI_DEST_BASE}.${BZ_DSI_VMAJOR}.${BZ_DSI_VMINOR}")
    set(BZ_DSI_INSTALLED_LINK_1 ON)
  else()
    get_filename_component(BZ_DSI_REAL "${BZ_DSI_DEST_BASE}.${BZ_DSI_VMAJOR}.${BZ_DSI_VMINOR}" REALPATH)
    get_filename_component(BZ_DSI_REAL_NAME "${BZ_DSI_REAL}" NAME)
    if(NOT EXISTS "${BZ_DSI_REAL}")
      execute_process(COMMAND ${CMAKE_COMMAND} -E remove
        "${BZ_DSI_DEST_BASE}.${BZ_DSI_VMAJOR}.${BZ_DSI_VMINOR}")
      execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
        "${BZ_DSI_BASE}.${BZ_DSI_VMAJOR}.${BZ_DSI_VMINOR}.${BZ_DSI_VRELEASE}"
        "${BZ_DSI_DEST_BASE}.${BZ_DSI_VMAJOR}.${BZ_DSI_VMINOR}")
      set(BZ_DSI_INSTALLED_LINK_1 ON)
    else()
      if("${BZ_DSI_REAL_NAME}" MATCHES "^${BZ_DSI_BASE}\\.${BZ_DSI_VMAJOR}\\.${BZ_DSI_VMINOR}\\.[0-9]+$")
        string(REPLACE "${BZ_DSI_BASE}.${BZ_DSI_VMAJOR}.${BZ_DSI_VMINOR}." "" BZ_DSI_TMP_VER "${BZ_DSI_REAL_NAME}")
        math(EXPR BZ_DSI_TMP_VER "${BZ_DSI_TMP_VER}")
        if(${BZ_DSI_TMP_VER} LESS ${BZ_DSI_VRELEASE})
          execute_process(COMMAND ${CMAKE_COMMAND} -E remove
            "${BZ_DSI_DEST_BASE}.${BZ_DSI_VMAJOR}.${BZ_DSI_VMINOR}")
          execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
            "${BZ_DSI_BASE}.${BZ_DSI_VMAJOR}.${BZ_DSI_VMINOR}.${BZ_DSI_VRELEASE}"
            "${BZ_DSI_DEST_BASE}.${BZ_DSI_VMAJOR}.${BZ_DSI_VMINOR}")
          set(BZ_DSI_INSTALLED_LINK_1 ON)
        endif()
      endif()
    endif()
  endif()
  if(BZ_DSI_INSTALLED_LINK_1)
    set(CMAKE_INSTALL_MANIFEST_FILES "${CMAKE_INSTALL_MANIFEST_FILES}"
      "${BZ_DSI_DEST_BASE}.${BZ_DSI_VMAJOR}.${BZ_DSI_VMINOR}")
    message(STATUS "Installing: ${BZ_DSI_DEST_BASE}.${BZ_DSI_VMAJOR}.${BZ_DSI_VMINOR}")
  else()
    message(STATUS "Up-to-date: ${BZ_DSI_DEST_BASE}.${BZ_DSI_VMAJOR}.${BZ_DSI_VMINOR}")
  endif()
  #
  # create libfoo.so symlink
  #
  set(BZ_DSI_INSTALLED_LINK_2 OFF)
  if(NOT EXISTS "${BZ_DSI_DEST_BASE}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E remove "${BZ_DSI_DEST_BASE}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
      "${BZ_DSI_BASE}.${BZ_DSI_VMAJOR}.${BZ_DSI_VMINOR}" "${BZ_DSI_DEST_BASE}")
    set(BZ_DSI_INSTALLED_LINK_2 ON)
  else()
    get_filename_component(BZ_DSI_REAL "${BZ_DSI_DEST_BASE}" REALPATH)
    get_filename_component(BZ_DSI_REAL_NAME "${BZ_DSI_REAL}" NAME)
    if(NOT EXISTS "${BZ_DSI_REAL}")
      execute_process(COMMAND ${CMAKE_COMMAND} -E remove "${BZ_DSI_DEST_BASE}")
      execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
        "${BZ_DSI_BASE}.${BZ_DSI_VMAJOR}.${BZ_DSI_VMINOR}" "${BZ_DSI_DEST_BASE}")
      set(BZ_DSI_INSTALLED_LINK_2 ON)
    else()
      if("${BZ_DSI_REAL_NAME}" MATCHES "^${BZ_DSI_BASE}\\.[0-9]+\\.[0-9]+\\.[0-9]+$")
        string(REPLACE "${BZ_DSI_BASE}." "" BZ_DSI_TMP_VER "${BZ_DSI_REAL_NAME}")
        string(REPLACE "." ";" BZ_DSI_TMP_VER_LIST "${BZ_DSI_TMP_VER}")
        list(GET BZ_DSI_TMP_VER_LIST 0 BZ_DSI_TMP_VER_MAJOR)
        list(GET BZ_DSI_TMP_VER_LIST 1 BZ_DSI_TMP_VER_MINOR)
        math(EXPR BZ_DSI_TMP_VER_MAJOR "${BZ_DSI_TMP_VER_MAJOR}")
        math(EXPR BZ_DSI_TMP_VER_MINOR "${BZ_DSI_TMP_VER_MINOR}")
        if(${BZ_DSI_TMP_VER_MAJOR} LESS ${BZ_DSI_VMAJOR})
          execute_process(COMMAND ${CMAKE_COMMAND} -E remove "${BZ_DSI_DEST_BASE}")
          execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
            "${BZ_DSI_BASE}.${BZ_DSI_VMAJOR}.${BZ_DSI_VMINOR}" "${BZ_DSI_DEST_BASE}")
          set(BZ_DSI_INSTALLED_LINK_2 ON)
        elseif((${BZ_DSI_TMP_VER_MAJOR} EQUAL ${BZ_DSI_VMAJOR}) AND
               (${BZ_DSI_TMP_VER_MINOR} LESS ${BZ_DSI_VMINOR}))
          execute_process(COMMAND ${CMAKE_COMMAND} -E remove "${BZ_DSI_DEST_BASE}")
          execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
            "${BZ_DSI_BASE}.${BZ_DSI_VMAJOR}.${BZ_DSI_VMINOR}" "${BZ_DSI_DEST_BASE}")
          set(BZ_DSI_INSTALLED_LINK_2 ON)
        endif()
      endif()
    endif()
  endif()
  if(BZ_DSI_INSTALLED_LINK_2)
    set(CMAKE_INSTALL_MANIFEST_FILES "${CMAKE_INSTALL_MANIFEST_FILES}" "${BZ_DSI_DEST_BASE}")
    message(STATUS "Installing: ${BZ_DSI_DEST_BASE}")
  else()
    message(STATUS "Up-to-date: ${BZ_DSI_DEST_BASE}")
  endif()
endmacro()
endif()
