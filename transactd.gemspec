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
  
  # read major/minor version from tdapcapi.h
  verfile = 'source/bzs/db/protocol/tdap/tdapcapi.h'
  unless File.exist?(verfile)
    raise 'Can not found ' + verfile
  end
  versions = {}
  File.open(verfile, "r") {|f|
    f.each_line { |l|
      if l.index("Build marker! Don't remove") != nil
        l = l.sub(/#define/, '').sub(/\/\/.*$/, '').gsub('"', '').strip().split(/\s/)
        versions[l[0].to_sym] = Integer(l[1]) if l.length == 2
      end
    }
  }
  unless (versions.has_key?(:CPP_INTERFACE_VER_MAJOR) &&
          versions.has_key?(:CPP_INTERFACE_VER_MINOR))
    raise 'Can not read versions from ' + verfile
  end
  # read release version from GEM_RELEASE_VERSION
  verfile = 'build/tdclrb/GEM_RELEASE_VERSION'
  unless File.exist?(verfile)
    raise 'Can not found ' + verfile
  end
  File.open(verfile, "r") {|f|
    l = f.read.gsub(/\s\n/, '')
    versions[:GEM_RELEASE_VERSION] = Integer(l)
  }
  s.version = versions[:CPP_INTERFACE_VER_MAJOR].to_s + '.' + 
              versions[:CPP_INTERFACE_VER_MINOR].to_s + '.' + 
              versions[:GEM_RELEASE_VERSION].to_s
  
  binary_file = File.join('bin', RUBY_VERSION.match(/\d+\.\d+/)[0], 'transactd.so')
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
    s.files      += Dir.glob('./*')
  else
    #
    # source code package
    #
    s.platform    = Gem::Platform::RUBY
    s.extensions  = ['build/tdclrb/bldgem/extconf.rb']
    s.files       = ['CMakeLists.txt']
    s.files      += Dir.glob('bin/common/*.dll') + Dir.glob('bin/common/*.so')
    s.files      += Dir.glob('source/**/*') + Dir.glob('build/common/**/*')
    s.files      += Dir.glob('build/swig/**/*') + Dir.glob('build/tdclc/**/*')
    s.files      += Dir.glob('build/tdclcpp/**/*') + Dir.glob('build/tdclrb/**/*')
    s.files      += Dir.glob('./*')
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
