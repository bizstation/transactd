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
include(../common/system.cmake)
include(../common/smart_install.cmake)
# ==========================================================
#   set install drectory
# ==========================================================
if(NOT COMMAND tdcl_set_output)
macro(tdcl_set_output TRANSACTD_BINARY_ROOT prefix)
  if("${TRANSACTD_RUBY_GEM_ROOT_PATH}" STREQUAL "")
    set(TRANSACTD_RUBY_GEM_ROOT_PATH "${TRANSACTD_BINARY_ROOT}")
  endif()
  ## install tdclcpp
  if((NOT WIN32) OR MSVC)
    foreach(tmp_build_type "_RELEASE" "_DEBUG" "_RELWITHDEBINFO" "_MINSIZEREL")
      set_target_properties(tdclcpp PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY${tmp_build_type} "${TRANSACTD_RUBY_GEM_ROOT_PATH}/bin/common"
        LIBRARY_OUTPUT_DIRECTORY${tmp_build_type} "${TRANSACTD_RUBY_GEM_ROOT_PATH}/bin/common"
        ARCHIVE_OUTPUT_DIRECTORY${tmp_build_type} "${TRANSACTD_RUBY_GEM_ROOT_PATH}/bin/common")
    endforeach()
    ## install to system dir (copy file if greater)
    if(WIN32)
      set(tmp_target_name "")
      if(TARGET "tdclcpp")
        set(tmp_target_name "tdclcpp")
      elseif(TARGET "tdclcppmr")
        set(tmp_target_name "tdclcppmr")
      elseif(TARGET "tdclcppm")
        set(tmp_target_name "tdclcppm")
      endif()
      if (NOT "${tmp_target_name}" STREQUAL "")
        string(TOUPPER "OUTPUT_NAME_${CMAKE_BUILD_TYPE}" tmp_var)
        get_target_property(tmp_var "${tmp_target_name}" "${tmp_var}")
        set(tmp_var "${tmp_var}.dll")
      else()
        set(tmp_var "tdclcpp_vc100_*.dll")
      endif()
      file(TO_CMAKE_PATH "${TRANSACTD_RUBY_GEM_ROOT_PATH}/bin/common/${tmp_var}" tmp_var)
      bz_smart_install(SOURCES "${tmp_var}" TO_WIN_SYSTEMDIR)
    else()
      if("${prefix}" STREQUAL "")
        install(TARGETS tdclcpp LIBRARY DESTINATION /usr/lib)
      else()
        install(TARGETS tdclcpp LIBRARY DESTINATION "${prefix}")
      endif()
    endif()
  endif()
endmacro()
endif()
