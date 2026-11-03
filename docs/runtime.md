# 开发说明

## 模块边界

| 模块 | 职责 |
| --- | --- |
| `assets.*` | 目录和 ZIP 资源读取、路径规范化、本地化覆盖 |
| `runtime.*`、`runtime_skip.cpp` | Lua 接口、对象状态、场景切换、游戏事件与活动协调 |
| `hob.*`、`hob_feedback.*` | 找物目标、工具条件、拾取反馈和奖励过渡 |
| 各小游戏的 `.cpp` / `.hpp` | 活动规则、操作状态、完成条件与呈现数据 |
| `gui.*`、`animation*`、`collada.*`、`particles.*` | 布局和动画数据、模型与粒子计算 |
| `checkpoint.cpp`、`activity_checkpoints.cpp` | 世界与活动状态序列化 |
| `profiles.*`、`game_saves.*`、`settings.*` | 用户档案、进度文件与设置持久化 |
| `player.cpp`、`player_skip.cpp` | SDL 绘制、输入、音频及跳过界面 |
| `player_menu.cpp` | 菜单、暂停、用户切换与安全保存协调 |
| `main.cpp` | 命令行、资源发现与应用生命周期 |

CMake 的 `deadtime_core` 仅依赖 libzip、pugixml 和 LuaJIT。`deadtime_player` 在核心上接入 SDL，最终入口为 `deadtime`。新增源文件显式列入所属目标，避免目录扫描意外把临时程序编入游戏。

## 代码风格

C++ 使用四空格缩进、110 列换行和仓库根目录的 `.clang-format`。调整代码后执行以下命令。

```sh
clang-format -i src/*.cpp src/*.hpp
```

游戏状态变化放在核心或相应活动模型中，展示层将输入转换为操作并渲染结果。新增逻辑应优先使用已有资源解析、动画和存档接口。保留原 Lua 的行为与素材编码，修改原资源时需要确认本地化覆盖和相关回调。

## 资源与脚本

资源可从 `assets/` 目录直接读取，也可以读取构建生成的 `resources.pak` 与 `localization.pak`。本地化层覆盖同名资源。查找统一大小写与路径分隔符，拒绝越界路径、符号链接及大小写冲突。

`.loc` 描述场景与对象，`.hob` 描述找物目标及工具要求，Lua 以状态段组织对象行为。每个对象状态使用独立环境，切换时调用进入与离开回调。场景切换在当前回调结束后处理。计时器触发前清除，回调可以重新设定。

Lua 仅开放游戏接口及 math 库。未支持的接口报告错误。图片通常由 RGB JPEG 与同名 `.a` 灰度 JPEG 合成，透明度和交互遮罩分别计算。

## 保存与恢复

`Runtime` 保存场景、物品、对象状态、Lua 环境、活动进度和随机状态。恢复时直接重建状态，避免重复执行首次访问与奖励回调。用户使用独立 ID 关联进度文件，写入通过临时文件和替换完成。

正常游戏在安全状态定期保存，也在暂停、失焦与退出时请求保存。未确认奖励、拖动和过渡动画期间等待安全状态后再落盘。菜单切换用户时重新加载相应世界。渲染缓存、声音设备和系统窗口由展示层管理。

## Windows 发布

在 MSYS2 UCRT64 终端安装依赖。

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-pkgconf \
  mingw-w64-ucrt-x86_64-libzip mingw-w64-ucrt-x86_64-pugixml \
  mingw-w64-ucrt-x86_64-luajit mingw-w64-ucrt-x86_64-SDL2 \
  mingw-w64-ucrt-x86_64-SDL2_image mingw-w64-ucrt-x86_64-SDL2_ttf \
  mingw-w64-ucrt-x86_64-SDL2_mixer mingw-w64-ucrt-x86_64-nsis
bash tools/package-windows.sh
```

脚本编译 Release、审计资源、收集运行库和许可，使用同一份载荷生成 portable 与 installer。默认构建目录是 `build/windows-release`，发布目录是 `dist/windows`。可通过三个位置参数指定资源目录、构建目录和发布目录，后两者必须位于仓库的 `build/` 与 `dist/` 下。

虚拟机安装前先创建快照。在普通 Windows 用户的 PowerShell 中可运行发布验证，输出目录必须尚不存在。

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File tools\verify-windows-release.ps1 `
  -Portable dist\windows\DeadTimeStories-0.1.0-windows-x64-portable.zip `
  -Installer dist\windows\DeadTimeStories-0.1.0-windows-x64-installer.exe `
  -OutputDirectory .local\release-verification
```

验证会检查启动渲染、两个包的载荷一致性、快捷方式、注册项、卸载及保留用户文件。它使用后台 SDL 驱动，前台操作与声音需要单独验证。

## macOS 发布

Apple Silicon 的打包入口如下，使用 Homebrew 安装的依赖及 Xcode Command Line Tools。

```sh
bash tools/package-macos.sh
```

输出为 `dist/macos/DeadTimeStories-0.1.0-macos-arm64-installer.pkg`，安装到 `/Applications/Deadtime Stories.app`。当前依赖的最低系统版本为 macOS 27.0。打包会复制第三方动态库、修正加载路径、收集许可与配方信息，并将游戏资源放入应用包。SDL2 兼容库动态加载的 SDL3 也一同封装。

应用使用 ad-hoc 签名以验证包内代码完整性。PKG 没有 Developer ID Installer 签名，也未经过 Apple 公证。下载后如被系统拦截，可在系统设置的隐私与安全性页面允许打开。

## 检查入口

```sh
./build/release/deadtime --assets assets --audit
./build/release/deadtime --assets assets --menu
./build/release/deadtime --assets assets --hob zones/jessie_home/hob_plants.hob
./build/release/deadtime --assets assets --minigame ouiji_board
```

`--help` 列出全部参数。独立场景、找物、小游戏和过场检查不写入普通用户进度；`--frames` 与 `--snapshot` 可用于有限帧渲染。资源校验和后台渲染不能替代前台通关及与原版的画面对照。

构建产物放在 `build/` 和 `dist/`，下载、逆向数据库、临时记录与截图放在 `.local/`，这些目录由 Git 忽略。
