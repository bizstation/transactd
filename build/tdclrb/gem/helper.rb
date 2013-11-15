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
def to_native_path(path)
  return File::ALT_SEPARATOR ? path.gsub('/', File::ALT_SEPARATOR) : path
end

def to_slash_path(path)
  return File::ALT_SEPARATOR ? path.gsub(File::ALT_SEPARATOR, '/') : path
end

def get_ruby_bitness
  return ['a'].pack('P').length > 4 ? 64 : 32
end

def get_windows_bitness
  return ENV.has_key?('ProgramFiles(x86)') ? 64 : 32
end

def get_windows_sysdir()
  ruby_bitness = get_ruby_bitness()
  windows_bitness = get_windows_bitness()
  if windows_bitness == 32
    return to_native_path(File.join(ENV['Systemroot'], 'System32/'))
  else
    if ruby_bitness == 32
      return to_native_path(File.join(ENV['Systemroot'], 'SysWOW64/'))
    else
      return to_native_path(File.join(ENV['Systemroot'], 'System32/'))
    end
  end
end

def get_common_binary_files(transactd_gem_root)
  ruby_bitness = get_ruby_bitness()
  windows_bitness = get_windows_bitness()
  ruby_bitness = 32 if windows_bitness == 32
  bin_files = Dir.glob(File.join(transactd_gem_root, 'bin/common/tdclc_' + ruby_bitness.to_s + '_*.dll'))
  bin_files += Dir.glob(File.join(transactd_gem_root, 'bin/common/tdclcpp_*_' + ruby_bitness.to_s + 'm_*.dll'))
  sysdir = get_windows_sysdir()
  bin_files = bin_files.map{ |i| '"' + to_native_path(i) + '" "' + sysdir + '"' }
  return bin_files
end

def make_makefile_prebuilt_win32(ruby_bin_path, transactd_gem_root)
  bin_files = get_common_binary_files(transactd_gem_root)
  begin
    mkfile_dummy = open('Makefile', 'w')
    mkfile = open('nmake.cmd', 'w')
    srcpath = File.join(transactd_gem_root, 'build/tdclrb/gem/Makefile.win32-prebuilt')
    copycmd = File.join(transactd_gem_root, 'build/common/copyifgreater.cmd')
    source = open(srcpath, 'r')
    mkfile.puts '@echo off'
    mkfile.puts 'set TRANSACTD_GEM_ROOT=' + to_native_path(transactd_gem_root)
    mkfile.puts 'set RUBY_BIN_PATH=' + to_native_path(ruby_bin_path)
    mkfile.puts 'set COPYCMD=' + to_native_path(copycmd)
    mkfile.puts source.read
    mkfile.puts ''
    bin_files.each{ |i|
      files = i.split(' ').map{ |i| i.strip.gsub!(/(^(\s|\"|)+)|((\s|\")+$)/, '')}
      srcfile = (files.length > 0) ? files[0] : 'unknown error'
      dstfile = (files.length > 1) ? files[1] : 'unknown error'
      mkfile.puts  <<EOS
call "%COPYCMD%" #{i} >> "%INSTALLLOG%" 2>> "%ERRMSG%"
call :getsize "%ERRMSG%"
if !getsize_ret! GTR 0 (
  if ERRORLEVEL 1 (
    echo ************************************************************** 1>&2
    echo EROOR MESSAGE 1>&2
    echo Failed to Install. 1>&2
    echo Perhaps it require to run as Administrator, or file is in use. 1>&2
    type "%ERRMSG%" 1>&2
    echo ************************************************************** 1>&2
    exit /b 1
  ) else (
    echo ************************************************************** >> "%INSTALLLOG%"
    echo WARNING MESSAGE >> "%INSTALLLOG%"
    echo Files not copied because it could not read the version from the binary. >> "%INSTALLLOG%"
    echo There is a possibility that the program does not work properly. >> "%INSTALLLOG%"
    echo Please check the compatibility of following binaries. >> "%INSTALLLOG%"
    type "%ERRMSG%" >> "%INSTALLLOG%"
    echo ************************************************************** >> "%INSTALLLOG%"
    del "%ERRMSG%"
  )
)
EOS
    }
    mkfile.puts 'exit /b 0'
    mkfile.puts 'endlocal'
    $makefile_created = true
  ensure
    source.close
    mkfile.close
    mkfile_dummy.close
  end
end
