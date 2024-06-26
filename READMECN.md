
# MetaTCL<img src="img/icon.png" align="right" width="120"/>

MetaTCL是一个为GoldSrc游戏控制台加入TCL脚本环境的MetaHookSV插件


<!-- badges: start -->


![Downloads](https://img.shields.io/github/downloads/DrAbcOfficial/metatcl/total?style=for-the-badge)
![Repo Size](https://img.shields.io/github/repo-size/DrAbcOfficial/metatcl?style=for-the-badge)
![Last Commit](https://img.shields.io/github/last-commit/DrAbcOfficial/metatcl?style=for-the-badge)

<!-- badges: end -->

# 🖥️ 安装
1. 下载并安装 [MetahookSV](https://github.com/hzqst/MetaHookSv)
2. 安装MetaHookSV
3. 将自Action或Realse下载的压缩包解压至 `(mod)` 文件夹, 列如: `C:/SteamLibrary/Half-life/halflife/`
4. 打开 `svencoop/metahook/configs/plugins.lst`, 另起一行添加 `Metatcl.dll`
5. 打开 `svencoop/metahook/configs/dllpaths.lst`, 另起一行添加 `tcl86t` 

# 🏗️ 构建
1. 克隆或下载 [MetahookSV](https://github.com/hzqst/MetaHookSv)
2. 克隆或下载本储存库
3. 复制 `Metatcl-master` 文件夹至 `MetaHookSv-master/Plugins`

    自动构建:

    1. 使用Powershell运行 `build-Metatcl.ps1`
   
    手动构建:
    1. 使用`git submodule update --init --recursive`初始化tcl库
    2. 切换至`tcl/win`文件夹
    3. 使用`nmake -f makefile.vc all`构建tcl模块，需注意应先使用`vcvarsamd64_x86.bat`(64位系统)或`vcvars32.bat`(32位系统)初始化环境
    4. 使用`namek -f makefile.vc install INSTALLDIR="../../tclib"`安装tclib
    5. 打开 `sln` 然后构建
 
 ----

 # 🎭控制台命令
 |命令|格式|帮助|
 |---|---|---|
 |s_tcl_eval|s_tcl_eval [tcl语句]|执行一句tcl语句|
 |s_tcl_exec|s_tcl_exec [tcl文件] <参数1> <参数2> <参数3>……|在`(mod)`文件夹和`(mod)/tcl`文件夹中执行对应的tcl文件，|
 |s_tcl_reset|s_tcl_reset|重置tcl虚拟机环境|

 # 🪡全局TCL函数
 |函数|格式|帮助|
 |---|---|---|
 |clientcmd|clientcmd [cmd]|执行一次客户端命令|
 |servercmd|servercmd [cmd]|执行一次服务端命令|
 |getcvar|getcvar [cvar]|获取一次cvar数值|
 |gethudinfo|gethudinfo [index]|获取指定玩家的hudinfo|
 |getlocalinfo|getlocalinfo|获取本地玩家的entityinfo|
 |gs_*|多种|原控制台命令可以添加gs_前缀后直接调用，如: 控制台中的`maps`命令可以在TCL文件中使用`gs_maps`进行调用

# 🔍 注意事项
1. `s_tcl_eval`将会在虚拟器上连续解释语句（类似tclsh环境），如需清理先前定义的变量，需使用`tcl_reset`

2. `s_tcl_exec`将会自动重置虚拟机

3. `s_tcl_exec`参数将会以`argv`为变量名，以数组全局变量的形式传入文件

4. 所有的命令都以`s_`开头, 什么鬼？  这是一种阻止“绝对善良”的服务器“帮助”你执行一些“绝对保证无害”的TCL脚本的方法。

# 📬使用例

1. 
控制台输入 
```
s_tcl_eval "set a Hello "

s_tcl_eval "set b Gordon"

s_tcl_eval "puts [$a + $b]"
```
将会在控制台输出：`[TCL] Hello Gordon`

2. 将以下内容在`halflife/tcl`文件夹中保存为`开关准心.tcl`

```tcl
#!/usr/bin/tclsh

#读取cvar `crosshair`的值，储存进变量cvar_c中
set cvar_c [getcvar crosshair]

#根据该值判断，以此切换准心开关并输出一条聊天内容
if { $cvar_c > 0 } {
    gs_say "我关闭了准心！"
    clientcmd crosshair 0
} else {
    gs_say "我开启了准心！"
    clientcmd crosshair 1
}
```

在控制台中输入`s_tcl_exec 开关准心`，你将看见准心切换时会输出一条不同的聊天
# 🙏🏻鸣谢:

本储存库使用来自于以下站点的运行库:

1. [IronTCL](https://www.irontcl.com/)
