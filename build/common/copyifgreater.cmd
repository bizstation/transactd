@echo off
@REM ===============================================================
@REM    Copyright (C) 2013 BizStation Corp All rights reserved.
@REM 
@REM    This program is free software; you can redistribute it and/or
@REM    modify it under the terms of the GNU General Public License
@REM    as published by the Free Software Foundation; either version 2
@REM    of the License, or (at your option) any later version.
@REM 
@REM    This program is distributed in the hope that it will be useful,
@REM    but WITHOUT ANY WARRANTY; without even the implied warranty of
@REM    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@REM    GNU General Public License for more details.
@REM 
@REM    You should have received a copy of the GNU General Public License
@REM    along with this program; if not, write to the Free Software 
@REM    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
@REM    02111-1307, USA.
@REM ===============================================================

@REM ***** CALL 64bit cmd.exe if exists! *****

:: SysNative alias is avalable if this is 32bit-cmd.exe on 64bit system.
:: %WinDir%\SysNative\cmd.exe is 64bit cmd.exe.
if exist %WinDir%\SysNative\cmd.exe (
  %WinDir%\SysNative\cmd.exe /c cscript //nologo "%~dp0copyifgreater.js" %*
) else (
  cmd /c cscript //nologo "%~dp0copyifgreater.js" %*
)
exit /b %ERRORLEVEL%
