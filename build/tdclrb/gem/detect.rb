# coding : utf-8
=begin =============================================================
   Copyright (C) 2013 BizStation Corp All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software 
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
   02111-1307, USA.
 ===================================================================
=end
require 'rbconfig'

def get_binary_path
  return File.join(RUBY_VERSION.match(/\d+\.\d+/)[0], 'transactd')
end

def has_binary(transactd_gem_root)
  prebuilt_path = File.join(transactd_gem_root, 'bin', get_binary_path() + '.so')
  common_path   = File.join(transactd_gem_root, 'bin/common')
  return File.exist?(prebuilt_path) && File.exist?(common_path)
end
