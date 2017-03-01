rem @echo off
set QUIET=-q

set CURRENTDIR=%CD%
set EXTERNAL=..\external
IF EXIST %EXTERNAL% GOTO :SETDIR
mkdir %EXTERNAL%
:SETDIR
cd %EXTERNAL%

:GETGLM
set GLM=glm
set GLMDIR=%GLM%
IF EXIST %GLMDIR% GOTO :GETGLFW
set GLMURL=https://github.com/g-truc/glm/archive/master.zip
set GLMZIP=master.zip
set GLMOUT=%GLMZIP%
echo downloading %GLMURL%
powershell -command "Invoke-WebRequest %GLMURL% -OutFile %GLMOUT%"
echo expanding %GLMZIP% to %GLMDIR%
powershell -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::ExtractToDirectory('%GLMOUT%', '.'); }"
ren glm-master %GLMDIR%
del /Q /F %GLMOUT%

:GETGLFW
set GLFW=glfw
set GLFWDIR=%GLFW%
IF EXIST %GLFWDIR% GOTO :GETIMGUI
set GLFWZIP=glfw-3.2.1.bin.WIN64.zip
set GLFWURL=https://github.com/glfw/glfw/releases/download/3.2.1/%GLFWZIP%
set GLFWOUT=%GLFWZIP%
echo downloading %GLFWURL%
powershell -command "Invoke-WebRequest %GLFWURL% -OutFile %GLFWOUT%"
echo expanding %GLFWZIP% to %GLFWDIR%
powershell -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::ExtractToDirectory('%GLFWOUT%', '.'); }"
ren glfw-3.2.1.bin.WIN64 %GLFWDIR%
del /Q /F %GLFWOUT%

:GETIMGUI
set IMGUI=imgui
set IMGUIDIR=%IMGUI%
IF EXIST %IMGUIDIR% GOTO :END
set IMGUIZIP=master.zip
set IMGUIURL=https://github.com/ocornut/imgui/archive/%IMGUIZIP%
set IMGUIOUT=%IMGUIZIP%
echo downloading %IMGUIURL%
powershell -command "Invoke-WebRequest %IMGUIURL% -OutFile %IMGUIOUT%"
echo expanding %IMGUIZIP% to %IMGUIDIR%
powershell -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::ExtractToDirectory('%IMGUIOUT%', '.'); }"
ren imgui-master %IMGUIDIR%
del /Q /F %IMGUIOUT%

:END
