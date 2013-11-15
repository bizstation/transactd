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
#   set install drectory
# ==========================================================
if(NOT COMMAND tdcl_set_output)
macro(tdcl_set_output TRANSACTD_BINARY_ROOT prefix)
  ## install to gem/common
  install(TARGETS ${this_target}
    LIBRARY DESTINATION "${TRANSACTD_RUBY_GEM_ROOT_PATH}/bin/common"
    RUNTIME DESTINATION "${TRANSACTD_RUBY_GEM_ROOT_PATH}/bin/common"
    ARCHIVE DESTINATION "${TRANSACTD_RUBY_GEM_ROOT_PATH}/bin/common"
  )
  ## install to system dir (copy file if greater)
  if(WIN32)
    get_target_property(tmp_var "${this_target}" LOCATION)
    get_filename_component(tmp_var "${tmp_var}" NAME)
    file(TO_CMAKE_PATH "${TRANSACTD_RUBY_GEM_ROOT_PATH}/bin/common/${tmp_var}" tmp_var)
    bz_smart_install(SOURCES "${tmp_var}" TO_WIN_SYSTEMDIR)
  else()
    if("${prefix}" STREQUAL "")
      install(TARGETS ${this_target} LIBRARY DESTINATION /usr/lib)
    else()
      install(TARGETS ${this_target} LIBRARY DESTINATION "${prefix}")
    endif()
  endif()
endmacro()
endif()
