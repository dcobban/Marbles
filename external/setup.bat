@echo off

set QUIET=-q

:CLONETEST
IF EXIST .\googletest GOTO :GETCMAKE
set GOOGLETESTURL=https://github.com/google/googletest/archive/master.zip
set GOOGLETESTZIP=googletext.zip
echo downloading %GOOGLETESTURL%
powershell -command "Invoke-WebRequest %GOOGLETESTURL% -OutFile %GOOGLETESTZIP%"
powershell -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::ExtractToDirectory('%GOOGLETESTZIP%', '.'); }"
ren googletest-master googletest
del /Q /F %GOOGLETESTZIP%

:GETCMAKE
rem set CMAKE=cmake-3.5.2-win32-x86
rem set CMAKEZIP=%CMAKE%.zip
rem set CMAKEURL=https://cmake.org/files/v3.5/%CMAKEZIP%
rem IF EXIST .\cmake GOTO :GENGOOGLETESTSLN
rem echo downloading %CMAKEURL%
rem powershell -command "Invoke-WebRequest %CMAKEURL% -OutFile %CMAKEZIP%"
rem powershell -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::ExtractToDirectory('%CMAKEZIP%', '.'); }"
rem ren %CMAKE% cmake
rem del /Q /F %CMAKEZIP%

rem:GENGOOGLETESTSLN
rem set CMAKEEXE=cmake/bin/cmake.exe
rem %CMAKEEXE% -G "Visual Studio 14 2015" --build .\build .\googletest\googletest\CMakeLists.txt


:END
