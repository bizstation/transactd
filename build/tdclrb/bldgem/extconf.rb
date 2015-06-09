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

transactd_gem_root_relative = '../../../'

require File.expand_path(File.join(transactd_gem_root_relative, 'build/tdclrb/gem/detect.rb'))
require File.expand_path(File.join(transactd_gem_root_relative, 'build/tdclrb/gem/helper.rb'))
require 'rbconfig'

transactd_gem_root = File.expand_path(File.join(File.dirname(__FILE__), transactd_gem_root_relative))
ruby_bin_path = RbConfig::CONFIG['bindir']
ruby_exe_path = File.join(RbConfig::CONFIG['bindir'], RbConfig::CONFIG['ruby_install_name'])

#
# use prebuilt binary
#
if has_binary(transactd_gem_root)
  make_makefile_prebuilt_win32(ruby_bin_path, transactd_gem_root) if RUBY_PLATFORM =~ /mswin/ || RUBY_PLATFORM =~ /mingw/
  exit
end

#
# build binary
#

require 'mkmf'
require 'open3'

# need cmake and swig
find_executable('cmake')
find_executable('swig')

# options
boost             = arg_config('--boost', '').gsub(/"\n/, '')
generator         = arg_config('--generator', '').gsub(/"\n/, '')
ruby_include_dirs = arg_config('--ruby_include_dirs', '').gsub(/"\n/, '')
ruby_library_path = arg_config('--ruby_library_path', '').gsub(/"\n/, '')
install_prefix    = arg_config('--install_prefix', '').gsub(/"\n/, '')
build_type        = arg_config('--build_type', '').gsub(/"\n/, '')
no_rb_tbr         = arg_config('--without_rb_thread_blocking_region', '').gsub(/"\n/, '').downcase
no_c_cpp          = arg_config('--without_c_cpp_clients')

# boost
if boost != '' && boost !=~ /^\-DBOOST_ROOT/
  boost = '-DBOOST_ROOT="' + to_slash_path(boost) + '"'
end

# detect generator
if generator == ''
  if RUBY_PLATFORM =~ /mswin/
    bit = (get_ruby_bitness() == 64) ? ' Win64' : ''
    generator = '-G "Visual Studio 10' + bit + '"'
  elsif RUBY_PLATFORM =~ /mingw/
    generator = '-G "MSYS Makefiles"'
  end
else
  if generator !=~ /^\-G /
    generator = '-G "' + generator + '"'
  end
end

# ruby_include_dirs
if ruby_include_dirs != '' && ruby_include_dirs !=~ /^\-DRUBY_SWIG_INCLUDE_PATH/
  ruby_include_dirs = '-DTRANSACTD_RUBY_INCLUDE_PATH="' + to_slash_path(ruby_include_dirs) + '"'
end

# ruby_library_path
if ruby_library_path != '' && ruby_library_path !=~ /^\-DRUBY_SWIG_LIBRARY_PATH/
  lib_path = to_slash_path(ruby_library_path)
  $LDFLAGS = $LDFLAGS + ' -libpath:' + File.dirname(lib_path)
  ruby_library_path = '-DTRANSACTD_RUBY_LIBRARY_PATH="' + lib_path + '"'
end

# install_prefix
if install_prefix != '' && install_prefix !=~ /^\-DCMAKE_INSTALL_PREFIX/
  install_prefix = '-DTRANSACTD_CLIENTS_PREFIX="' + to_slash_path(install_prefix) + '"'
end

# build_type
if build_type !=~ /^\-DCMAKE_BUILD_TYPE/
  if build_type != ''
    build_type = '-DCMAKE_BUILD_TYPE=' + build_type
  else
    build_type = '-DCMAKE_BUILD_TYPE=Release'
  end
end

# ruby executable path
ruby_executable = File.join(RbConfig::CONFIG['bindir'], RbConfig::CONFIG['ruby_install_name'])
ruby_executable = '-DTRANSACTD_RUBY_EXECUTABLE_PATH="' + to_slash_path(ruby_executable) + '"'

# output dir
gem_root = '-DTRANSACTD_RUBY_GEM_ROOT_PATH="' + to_slash_path(transactd_gem_root) + '"'

# rb_thread_blocking_region or rb_thread_call_without_gvl
use_TCWOG = have_func('rb_thread_call_without_gvl', 'ruby/thread.h')
use_TBR = (! use_TCWOG) && have_func('rb_thread_blocking_region')
if no_rb_tbr != '' && no_rb_tbr != 'off'
  use_TCWOG = false
  use_TBR = false
end
rb_tbr = ' -DTRANSACTD_HAVE_RB_THREAD_CALL_WITHOUT_GVL=' +
  (use_TCWOG ? 'ON' : 'OFF') +
  ' -DTRANSACTD_HAVE_RB_THREAD_BLOCKING_REGION=' +
  (use_TBR ? 'ON' : 'OFF')

# no_c_cpp
no_c_cpp = no_c_cpp ? ' -DTRANSACTD_WITHOUT_C_CPP_CLIENTS=ON' : ''

# cmake
cmake_cmd = ['cmake', to_native_path(transactd_gem_root_relative), '-DTRANSACTD_RUBY_GEM=ON',
              generator, boost, ruby_executable, ruby_library_path, ruby_include_dirs,
              install_prefix, gem_root, build_type, rb_tbr, no_c_cpp, '>> cmake_generate.log'].join(' ')
begin
  f = open('cmake_generate.log', 'w')
  f.puts cmake_cmd
ensure
  f.close
end
stdout, stderr, status = Open3.capture3(cmake_cmd)
succeed = (status == 0)
STDERR.puts stderr if !succeed

# crete dummy Makefile for Visual Studio .sln
if /Visual Studio/ =~ generator
  FileUtils.copy('../gem/Makefile.win32-VS', './nmake.cmd')
  begin
    mkfile_dummy = open('Makefile', 'w')
  ensure
    mkfile_dummy.close
  end
end

$makefile_created = true if succeed
