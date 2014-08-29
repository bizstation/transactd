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
#   replace CXX_FLAGS
# ==========================================================
if(NOT COMMAND bz_replace_cxx_flag)
macro(bz_replace_cxx_flag ptn val build_type)
  if("${build_type}" STREQUAL "_")
    set(build_type "")
  elseif(NOT ("${build_type}" STREQUAL ""))
    string(REGEX MATCH "^_" tmp_match "${build_type}")
    if("${tmp_match}" STREQUAL "")
      set(build_type "_${build_type}")
    endif()
  endif()
  string(REGEX REPLACE "${ptn}" "${val}"
    CMAKE_CXX_FLAGS${build_type} "${CMAKE_CXX_FLAGS${build_type}}")
endmacro()
endif()

# ==========================================================
#   remove CXX_FLAGS
# ==========================================================
if(NOT COMMAND bz_remove_cxx_flag)
macro(bz_remove_cxx_flag opt build_type)
  bz_replace_cxx_flag("${opt}" "" "${build_type}")
endmacro()
endif()

# ==========================================================
#   add CXX_FLAGS
# ==========================================================
if(NOT COMMAND bz_add_cxx_flag)
macro(bz_add_cxx_flag opt build_type)
  if("${build_type}" STREQUAL "_")
    set(build_type "")
  elseif(NOT ("${build_type}" STREQUAL ""))
    string(REGEX MATCH "^_" tmp_match "${build_type}")
    if("${tmp_match}" STREQUAL "")
      set(build_type "_${build_type}")
    endif()
  endif()
  set(CMAKE_CXX_FLAGS${build_type} "${CMAKE_CXX_FLAGS${build_type}} ${opt}")
endmacro()
endif()
