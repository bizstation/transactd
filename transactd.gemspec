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
spec_build = Gem::Specification.new do |s|
  s.name        = 'transactd'
  s.summary     = 'Transactd client'
  s.description = 'Transactd client for ruby gem'
  s.author      = 'BizStation Corp.'
  s.email       = 'transactd@bizstation.jp'
  s.homepage    = 'http://www.bizstation.jp/ja/transactd'
  s.license     = 'GPL v2'
  
  # read version from tdclrb.rc
  verfile = 'build/tdclrb/tdclrb.rc'
  raise 'Can not found ' + verfile unless File.exist?(verfile)
  verpattern = /^[\t ]*VALUE[\t ]+"ProductVersion"[\t ]*,[\t ]*"([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)"[\t\r ]*$/i
  ver_str = ''
  File.open(verfile, "r") {|f|
    f.each_line { |l|
      m = verpattern.match(l)
      if m != nil
        ver_str = m[1].split('.').slice(0..2).join('.')
        break
      end
    }
  }
  raise 'Can not read versions from ' + verfile + ' (' + ver_str + ')' if ver_str.length == 0
  s.version = ver_str
  
  binary_file = File.join('bin', RUBY_VERSION.sub(/(\d+\.\d+)\..*/, '\1'), 'transactd.so')
  binarymode = File.exist?(binary_file)
  
  if binarymode
    #
    # use prebuilt binary
    #
    s.platform    = Gem::Platform.local
    s.extensions  = ['build/tdclrb/bldgem/extconf.rb']
    s.files       = Dir.glob('bin/**/*.so')
    s.files      += Dir.glob('bin/common/*.dll') + Dir.glob('bin/common/*.so')
    s.files      += Dir.glob('build/tdclrb/gem/**/*')
    s.files      += Dir.glob('build/common/copyifgreater.*')
    s.files      += Dir.glob('build/common/system.*')
    s.files      += Dir.glob('source/bzs/test/tdclrb/*')
    s.files      += Dir.glob('./*').reject { |f| f.match(/RELEASE_NOTE/) || f.match(/BUILD_/) }
  else
    #
    # source code package
    #
    s.platform    = Gem::Platform::RUBY
    s.extensions  = ['build/tdclrb/bldgem/extconf.rb']
    s.files       = ['CMakeLists.txt']
    s.files      += Dir.glob('bin/common/*.dll') + Dir.glob('bin/common/*.so')
    s.files      += Dir.glob('source/**/*') + Dir.glob('build/common/**/*')
    s.files      += Dir.glob('build/swig/*') + Dir.glob('build/swig/ruby/*')
    s.files      += Dir.glob('build/swig/ruby/**/*') + Dir.glob('build/tdclrb/**/*')
    s.files      += Dir.glob('build/tdclc/**/*') + Dir.glob('build/tdclcpp/**/*')
    s.files      += Dir.glob('./*').reject { |f| f.match(/RELEASE_NOTE/) || f.match(/BUILD_/) }
    if RUBY_PLATFORM =~ /mswin/ || RUBY_PLATFORM =~ /mingw/
      # add prebuilt binary
      tdclc_file  = File.join('bin', 'common', 'tdclc_*.*')
      s.files    += Dir.glob(tdclc_file)
    end
  end
  
  msgs = []
  if binarymode
    msgs.push('You installed PRE-BUILT BINARY of Transactd client.')
  else
    msgs.push('You build Transactd client from source code.')
  end
  if (RUBY_PLATFORM =~ /mswin/)
    msgs.push('*** Please check install log ***')
    msgs.push('TRANSACTD_GEM_DIR/install.log')
  else
    msgs.push('Install log is here:')
    msgs.push('TRANSACTD_GEM_DIR/../build/tdclrb/bldgem/install_manifest.txt')
  end
  msgs.push('(TRANSACTD_GEM_DIR: `gem which transactd` and remove "lib/transactd.rb")')
  msgs.push('-' * 50)
  msgs.unshift('-' * 50)
  s.post_install_message = msgs.join("\n") if msgs.length > 0
end
