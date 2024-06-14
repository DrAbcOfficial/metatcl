param([int]$param)

if($param -eq 0){
    Write-Output "[Chose your building target]"
    Write-Output "#1 Release"
    Write-Output "#2 Debug"
    $Chosen = Read-Host
}
else{
    $Chosen=$param
}
switch ($Chosen) {
    "1" { $BuildTarget = "Release" }
    "2" { $BuildTarget = "Debug" }
    Default {
        Write-Output "Ivalid Input, exiting.. 1 = Release, 2 = Debug"
        Exit
    }
}

Write-Warning "Starting tclib building..."
&"./build-tclmodule.bat"

Write-Warning "Starting plugin building..."
$vsLocation=&"../../tools/vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if(Test-Path("$($vsLocation)\Common7\Tools\vsdevcmd.bat")){
    &"$($vsLocation)\Common7\Tools\vsdevcmd.bat" "-arch=x86"
    &"$($vsLocation)\Msbuild\Current\Bin\MSBuild.exe" "$(Split-Path -Parent $MyInvocation.MyCommand.Definition)/Metatcl.vcxproj" /p:Configuration=$($BuildTarget) /p:Platform="Win32"
    Copy-Item -Force -Path "./$($BuildTarget)/Metatcl.dll" -Destination "./Build/metahook/plugins/Metatcl.dll"
    Copy-Item -Force -Path "./tclib/bin/tcl86t.dll" -Destination "./Build/metahook/dlls/tcl86t/tcl86t.dll"
    Copy-Item -Force -Path "./tclib/lib/tcl8.6" -Destination "./Build/metahook/dlls/tcl86t/lib"
    Copy-Item -Force -Path "./tclib/lib/reg1.3" -Destination "./Build/metahook/dlls/tcl86t/pkg/reg1.3"
    Copy-Item -Force -Path "./tclib/lib/dde1.4" -Destination "./Build/metahook/dlls/tcl86t/pkg/dde1.4"
}
else {
    Write-Error "`n`
        -------------------------------------------------`n`
        (╯‵□′)╯︵┻━┻ Can not find Visual Studio`n`
        Is the plugin folder in the correct location?`n`
        Eg: ./Github/MetaHookSv/Plugins/Metatcl`n`
        -------------------------------------------------`n"
}
Exit