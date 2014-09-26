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
# ==========================================================
#   Version infomation macro
# ==========================================================
if(NOT COMMAND transactd_ver_info_lic)
macro(transactd_ver_info_lic)
  if(TRANSACTD_COMMERCIAL)
    set(TRANSACTD_VER_POSTFIX "(Commercial)")
  else()
    set(TRANSACTD_VER_POSTFIX "(GPL V2)")
  endif()
endmacro()
endif()


# ==========================================================
#   make subdir, copy CMakeLists.txt and BUILDNUMBER.txt
# ==========================================================
if(NOT COMMAND transactd_copy_subdir)
macro(transactd_copy_subdir TRANSACTD_ROOT srcname dstname)
  execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory
    ${TRANSACTD_ROOT}/build/${dstname})
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${TRANSACTD_ROOT}/build/${srcname}/CMakeLists.txt
    ${TRANSACTD_ROOT}/build/${dstname}/CMakeLists.txt)
  if(WIN32 AND NOT EXISTS "${TRANSACTD_ROOT}/build/${dstname}/BUILDNUMBER.txt")
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy
      ${TRANSACTD_ROOT}/build/${srcname}/BUILDNUMBER.txt
      ${TRANSACTD_ROOT}/build/${dstname}/BUILDNUMBER.txt)
  endif()
endmacro()
endif()


# ==========================================================
#   read build number if BUILDNUMBER.txt exists
# ==========================================================
if(NOT COMMAND transactd_read_build_number)
macro(transactd_read_build_number TRANSACTD_ROOT)
  if(WIN32)
    set(${this_target}_BuildNumber_path "${TRANSACTD_ROOT}/build/${this_target}/BUILDNUMBER.txt")
    if(EXISTS "${${this_target}_BuildNumber_path}")
      file(STRINGS "${${this_target}_BuildNumber_path}" TRANSACTD_BUILD_NUMBER_PRE)
      MATH(EXPR TRANSACTD_BUILD_NUMBER "${TRANSACTD_BUILD_NUMBER_PRE} + 1")
      file(WRITE "${${this_target}_BuildNumber_path}" "${TRANSACTD_BUILD_NUMBER}")
      message(STATUS "${this_target} increment build number : ${TRANSACTD_BUILD_NUMBER_PRE} -> ${TRANSACTD_BUILD_NUMBER}")
    else()
      set(TRANSACTD_BUILD_NUMBER 1)
      message(STATUS "${this_target} BUILDNUMBER.txt not found so BuildNumber = ${TRANSACTD_BUILD_NUMBER}")
    endif()
  else()
    set(TRANSACTD_BUILD_NUMBER 1)
  endif()
endmacro()
endif()


# ==========================================================
#   read version
# ==========================================================
if(NOT COMMAND transactd_read_version)
macro(transactd_read_version TRANSACTD_ROOT)
  if(NOT TRANSACTD_VERSION_READ)
    if(WIN32 AND NOT MINGW)
      set(TRANSACTD_VER_CMD_GREP "findstr")
    else()
      set(TRANSACTD_VER_CMD_GREP "grep")
    endif()
    # file that version defined
    set(TRANSACTD_VER_FILE_PATH "${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/tdapcapi.h")
    if(WIN32 AND NOT MINGW)
      file(TO_NATIVE_PATH "${TRANSACTD_VER_FILE_PATH}" TRANSACTD_VER_FILE_PATH)
    endif()
    if(NOT EXISTS "${TRANSACTD_VER_FILE_PATH}")
      message(SEND_ERROR "Can not find file ${TRANSACTD_VER_FILE_PATH}")
    endif()
    # temporary file
    set(TRANSACTD_VER_TMPFILE_PATH "${CMAKE_CURRENT_BINARY_DIR}/transactd_versions.tmp")
    if(NOT EXISTS "${TRANSACTD_VER_TMPFILE_PATH}")
      execute_process(
        COMMAND ${TRANSACTD_VER_CMD_GREP} "##" "${TRANSACTD_VER_FILE_PATH}"
        OUTPUT_FILE ${TRANSACTD_VER_TMPFILE_PATH}
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()
    # read from temporary file
    if(WIN32 AND NOT MINGW)
      file(TO_NATIVE_PATH "${TRANSACTD_VER_TMPFILE_PATH}" TRANSACTD_VER_TMPFILE_PATH)
    endif()
    foreach(VALNAME C_INTERFACE_VER_MAJOR   C_INTERFACE_VER_MINOR   C_INTERFACE_VER_RELEASE
                    CPP_INTERFACE_VER_MAJOR CPP_INTERFACE_VER_MINOR CPP_INTERFACE_VER_RELEASE
                    TRANSACTD_VER_MAJOR     TRANSACTD_VER_MINOR     TRANSACTD_VER_RELEASE)
      execute_process(
        COMMAND ${TRANSACTD_VER_CMD_GREP} "${VALNAME}" "${TRANSACTD_VER_TMPFILE_PATH}"
        OUTPUT_VARIABLE TRANSACTD_VER_TMP_VAR
        OUTPUT_STRIP_TRAILING_WHITESPACE)
      string(REGEX REPLACE "/\\*.*" "" TRANSACTD_VER_TMP_VAR "${TRANSACTD_VER_TMP_VAR}")
      string(REGEX REPLACE "//.*"   "" TRANSACTD_VER_TMP_VAR "${TRANSACTD_VER_TMP_VAR}")
      string(REGEX REPLACE "${VALNAME}" "" TRANSACTD_VER_TMP_VAR "${TRANSACTD_VER_TMP_VAR}")
      string(REGEX REPLACE "[^0-9]" "" TRANSACTD_VER_TMP_VAR "${TRANSACTD_VER_TMP_VAR}")
      if("${TRANSACTD_VER_TMP_VAR}" STREQUAL "")
        message(SEND_ERROR "Can not read version ${VALNAME}")
      endif()
      set(TDVER_${VALNAME}  "${TRANSACTD_VER_TMP_VAR}")
    endforeach()
    set(TRANSACTD_VERSION_READ ON)
  endif()
endmacro()
endif()


# ==========================================================
#   generate rc file
# ==========================================================
if(NOT COMMAND transactd_generate_rc_file)
macro(transactd_generate_rc_file)
if(WIN32)
  cmake_parse_arguments(TD_RC "" "TRANSACTD_ROOT;OUTFILE;F_VER;P_VER;P_NAME" "F_DESC" ${ARGN})
  # split version
  foreach(ForP F P)
    string(REPLACE "." ";" TD_RC_${ForP}_VER ${TD_RC_${ForP}_VER})
    list(GET TD_RC_${ForP}_VER 0 TD_RC_${ForP}_VER_MAJOR)
    list(GET TD_RC_${ForP}_VER 1 TD_RC_${ForP}_VER_MINOR)
    list(GET TD_RC_${ForP}_VER 2 TD_RC_${ForP}_VER_RELEASE)
    list(GET TD_RC_${ForP}_VER 3 TD_RC_${ForP}_VER_BUILD)
  endforeach()
  # generate rc file
  configure_file("${TD_RC_TRANSACTD_ROOT}/build/common/transactd.rc.in" "${TD_RC_OUTFILE}")
endif()
endmacro()
endif()


# ==========================================================
#   add rc file
# ==========================================================
if(NOT COMMAND transactd_add_rc_file)
macro(transactd_add_rc_file)
  if(MSVC)
    set(${this_target}_SOURCE_FILES
      "${${this_target}_SOURCE_FILES}" "${${this_target}_RC_FILE}")
  elseif(MINGW)
    set(CMAKE_RC_COMPILER_INIT windres)
    ENABLE_LANGUAGE(RC)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -o <OBJECT> <SOURCE>")
    set(${this_target}_SOURCE_FILES
      "${${this_target}_SOURCE_FILES}" "${${this_target}_RC_FILE}")
  endif()
endmacro()
endif()


# ==========================================================
#   transactd_set_MTMD
# ==========================================================
if(NOT COMMAND transactd_set_MTMD)
macro(transactd_set_MTMD MT_OR_MD)
  if(MSVC)
    string(TOUPPER "${MT_OR_MD}" MT_OR_MD)
    if( (NOT ("${MT_OR_MD}" STREQUAL "MT")) AND
        (NOT ("${MT_OR_MD}" STREQUAL "MD")))
      message(ERROR "[${MT_OR_MD}] is invalid. Please specify MT or MD.")
    endif()
    foreach(build_type "_RELEASE" "_DEBUG" "_RELWITHDEBINFO" "_MINSIZEREL")
      set(CMAKE_CXX_FLAGS${build_type}_BEFORE_${this_target} "${CMAKE_CXX_FLAGS${build_type}}")
      string(REGEX REPLACE "/MTd" " "
        CMAKE_CXX_FLAGS${build_type} "${CMAKE_CXX_FLAGS${build_type}}")
      string(REGEX REPLACE "/MDd" " "
        CMAKE_CXX_FLAGS${build_type} "${CMAKE_CXX_FLAGS${build_type}}")
      string(REGEX REPLACE "/MT" " "
        CMAKE_CXX_FLAGS${build_type} "${CMAKE_CXX_FLAGS${build_type}}")
      string(REGEX REPLACE "/MD" " "
        CMAKE_CXX_FLAGS${build_type} "${CMAKE_CXX_FLAGS${build_type}}")
      if("${CMAKE_CXX_FLAGS${build_type}}" MATCHES "(.* )?/D_DEBUG( .*)?$")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS${build_type}} /${MT_OR_MD}d")
      else()
        set(CMAKE_CXX_FLAGS${build_type} "${CMAKE_CXX_FLAGS${build_type}} /${MT_OR_MD}")
      endif()
    endforeach()
  endif()
endmacro()
endif()


# ==========================================================
#   transactd_reset_MTMD
# ==========================================================
if(NOT COMMAND transactd_reset_MTMD)
macro(transactd_reset_MTMD)
  if(MSVC)
    foreach(build_type "_RELEASE" "_DEBUG" "_RELWITHDEBINFO" "_MINSIZEREL")
      set(CMAKE_CXX_FLAGS${build_type} "${CMAKE_CXX_FLAGS${build_type}_BEFORE_${this_target}}")
    endforeach()
  endif()
endmacro()
endif()


# ==========================================================
#   transactd_has_MTMD_option
# ==========================================================
if(NOT COMMAND transactd_has_MTMD_option)
macro(transactd_has_MTMD_option option_string)
  set(transactd_has_MTMD_option_return OFF)
  if(MSVC)
    if( ("${option_string}" MATCHES "(.* )?/MT( .*)?$") OR
        ("${option_string}" MATCHES "(.* )?/MTd( .*)?$") OR
        ("${option_string}" MATCHES "(.* )?/MD( .*)?$") OR
        ("${option_string}" MATCHES "(.* )?/MDd( .*)?$") )
      set(transactd_has_MTMD_option_return ON)
    endif()
  endif()
endmacro()
endif()


# ==========================================================
#   transactd_link_boost_libraries MACRO
#     boost_components  : "system;chrono;thread;filesystem"
# ==========================================================
if(NOT COMMAND transactd_link_boost_libraries)
macro(transactd_link_boost_libraries boost_components)
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_LIBS ON)
  if("${BOOST_ROOT}" STREQUAL "")
    if(WIN32)
      bz_get_compiler_bitness()
      set(BOOST_ROOT "$ENV{TI_BOOST_ROOT_${BZ_BITNESS}}")
    endif()
    if("${BOOST_ROOT}" STREQUAL "")
      set(BOOST_ROOT "$ENV{BOOST_ROOT}")
    endif()
    if(NOT ("${BOOST_ROOT}" STREQUAL ""))
      message(STATUS "BOOST_ROOT : ${BOOST_ROOT}")
    endif()
  endif()
  ## use find_package to find Boost_LIBRARIES (on Unix or MinGW)
  if(UNIX OR MINGW)
    find_package(Boost COMPONENTS "${boost_components}")
    if(NOT Boost_FOUND)
      message(SEND_ERROR "Boost not found. ${BOOST_ROOT}")
    endif()
    if(MINGW)
      target_link_libraries(${this_target} ${Boost_LIBRARIES})
    else()
      if(APPLE)
        target_link_libraries(${this_target} ${Boost_LIBRARIES})
      else()
        # need "-lrt" after chrono or timer
        target_link_libraries(${this_target} ${Boost_LIBRARIES} rt)
      endif()
    endif()
  endif()
  ## find Boost for Visual Studio /MD or /MDd or /MT or /MTd
  if(MSVC)
    get_boost_libs("${boost_components}")
    if("${CMAKE_CONFIGURATION_TYPES}" STREQUAL "")
      set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo MinSizeRel)
    endif()
    # find boost libraries with project compile flag
    get_property(TMP_${this_target}_COMPILE_FLAGS
      TARGET ${this_target} PROPERTY COMPILE_FLAGS)
    transactd_has_MTMD_option("${TMP_${this_target}_COMPILE_FLAGS}")
    if("${transactd_has_MTMD_option_return}" STREQUAL "ON")
      get_boost_libs_from_compiler_flags("${TMP_${this_target}_COMPILE_FLAGS}")
      set(boost_libs_for_${this_target} "${get_boost_libs_from_compiler_flags_return}")
    else()
      set(boost_libs_for_${this_target} "")
    endif()
    set(boost_libs_listlen 0)
    # buildtype-specified libraries
    foreach(BT_NAME "" ${CMAKE_CONFIGURATION_TYPES})
      if(NOT("${BT_NAME}" STREQUAL ""))
        set(BT_NAME "_${BT_NAME}")
        string(TOUPPER "${BT_NAME}" BT_NAME)
      endif()
      transactd_has_MTMD_option("${CMAKE_CXX_FLAGS${BT_NAME}}")
      if("${transactd_has_MTMD_option_return}" STREQUAL "ON")
        # find buildtype-specified boost libraries
        get_boost_libs_from_compiler_flags("${CMAKE_CXX_FLAGS${BT_NAME}}")
        set(boost_libs_for${BT_NAME} "${get_boost_libs_from_compiler_flags_return}")
      else()
        # use project-specified libaries if buildtype-specified option is not set
        if("${boost_libs_for_${this_target}}" STREQUAL "")
          set(boost_libs_for${BT_NAME} "${Boost_LIBRARIES_STATIC_NOTUSE_RUNTIME_NO_DEBUG}")
        else()
          set(boost_libs_for${BT_NAME} "${boost_libs_for_${this_target}}")
        endif()
      endif()
      # check number of boost libaries
      string(REGEX REPLACE ";$" "" boost_libs_for${BT_NAME} "${boost_libs_for${BT_NAME}}")
      list(LENGTH boost_libs_for${BT_NAME} boost_libs_listlen${BT_NAME})
      if(${boost_libs_listlen${BT_NAME}} GREATER ${boost_libs_listlen})
        set(boost_libs_listlen ${boost_libs_listlen${BT_NAME}})
      endif()
    endforeach()
    # add libraries
    if(${boost_libs_listlen} GREATER 0)
      foreach(idx RANGE 1 ${boost_libs_listlen})
        math(EXPR i "${idx} - 1")
        add_library(boostlibs_for_${this_target}_${idx} SHARED IMPORTED)
        foreach(BT_NAME "" ${CMAKE_CONFIGURATION_TYPES})
          if(NOT("${BT_NAME}" STREQUAL ""))
            set(BT_NAME "_${BT_NAME}")
            string(TOUPPER "${BT_NAME}" BT_NAME)
          endif()
          if(NOT("${boost_libs_for${BT_NAME}}" STREQUAL ""))
            list(GET boost_libs_for${BT_NAME} ${i} boost_lib_tmp)
            set_property(TARGET boostlibs_for_${this_target}_${idx} PROPERTY
              IMPORTED_IMPLIB${BT_NAME} "${boost_lib_tmp}")
          endif()
        endforeach()
        if(NOT "${boost_components}" STREQUAL "")
          target_link_libraries(${this_target} boostlibs_for_${this_target}_${idx})
        endif()
      endforeach()
    endif()
  endif()
  include_directories("${Boost_INCLUDE_DIRS}")
endmacro()
endif()
