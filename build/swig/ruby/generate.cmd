@REM =================================================================
@REM Copyright (C) 2014 BizStation Corp All rights reserved.
@REM
@REM This program is free software; you can redistribute it and/or
@REM modify it under the terms of the GNU General Public License
@REM as published by the Free Software Foundation; either version 2
@REM of the License, or (at your option) any later version.
@REM
@REM This program is distributed in the hope that it will be useful,
@REM but WITHOUT ANY WARRANTY; without even the implied warranty of
@REM MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@REM GNU General Public License for more details.
@REM
@REM You should have received a copy of the GNU General Public License
@REM along with this program; if not, write to the Free Software
@REM Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
@REM 02111-1307, USA.
@REM =================================================================
@echo off
setlocal enabledelayedexpansion

if "%~1"=="" (
  @rem transactd\build\swig\ruby\generate.cmd\..\..\..\..\
  set tmp_td_path=%~dp0%
  set TRANSACTD_ROOT=!tmp_td_path:~0,-17!
) else (
  set TRANSACTD_ROOT=%~1
)

set OUTPUT_ROOT=%TRANSACTD_ROOT%\build\swig\ruby\gen
set SWIG_I_FILE=%TRANSACTD_ROOT%\build\swig\tdcl.i
set SWIG_OUTFILE=%OUTPUT_ROOT%\tdclrb_wrap.cpp
set SWIG_LOGFILE=%OUTPUT_ROOT%\generate.log
set SWIG_ERRORLOG=%OUTPUT_ROOT%\generate.err.log

pushd "%~d0\"

if not exist "%OUTPUT_ROOT%" ( mkdir "%OUTPUT_ROOT%" )

swig -c++ -ruby -DSWIGWIN -D_WIN32 -I"%TRANSACTD_ROOT%" -I"%TRANSACTD_ROOT%\source" ^
  -o "%SWIG_OUTFILE%" "%SWIG_I_FILE%" > "%SWIG_LOGFILE%" 2> "%SWIG_ERRORLOG%"

popd
exit /b 0
endlocal
