##=================================================================
#   Copyright (C) 2012-2016 BizStation Corp All rights reserved.
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
## Transactd server plugin
##
##  *** Please run with ../../CMakeLists.txt ***
##
cmake_minimum_required(VERSION 2.6)
include(../common/transactd_required.cmake)
include(../common/transactd_common.cmake)
include(../common/options.cmake)
include(../common/system.cmake)
include(../common/get_boost_libs.cmake)


set(this_target transactd)
project(${this_target})


# ==========================================================
# set global variables
# ==========================================================
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  message(SEND_ERROR "Transactd server plugin must be built with MySQL sources.")
endif()
if("${TRANSACTD_ROOT}" STREQUAL "")
  get_filename_component(TRANSACTD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
endif()
if("${TRANSACTD_MYSQL_SOURCE_ROOT}" STREQUAL "")
  set(TRANSACTD_MYSQL_SOURCE_ROOT "${CMAKE_SOURCE_DIR}")
endif()
if("${TRANSACTD_MYSQL_BINARY_ROOT}" STREQUAL "")
  set(TRANSACTD_MYSQL_BINARY_ROOT "${CMAKE_BINARY_DIR}")
endif()


# ==========================================================
# version info
# ==========================================================
transactd_read_version("${TRANSACTD_ROOT}")


# ==========================================================
# source files
# ==========================================================
set(${this_target}_SOURCE_FILES
  ${TRANSACTD_ROOT}/source/bzs/db/engine/mysql/database.cpp
  ${TRANSACTD_ROOT}/source/bzs/db/engine/mysql/dbManager.cpp
  ${TRANSACTD_ROOT}/source/bzs/db/engine/mysql/errorMessage.cpp
  ${TRANSACTD_ROOT}/source/bzs/db/engine/mysql/ha.cpp
  ${TRANSACTD_ROOT}/source/bzs/db/engine/mysql/mydebuglog.cpp
  ${TRANSACTD_ROOT}/source/bzs/db/engine/mysql/mysqlThd.cpp
  ${TRANSACTD_ROOT}/source/bzs/db/engine/mysql/mysqlProtocol.cpp
  ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/myDateTime.cpp
  ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/mysql/tdapCommandExecuter.cpp
  ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/mysql/characterset.cpp
  ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/mysql/databaseSchema.cpp
  ${TRANSACTD_ROOT}/source/bzs/db/protocol/hs/hsCommandExecuter.cpp
  ${TRANSACTD_ROOT}/source/bzs/db/transactd/appModule.cpp
  ${TRANSACTD_ROOT}/source/bzs/db/transactd/connManager.cpp
  ${TRANSACTD_ROOT}/source/bzs/db/transactd/transactd.cpp
  ${TRANSACTD_ROOT}/source/bzs/env/crosscompile.cpp
  ${TRANSACTD_ROOT}/source/bzs/netsvc/server/serverCpt.cpp
  ${TRANSACTD_ROOT}/source/bzs/netsvc/server/serverTpool.cpp
  ${TRANSACTD_ROOT}/source/bzs/rtl/debuglog.cpp
  ${TRANSACTD_ROOT}/source/bzs/rtl/stl_uty.cpp
)
if(WIN32)
  set(${this_target}_SOURCE_FILES ${${this_target}_SOURCE_FILES}
    ${TRANSACTD_ROOT}/source/bzs/netsvc/server/serverPipe.cpp)
endif()


# ==========================================================
# include directories
# ==========================================================
include_directories(
  ${TRANSACTD_MYSQL_SOURCE_ROOT};
  ${TRANSACTD_MYSQL_SOURCE_ROOT}/extra/yassl/include;
  ${TRANSACTD_MYSQL_SOURCE_ROOT}/include/mysql;
  ${TRANSACTD_MYSQL_SOURCE_ROOT}/libbinlogevents/export;
  ${TRANSACTD_MYSQL_SOURCE_ROOT}/libbinlogevents/include;
  ${TRANSACTD_MYSQL_BINARY_ROOT}/libbinlogevents/include;
  ${TRANSACTD_MYSQL_SOURCE_ROOT}/zlib;
  ${TRANSACTD_ROOT}/source;)
if(UNIX)
  include_directories(${TRANSACTD_ROOT}/source/linux;)
endif()


# ==========================================================
# add resource file (for Windows Visual Studio)
# ==========================================================
if(WIN32)
  transactd_ver_info_lic()
  # detect MySQL or MariaDB with $PRODUCTNAME which is set in:
  #   mysql/cmake/mysql_version.cmake   SET(PRODUCTNAME "MySQL Server")
  #   mariadb/cmake/mysql_version.cmake SET(PRODUCTNAME "MariaDB Server")
  string(REGEX MATCH ".*MariaDB.*" TDVER_DB_TMP "${PRODUCTNAME}")
  if("${TDVER_DB_TMP}" STREQUAL "")
    set(TDVER_DB_PRODUCT "MySQL")
  else()
    set(TDVER_DB_PRODUCT "MariaDB")
  endif()
  # disable versioninfo_dll.rc (add in MYSQL_ADD_PLUGIN macro)
  #   set file property as "header" then not compiled
  set_source_files_properties(
    ${TRANSACTD_MYSQL_BINARY_ROOT}/versioninfo_dll.rc
    PROPERTIES HEADER_FILE_ONLY TRUE)
  # create / add rc file
  set(${this_target}_RC_FILE "${CMAKE_CURRENT_BINARY_DIR}/transactd.rc")
  transactd_generate_rc_file(
    TRANSACTD_ROOT "${TRANSACTD_ROOT}"
    OUTFILE "${${this_target}_RC_FILE}"
    F_VER   "${TDVER_TRANSACTD_VER_MAJOR}.${TDVER_TRANSACTD_VER_MINOR}.${TDVER_TRANSACTD_VER_RELEASE}.0"
    P_VER   "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}.0"
    P_NAME  "Transactd Plugin ${TRANSACTD_VER_POSTFIX}"
    F_DESC  "Transactd Plugin for ${TDVER_DB_PRODUCT}"
  )
  transactd_add_rc_file()
endif()


# ==========================================================
# add_library as MySQL plugin
# ==========================================================
if(WIN32)
  set(output_name transactd)
else()
  set(output_name libtransactd)
endif()
MYSQL_ADD_PLUGIN(${this_target} ${${this_target}_SOURCE_FILES}
  MODULE_ONLY MODULE_OUTPUT_NAME "${output_name}")


# ==========================================================
# add SONAME
# ==========================================================
if(NOT APPLE)
  set_target_properties(${this_target} PROPERTIES
    SOVERSION ${TDVER_TRANSACTD_VER_MAJOR}.${TDVER_TRANSACTD_VER_MINOR}.${TDVER_TRANSACTD_VER_RELEASE})
endif()


# ==========================================================
# Add compile definitions
# ==========================================================
add_definitions(-DBOOST_ALL_NO_LIB -DPACKAGE_NO_EXPORT)
if(TRANSACTD_HANDLERSOCKET)
  add_definitions(-DUSE_HANDLERSOCKET)
endif()
if(MSVC)
  remove_definitions(-DWIN32_LEAN_AND_MEAN)
elseif(NOT MINGW)
  add_definitions(-DLINUX)
endif()


# ==========================================================
# Add compile options and linker options
# ==========================================================
if(MSVC)
  set(CMAKE_CXX_FLAGS "/W3 /wd4068 /GL /Z7 /GF /Oy- /EHa")
  foreach(build_type "_RELEASE" "_DEBUG" "_RELWITHDEBINFO" "_MINSIZEREL")
    bz_remove_cxx_flag("/EHsc" "${build_type}")
    bz_remove_cxx_flag("/EHs" "${build_type}")
    bz_remove_cxx_flag("/Ob2" "${build_type}")
  endforeach()
  bz_add_cxx_flag("/MT" "_RELEASE")
  bz_add_cxx_flag("/MTd" "_DEBUG")
  bz_add_cxx_flag("/MT" "_RELWITHDEBINFO")
  bz_add_cxx_flag("/MD" "_MINSIZEREL")
  set_target_properties(${this_target} PROPERTIES LINK_FLAGS
    "/ALLOWISOLATION /OPT:REF /OPT:ICF")
else()
  # Set -fno-rtti to mysqlProtocol.cpp
  if((MYSQL_VERSION_ID VERSION_GREATER 100000) OR (MYSQL_VERSION_ID VERSION_LESS 50600))
    set_source_files_properties(
      "${TRANSACTD_ROOT}/source/bzs/db/engine/mysql/mysqlProtocol.cpp"
      PROPERTIES COMPILE_FLAGS -fno-rtti)
  endif()
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS
      "-std=c++11 -frtti -Wno-unknown-pragmas -Wno-char-subscripts -Wno-deprecated-register -Wno-ignored-qualifiers -Wno-missing-field-initializers -Wno-sign-compare")
  else()
    set(CMAKE_CXX_FLAGS "-std=c++0x -frtti -Wno-unknown-pragmas")
  endif()
endif()


# ==========================================================
# add boost libraries and include path
# ==========================================================
transactd_link_boost_libraries("filesystem;thread;chrono;system")

# linux need "-dl -stdc++" 
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  target_link_libraries(${this_target} dl stdc++)
endif()



# ==========================================================
# set output path (on Visual Studio)
# ==========================================================
if(MSVC)
  set(pluginpath "${TRANSACTD_MYSQL_BINARY_ROOT}/sql/lib/plugin")
  foreach(build_type "RELEASE" "DEBUG" "RELWITHDEBINFO" "MINSIZEREL")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/win_install_${build_type}.cmake
"
  file(GLOB dellist \"${pluginpath}/${output_name}*\")
  if(NOT \"\${dellist}\" STREQUAL \"\")
    file(REMOVE \${dellist})
  endif()
  file(GLOB copylist
    \"${CMAKE_CURRENT_BINARY_DIR}/${build_type}/${output_name}*\")
  file(COPY \${copylist} DESTINATION \"${pluginpath}/\")
")
  endforeach()
  add_custom_command(TARGET ${this_target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -P
    ${CMAKE_CURRENT_BINARY_DIR}/win_install_$<CONFIGURATION>.cmake)
endif()
