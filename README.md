
# MetaTCL<img src="img/icon.png" align="right" width="120"/>

A MetaHookSV plugin that introduces TCL script enviroment to the Goldsrc console.


<!-- badges: start -->


![Downloads](https://img.shields.io/github/downloads/DrAbcOfficial/metatcl/total?style=for-the-badge)
![Repo Size](https://img.shields.io/github/repo-size/DrAbcOfficial/metatcl?style=for-the-badge)
![Last Commit](https://img.shields.io/github/last-commit/DrAbcOfficial/metatcl?style=for-the-badge)

<!-- badges: end -->

----

# 🀄 <u>[点我获得中文！](READMECN.md)</u>

# 🖥️ Install
1. Download and install [MetahookSV](https://github.com/hzqst/MetaHookSv)
2. Install
3. copy folder into `(mod)` folder, e.g: `C:/SteamLibrary/Half-life/halflife/`
4. Open `svencoop/metahook/configs/plugins.lst`, add `Metatcl.dll` as a new line. 
5. Open `svencoop/metahook/configs/dllpaths.lst`, add `tcl86t` as a new line. 

# 🏗️ Build
1. clone or download [MetahookSV](https://github.com/hzqst/MetaHookSv)
2. clone or download repository
3. copy `Metatcl-master` into `MetaHookSv-master/Plugins`

    Automatic:

    1. Run `build-Metatcl.ps1` in PowerShell
   
    Manual:
   
    1. use `git submodule update --init --recursive` init tcl submodule
    2. cd to `tcl/win`
    3. use `nmake -f makefile.vc all` building tcl module, notice need use `vcvarsamd64_x86.bat`(x64 sys) or `vcvars32.bat`(x86 sys) setup nmake
    4. use`namek -f makefile.vc install INSTALLDIR="../../tclib"`install tclib
    5. open `sln` and build
 
 ----

  # 🎭Console command
 |command|param|help|
 |---|---|---|
 |s_tcl_eval|s_tcl_eval [tcl script]|eval a tcl script|
 |s_tcl_exec|s_tcl_exec [tcl file] <param1> <param2> <param3>……|exec tcl file in `(mod)`and`(mod)/tcl`folder|
 |s_tcl_reset|s_tcl_reset|reset tcl enviroment|

 # 🪡Global TCL proc
 |proc|param|help|
 |---|---|---|
 |clientcmd|clientcmd [cmd]|exec client command|
 |servercmd|servercmd [cmd]|exec server command|
 |getcvar|getcvar [cvar]|get cvar value|
 |gethudinfo|gethudinfo [index]|get specific player hud info|
 |getlocalinfo|getlocalinfo|get local player entityinfo|
 |gs_*|mulit|console command could be call directly by add gs_ prefix，e.g: console command `maps` could be called in TCL file by using `gs_maps`

# 🔍 Note
1. `s_tcl_eval` will continuously interpret statements on the virtualiser (similar to the tclsh environment), to clean up previously defined variables use `tcl_reset`.

2. The `s_tcl_exec` will automatically reset the VM.

3. The `s_tcl_exec` parameter will be passed into the file as an array of global variables with `argv` as the variable name.

4. All command start with `s_`, wth? It's a simple way to stop some "generous" server from "helping" you execute some "absolutely harmless" tcl script.

# 📬Example

1. 
Console input 
```
s_tcl_eval "set a Hello "

s_tcl_eval "set b Gordon"

s_tcl_eval "puts [$a + $b]"
```
and you will see `[TCL] Hello Gordon` in console

2. save following content as `crosshair_switch.tcl` into `halflife/tcl` folder
```tcl
#!/usr/bin/tclsh

#read cvar `crosshair` value into cvar_c
set cvar_c [getcvar crosshair]

#check condition to switch crosshair
if { $cvar_c > 0 } {
    gs_say "Crosshair Off!"
    clientcmd crosshair 0
} else {
    gs_say "Crosshair On!"
    clientcmd crosshair 1
}
```

and input`s_tcl_exec crosshair_switch`，you will see switch crosshair with a diffrent say comment

# 🙏🏻Acknowledgements:

This repository uses library from the following site:

1. [IronTCL](https://www.irontcl.com/)
