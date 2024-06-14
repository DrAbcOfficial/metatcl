@ECHO OFF
SET VSWHERE="..\..\tools\vswhere.exe"
for /f "usebackq tokens=*" %%i in (`..\..\tools\vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
  set VSPATH=%%i
)

CD ".\tcl\win"

if exist "%VSPATH%\VC\Auxiliary\Build\Microsoft.VCToolsVersion.default.txt" (
  set /p Version=<"%VSPATH%\VC\Auxiliary\Build\Microsoft.VCToolsVersion.default.txt"
)

if "%processor_architecture%"=="x86" (
SET Host=x86
CALL "%VSPATH%\VC\Auxiliary\Build\vcvars32.bat"
) else (
SET Host=x64
CALL "%VSPATH%\VC\Auxiliary\Build\vcvarsamd64_x86.bat"
)
"%VSPATH%\VC\Tools\MSVC\%Version%\bin\Host%Host%\x86\nmake.exe" -f makefile.vc all
"%VSPATH%\VC\Tools\MSVC\%Version%\bin\Host%Host%\x86\nmake.exe" -f makefile.vc install INSTALLDIR="../../tclib"