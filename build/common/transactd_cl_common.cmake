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
#   add source files macro
# ==========================================================
if(NOT COMMAND tdcl_add_source_files)
macro(tdcl_add_source_files TRANSACTD_ROOT)
  set(${this_target}_SOURCE_FILES
    ${${this_target}_SOURCE_FILES}
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/btrDate.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/tdapSchema.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/connMgr.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/database.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/databaseFactory.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/dbDef.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/errorMessage.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/fieldDDF.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/fileDDF.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/indexDDF.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/nsDatabase.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/nsTable.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/sharedData.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/client/table.cpp
    ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/mysql/characterset.cpp
    ${TRANSACTD_ROOT}/source/bzs/rtl/datetime.cpp
    ${TRANSACTD_ROOT}/source/bzs/rtl/stringBuffers.cpp
    ${TRANSACTD_ROOT}/source/bzs/rtl/strtrim.cpp
  )
  if(WIN32)
    set(${this_target}_SOURCE_FILES ${${this_target}_SOURCE_FILES}
      ${TRANSACTD_ROOT}/source/bzs/db/protocol/tdap/myDateTime.cpp
    )
  else()
    set(${this_target}_SOURCE_FILES ${${this_target}_SOURCE_FILES}
      ${TRANSACTD_ROOT}/source/bzs/env/crosscompile.cpp
      ${TRANSACTD_ROOT}/source/bzs/env/mbcswchrLinux.cpp
    )
  endif()
endmacro()
endif()


# ==========================================================
#   add include directory macro
# ==========================================================
if(NOT COMMAND tdcl_add_include_directory)
macro(tdcl_add_include_directory TRANSACTD_ROOT)
  include_directories(${TRANSACTD_ROOT}/source;)
  if(UNIX)
    include_directories(${TRANSACTD_ROOT}/source/linux;)
  endif()
endmacro()
endif()

# ==========================================================
#   add compiler / linker options macro
# ==========================================================
if(NOT COMMAND tdcl_add_compiler_options)
macro(tdcl_add_compiler_options)
  foreach(build_type "" "_RELEASE" "_DEBUG" "_RELWITHDEBINFO" "_MINSIZEREL")
    bz_add_cxx_flag("-DBOOST_ALL_NO_LIB -DPIC" "${build_type}")
    bz_remove_cxx_flag("-fno-exceptions" "${build_type}")
    bz_remove_cxx_flag("-fno-rtti" "${build_type}")
    bz_remove_cxx_flag("-fno-implicit-templates" "${build_type}")
    bz_remove_cxx_flag("/EHsc" "${build_type}")
    bz_remove_cxx_flag("/EHs" "${build_type}")
    bz_remove_cxx_flag("-DPACKAGE_NO_EXPORT" "${build_type}")
    if(MSVC)
      bz_add_cxx_flag("/EHa /wd4068 /DARBTREGN_PKG /DTRDCL_AUTOLINK_OFF /DTRDCLENGN_EXPORTS" "${build_type}")
      bz_set_MTMTd_cxx_flag("${build_type}")
    else()
      bz_add_cxx_flag("-fPIC -fabi-version=2 -fexceptions -finput-charset=utf-8" "${build_type}")
      bz_add_cxx_flag("-fno-omit-frame-pointer -fno-strict-aliasing -fpermissive -frtti" "${build_type}")
      if(NOT MINGW)
        bz_add_cxx_flag("-DLINUX" "${build_type}")
      endif()
    endif()
  endforeach()
  if(MINGW)
    add_definitions(-DWIN32)
    add_definitions(-D_WIN32)
  endif()
endmacro()
endif()
