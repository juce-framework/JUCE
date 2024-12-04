REM        Copyright 2017, 2019-2021, 2023-2024 Avid Technology, Inc.
REM All rights reserved.
REM 
REM This file is part of the Avid AAX SDK.
REM 
REM The AAX SDK is subject to commercial or open-source licensing.
REM 
REM By using the AAX SDK, you agree to the terms of both the Avid AAX SDK License
REM Agreement and Avid Privacy Policy.
REM 
REM AAX SDK License: https://developer.avid.com/aax
REM Privacy Policy: https://www.avid.com/legal/privacy-policy-statement
REM 
REM Or: You may also use this code under the terms of the GPL v3 (see
REM www.gnu.org/licenses).
REM 
REM THE AAX SDK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
REM EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
REM DISCLAIMED.

set OutDir="%~fn1"
set IconSource="%~fn2"

echo Create Package Directories

IF EXIST %OutDir% GOTO OUTDIR_EXISTS
mkdir %OutDir%
:OUTDIR_EXISTS

IF EXIST %OutDir%\..\Win32 GOTO Win32_EXISTS
mkdir %OutDir%\..\Win32
:Win32_EXISTS

IF EXIST %OutDir%\..\x64 GOTO X64_EXISTS
mkdir %OutDir%\..\x64
:X64_EXISTS

IF EXIST %OutDir%\..\Resources GOTO RESOURCES_EXISTS
mkdir %OutDir%\..\Resources
:RESOURCES_EXISTS

echo Set Folder Icon

IF EXIST %OutDir%\..\..\PlugIn.ico GOTO ICON_EXISTS
copy /Y %IconSource% %OutDir%\..\..\PlugIn.ico > NUL
:ICON_EXISTS

attrib -r %OutDir%\..\..
attrib -h -r -s %OutDir%\..\..\desktop.ini
echo [.ShellClassInfo] > %OutDir%\..\..\desktop.ini 
echo IconResource=PlugIn.ico,0 >> %OutDir%\..\..\desktop.ini 
echo ;For compatibility with Windows XP >> %OutDir%\..\..\desktop.ini 
echo IconFile=PlugIn.ico >> %OutDir%\..\..\desktop.ini 
echo IconIndex=0 >> %OutDir%\..\..\desktop.ini 
attrib +h +r +s %OutDir%\..\..\PlugIn.ico
attrib +h +r +s %OutDir%\..\..\desktop.ini
attrib %OutDir%\..\..
