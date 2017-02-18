rem @echo off
set QUIET=-q

set CURRENTDIR=%CD%
set EXTERNAL=..\external
IF EXIST %EXTERNAL% GOTO :SETDIR
mkdir %EXTERNAL%
:SETDIR
cd %EXTERNAL%

:GETGOOGLETEST
cd %EXTERNAL%
set GOOGLETEST=googletest
set GOOGLETESTDIR=%GOOGLETEST%
IF EXIST %GOOGLETESTDIR% GOTO :END
set GOOGLETESTURL=https://github.com/google/googletest/archive/master.zip
set GOOGLETESTZIP=googletext.zip
set GOOGLETESTOUT=%GOOGLETESTZIP%
echo downloading %GOOGLETESTURL%
powershell -command "Invoke-WebRequest %GOOGLETESTURL% -OutFile %GOOGLETESTOUT%"
echo expanding %GOOGLETESTZIP% to %GOOGLETESTDIR%
powershell -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::ExtractToDirectory('%GOOGLETESTOUT%', '.'); }"
ren googletest-master %GOOGLETESTDIR%
del /Q /F %GOOGLETESTOUT%

:END
