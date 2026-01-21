# RAMViewer 用户手册

## 目录

1. [简介](#1-简介)
2. [系统要求](#2-系统要求)
3. [安装](#3-安装)
4. [快速入门](#4-快速入门)
5. [CLI命令参考](#5-cli命令参考)
6. [交互模式](#6-交互模式)
7. [GUI模式](#7-gui模式)
8. [变量语法](#8-变量语法)
9. [MCU库集成](#9-mcu库集成)
10. [通信协议](#10-通信协议)
11. [使用示例](#11-使用示例)
12. [故障排除](#12-故障排除)

---

## 1. 简介

### 1.1 概述

RAMViewer (ramgs) 是一款基于串口通信的MCU RAM读写工具，专为嵌入式系统调试和开发设计。开发者可以通过串口连接实时读写MCU内存变量。

### 1.2 主要特性

- **实时变量访问**: 通过变量名读写MCU RAM变量
- **符号解析**: 使用ELF调试符号自动解析变量名到内存地址
- **复杂类型支持**: 支持结构体、数组、位字段和枚举
- **批量操作**: 单条命令读写多个变量
- **周期监控**: 可配置间隔的连续变量监控
- **GUI应用**: 功能完整的图形界面，支持实时图表
- **轻量级MCU库**: 最小RAM占用(约560字节)

### 1.3 架构

```
+----------------+      串口         +------------------+
|   PC (ramgs)   | <------------->  |   MCU (库)        |
+----------------+                  +------------------+
        |                                   |
        v                                   v
  symbols.json                         RAM变量
  (从ELF文件生成)                     (MCU内存中)
```

### 1.4 工作流程

1. 编译带调试符号的MCU固件
2. 使用 `ramgs create` 从ELF文件提取符号
3. 使用 `ramgs open` 通过串口连接MCU
4. 使用 `ramgs get` 和 `ramgs set` 读写变量

---

## 2. 系统要求

### 2.1 PC端要求

| 组件 | 要求 |
|------|------|
| 操作系统 | Windows 10/11 (推荐64位) |
| Python | 3.8 或更高版本 |
| 依赖库 | click, pyserial, PySide6, pyqtgraph |
| 磁盘空间 | 约100 MB (含依赖) |

### 2.2 MCU端要求

| 组件 | 要求 |
|------|------|
| RAM | 约560字节用于库缓冲区 |
| UART | 任意带RX/TX中断的UART外设 |
| 编译器 | 任意C编译器 (GCC, IAR, Keil等) |

### 2.3 支持的ELF格式

- 标准ELF (含DWARF调试信息)
- Renesas RL78 (.abs)
- ARM Cortex-M (.elf, .axf)
- Microchip PIC (.elf)
- 通用GCC输出 (.out)

---

## 3. 安装

### 3.1 从源码安装

```bash
# 进入项目目录
cd mcu-terminal/ramgs

# 安装依赖
pip install -r requirements.txt

# 验证安装
python -m ramgs --version
```

### 3.2 构建独立可执行文件

```bash
cd mcu-terminal/ramgs

# 运行构建脚本
build.bat

# 输出: dist/ramgs.exe
```

### 3.3 作为Python包安装

```bash
cd mcu-terminal/ramgs

# 开发模式安装
pip install -e .

# 现在可以直接运行
ramgs --version
```

### 3.4 目录结构

```
mcu-terminal/
├── ramgs/                  # PC端CLI工具
│   ├── ramgs/              # Python包
│   │   ├── __init__.py
│   │   ├── __main__.py     # 入口点
│   │   ├── cli.py          # 命令行接口
│   │   ├── config.py       # 配置常量
│   │   ├── protocol.py     # 通信协议
│   │   ├── serial_manager.py
│   │   ├── state_manager.py
│   │   ├── symbol_resolver.py
│   │   ├── type_converter.py
│   │   ├── variable_parser.py
│   │   ├── chart.py        # 实时图表显示
│   │   ├── repl/           # 交互模式
│   │   │   ├── __init__.py
│   │   │   ├── repl.py     # REPL主循环
│   │   │   ├── session.py  # 会话管理
│   │   │   ├── commands.py # 命令处理
│   │   │   └── completer.py# 自动补全
│   │   └── gui/            # GUI模式
│   │       ├── __init__.py
│   │       ├── main_window.py      # 主应用窗口
│   │       ├── connection_panel.py # 串口连接控件
│   │       ├── symbol_manager.py   # 符号文件管理
│   │       ├── variable_list.py    # 变量树形视图
│   │       ├── variable_config_dialog.py # 变量设置对话框
│   │       ├── curve_list.py       # 监控曲线列表
│   │       ├── chart_widget.py     # 实时图表(pyqtgraph)
│   │       ├── data_collector.py   # 后台采样线程
│   │       └── project_manager.py  # 工程保存/加载
│   ├── requirements.txt
│   ├── setup.py
│   └── build.bat
├── mcu_lib/                # MCU端C库
│   ├── inc/
│   │   └── ramviewer.h
│   └── src/
│       └── ramviewer.c
├── elfsymbol/              # ELF符号提取工具
│   └── elfsym.exe
├── test/                   # 测试文件
│   ├── test.abs
│   └── symbols.json
└── docs/                   # 文档
    ├── user_manual.md
    └── user_manual_zh.md
```

---

## 4. 快速入门

### 4.1 步骤1: 生成符号文件

首先从ELF文件提取符号:

```bash
ramgs create firmware.elf
```

这将创建包含所有全局变量及其地址和类型的 `symbols.json` 文件。

### 4.2 步骤2: 连接MCU

打开串口连接:

```bash
ramgs open --name COM3 --baud 115200
```

### 4.3 步骤3: 读取变量

读取单个变量:

```bash
ramgs get temperature
```

读取多个变量:

```bash
ramgs get temperature,pressure,humidity
```

### 4.4 步骤4: 写入变量

设置单个变量:

```bash
ramgs set motor_speed=1500
```

设置多个变量:

```bash
ramgs set motor_speed=1500,direction=1,enabled=true
```

### 4.5 步骤5: 关闭连接

```bash
ramgs close
```

---

## 5. CLI命令参考

### 5.1 全局选项

```bash
ramgs [选项] 命令 [参数]...

选项:
  --version  显示版本并退出
  --help     显示帮助信息并退出
```

### 5.2 open - 连接串口

**语法:**
```bash
ramgs open --name <端口> [--baud <波特率>] [--endian <字节序>]
```

**选项:**

| 选项 | 必需 | 默认值 | 描述 |
|------|------|--------|------|
| `--name` | 是 | - | 串口名称 (COM1, COM3等) |
| `--baud` | 否 | 9600 | 波特率 |
| `--endian` | 否 | little | 字节序: `little` 或 `big` |

**示例:**
```bash
# 基本连接
ramgs open --name COM3 --baud 9600

# 高速连接
ramgs open --name COM3 --baud 115200

# 大端MCU
ramgs open --name COM3 --baud 9600 --endian big
```

**说明:**
- 连接状态保存到 `~/.ramgs/state.json`
- 后续命令使用保存的连接设置

---

### 5.3 close - 断开连接

**语法:**
```bash
ramgs close
```

**示例:**
```bash
ramgs close
# 输出: Disconnected from COM3
```

---

### 5.4 create - 生成符号文件

**语法:**
```bash
ramgs create <ELF文件> [-o <输出文件>]
```

**参数:**

| 参数 | 描述 |
|------|------|
| `ELF文件` | 带调试符号的ELF/AXF/ABS文件路径 |

**选项:**

| 选项 | 默认值 | 描述 |
|------|--------|------|
| `-o, --output` | symbols.json | 输出文件路径 |

**示例:**
```bash
# 默认输出
ramgs create firmware.elf

# 自定义输出路径
ramgs create firmware.elf -o project_symbols.json
```

**说明:**
- 需要 `elfsymbol/elfsym.exe` 存在
- ELF文件必须包含DWARF调试信息

---

### 5.5 get - 读取变量

**语法:**
```bash
ramgs get [选项] <变量列表>
```

**参数:**

| 参数 | 描述 |
|------|------|
| `变量列表` | 逗号分隔的变量名列表 |

**选项:**

| 选项 | 描述 |
|------|------|
| `-i, --interval <毫秒>` | 读取间隔(毫秒) |
| `-c, --count <次数>` | 读取次数 (0 = 无限) |

**输出格式:**
```
var1=value1,var2=value2,var3=value3
```

**示例:**

```bash
# 单个变量
ramgs get temperature
# 输出: temperature=25.5

# 多个变量
ramgs get temperature,pressure,humidity
# 输出: temperature=25.5,pressure=101325,humidity=65

# 周期读取 (每100ms读一次，共10次)
ramgs get --interval 100 --count 10 temperature

# 连续监控 (每500ms读一次，直到Ctrl+C)
ramgs get --interval 500 --count 0 temperature,pressure

# 简写形式
ramgs get -i 100 -c 50 sensor_value
```

**执行模式:**

| 间隔 | 次数 | 行为 |
|------|------|------|
| 未设置 | 未设置 | 单次读取 |
| 已设置 | 未设置 | 无限循环 (按ESC停止) |
| 已设置 | 0 | 无限循环 (按ESC停止) |
| 已设置 | N>0 | 读取N次 (可按ESC提前停止) |

---

### 5.6 set - 写入变量

**语法:**
```bash
ramgs set [选项] <赋值列表>
```

**参数:**

| 参数 | 描述 |
|------|------|
| `赋值列表` | 逗号分隔的 var=value 对 |

**选项:**

| 选项 | 描述 |
|------|------|
| `-i, --interval <毫秒>` | 写入间隔(毫秒) |
| `-c, --count <次数>` | 写入次数 (0 = 无限) |

**值格式:**

| 格式 | 示例 | 描述 |
|------|------|------|
| 十进制 | `123` | 整数值 |
| 十六进制 | `0xFF` | 十六进制值 |
| 二进制 | `0b1010` | 二进制值 |
| 浮点数 | `3.14` | 浮点值 |
| 布尔值 | `true`/`false` | 布尔值 |

**示例:**

```bash
# 单个变量
ramgs set motor_speed=1500

# 多个变量
ramgs set motor_speed=1500,direction=1,enabled=true

# 十六进制值
ramgs set register=0xFF

# 周期写入 (每100ms写一次，共50次)
ramgs set --interval 100 --count 50 test_counter=1

# 连续写入
ramgs set --interval 100 --count 0 heartbeat=1
```

---

### 5.7 ports - 列出串口

**语法:**
```bash
ramgs ports
```

**示例:**
```bash
ramgs ports
# 输出:
# COM1: Communications Port (COM1)
# COM3: USB Serial Device (COM3)
# COM4: Arduino Uno (COM4)
```

---

### 5.8 status - 显示连接状态

**语法:**
```bash
ramgs status
```

**示例:**
```bash
ramgs status
# 输出:
# Status: Connected
#   Port: COM3
#   Baud: 115200
#   Endian: little
#   Symbols: C:\Projects\symbols.json
```

---

### 5.9 chart - 显示实时图表

**语法:**
```bash
ramgs chart [选项] <变量列表>
```

**描述:**

在图形窗口中实时显示变量值的图表。此命令可用于可视化变量随时间的变化。

**参数:**

| 参数 | 描述 |
|------|------|
| `变量列表` | 逗号分隔的变量名列表 (最多8个变量) |

**选项:**

| 选项 | 必需 | 描述 |
|------|------|------|
| `-i, --interval <毫秒>` | 是 | 采样间隔(毫秒) |
| `-c, --count <次数>` | 否 | 采样次数 (0 = 无限, 默认: 0) |

**示例:**

```bash
# 以100ms间隔绘制单个变量
ramgs chart counter -i 100

# 绘制多个变量
ramgs chart temperature,pressure -i 50

# 采集200个样本后停止
ramgs chart speed,position -i 100 -c 200
```

**说明:**
- 最多可同时绘制8个变量
- 按ESC键停止数据采集 (图表窗口保持打开以便分析)
- 关闭图表窗口返回命令提示符
- 需要matplotlib依赖 (已包含在requirements.txt中)

---

### 5.10 image - 生成静态图表图片

**语法:**
```bash
ramgs image [选项] <变量列表>
```

**描述:**

采集变量数据并生成静态PNG图表图片。与 `chart` 命令显示实时交互窗口不同，此命令生成文件，可用于报告、自动化或后续处理。

**参数:**

| 参数 | 描述 |
|------|------|
| `变量列表` | 逗号分隔的变量名列表 (最多8个变量) |

**选项:**

| 选项 | 必需 | 描述 |
|------|------|------|
| `-i, --interval <毫秒>` | 是 | 采样间隔(毫秒) |
| `-c, --count <次数>` | 是 | 采样次数 (必须 > 0, 不允许无限) |

**输出:**
- 图片文件保存到当前工作目录下的 `ramgs_tmp_imgs/` 文件夹
- 文件名格式: `image_<变量名>_YYYYMMDD_HHMMSS_fff.png`
- 生成图片的完整路径输出到标准输出

**示例:**

```bash
# 以100ms间隔生成50个样本的图片
ramgs image counter -i 100 -c 50
# 输出: Collected 50 samples. Image: C:\myproject\ramgs_tmp_imgs\image_counter_20260116_143052_123.png
# C:\myproject\ramgs_tmp_imgs\image_counter_20260116_143052_123.png

# 多个变量
ramgs image temperature,pressure -i 50 -c 100

# 在自动化脚本中使用
for /f "delims=" %i in ('ramgs image sensor -i 100 -c 50') do set IMG_PATH=%i
```

**与 `chart` 命令的区别:**

| 特性 | chart | image |
|------|-------|-------|
| 显示方式 | 实时GUI窗口 | 静态PNG文件 |
| 交互功能 | 暂停、滚动、导出 | 无 |
| -c 选项 | 可选 (默认: 无限) | 必须 (必须 > 0) |
| 输出 | 窗口 | 文件路径 |
| 用途 | 实时监控 | 自动化、报告 |

**说明:**
- 按ESC键可提前中断数据采集 (仍会用已采集的数据生成图片)
- 如果 `ramgs_tmp_imgs/` 目录不存在会自动创建
- 单张图片最多包含8个变量

---

### 5.11 gui - 启动GUI应用

**语法:**
```bash
ramgs gui
```

**描述:**

启动功能完整的图形用户界面应用。GUI为MCU变量监控提供了集成环境，支持实时图表可视化。

**功能特性:**

| 功能 | 描述 |
|------|------|
| 连接面板 | 选择串口、波特率、字节序；连接/断开 |
| 符号管理 | 从JSON文件加载符号或从ELF文件生成 |
| 变量浏览器 | 层次树形视图显示所有符号，支持搜索过滤 |
| 曲线列表 | 添加/移除监控变量，切换显示/隐藏 |
| 变量配置 | 设置标注、缩放系数、数组索引、曲线颜色 |
| 实时图表 | 高性能滚动图表，使用pyqtgraph |
| 工具栏 | 采样频率、开始/停止、保存CSV、打开/保存工程 |
| 工程文件 | 保存/加载所有设置到JSON工程文件 |

**示例:**

```bash
# 启动GUI应用
ramgs gui
```

**说明:**
- 需要PySide6和pyqtgraph依赖 (已包含在requirements.txt中)
- GUI与CLI状态独立运行
- 工程文件(.ramproj)存储连接设置、已加载符号和监控变量

---

## 6. 交互模式

RAMViewer 提供交互式REPL (Read-Eval-Print Loop) 模式，方便实时调试和变量操作。

### 6.1 进入交互模式

不带任何子命令运行 `ramgs` 进入交互模式:

```bash
ramgs
```

**输出:**
```
RAMViewer Interactive Mode
Type /help for available commands, /quit to exit

ramgs>
```

### 6.2 功能特性

- **命令历史**: 自动保存到 `~/.ramgs/history`，跨会话保留
- **自动补全**: Tab键补全命令和变量名
- **历史建议**: 输入时显示历史记录建议
- **状态恢复**: 启动时自动恢复之前的连接设置
- **快捷语法**: 无需命令前缀即可快速访问变量

### 6.3 交互命令

交互模式中所有命令以 `/` 开头:

| 命令 | 描述 |
|------|------|
| `/help` | 显示帮助信息 |
| `/quit`, `/exit` | 退出交互模式 (保留状态供下次会话使用) |
| `/quit -f` | 强制退出 (关闭连接, 清除状态) |
| `/ports` | 列出可用串口 |
| `/status` | 显示连接和符号状态 |
| `/open` | 打开串口连接 |
| `/close` | 关闭串口连接 |
| `/create <elf>` | 从ELF文件生成symbols.json |
| `/load <file>` | 加载符号文件 |
| `/get <vars>` | 读取变量值 |
| `/set <assigns>` | 写入变量值 |
| `/chart <vars>` | 显示实时图表 |
| `/image <vars>` | 生成静态图表图片 |

### 6.4 快捷语法

交互模式支持常用操作的快捷方式:

```bash
# 读取变量 (等同于 /get counter)
ramgs> counter
counter=42

# 写入变量 (等同于 /set counter=0)
ramgs> counter=0
OK

# 读取结构体成员
ramgs> sensor.temperature
sensor.temperature=25.5

# 多个变量
ramgs> temp,pressure,humidity
temp=25.5,pressure=101325,humidity=65
```

### 6.5 交互模式示例

**基本会话:**
```bash
$ ramgs
RAMViewer Interactive Mode
Type /help for available commands, /quit to exit

ramgs> /open --name COM3 --baud 115200
Connected to COM3 at 115200 baud (little-endian)

[COM3] > /load symbols.json
Loaded 750 symbols from symbols.json

[COM3] > counter
counter=42

[COM3] > counter=100
OK

[COM3] > counter
counter=100

[COM3] > /get -i 500 temperature
Press ESC to stop...
temperature=25.3
temperature=25.4
temperature=25.5

Stopped by user (ESC)

[COM3] > /chart counter -i 100
Chart window opened. Press ESC to stop data collection...
...

[COM3] > /quit
Goodbye! (state preserved for next session)
```

**说明:**
- 连接后提示符变为 `[COMx] >`
- 使用 Ctrl+D 或 `/quit` 退出 (状态保留, 下次会话自动恢复)
- 使用 `/quit -f` 强制退出 (清除状态)
- 按 Ctrl+C 取消当前输入 (不会退出)
- 按 ESC 键可停止重复操作

---

## 7. GUI模式

RAMViewer 提供功能完整的图形用户界面，用于变量监控和实时图表可视化。通过 `ramgs gui` 启动。

### 7.1 主窗口布局

GUI窗口分为以下区域：

```
+------------------------------------------------------------------+
|  工具栏 (采样频率、开始/停止、CSV、工程打开/保存)                   |
+------------------+-----------------------------------------------+
|  连接            |                                               |
|  面板            |                                               |
+------------------+            实时图表区域                        |
|  符号            |                                               |
|  管理器          |           (基于pyqtgraph)                      |
+------------------+                                               |
|  变量            |                                               |
|  列表            +-----------------------------------------------+
|  (树形视图)      |  曲线列表 (监控变量)                            |
+------------------+-----------------------------------------------+
|  状态栏                                                          |
+------------------------------------------------------------------+
```

### 7.2 连接面板

连接面板用于管理串口连接：

| 控件 | 描述 |
|------|------|
| 端口下拉框 | 从可用串口中选择 |
| 波特率 | 选择波特率 (9600, 115200等) |
| 字节序 | 选择小端或大端 |
| 连接按钮 | 连接或断开MCU |

### 7.3 符号管理器

符号管理器处理符号文件操作：

| 按钮 | 功能 |
|------|------|
| 加载符号 | 加载已有的symbols.json文件 |
| 从ELF生成 | 使用elfsym.exe从ELF文件创建symbols.json |
| 符号计数 | 显示已加载的符号数量 |

### 7.4 变量浏览器

变量列表以层次树形结构显示所有符号：

- **文件**: 顶层节点表示源文件
- **变量**: 第二层显示每个文件中的全局变量
- **成员**: 嵌套层级显示结构体成员和数组元素

**功能特性：**
- 搜索过滤器按名称查找变量
- 双击将变量添加到监控列表
- 右键菜单进行变量操作

### 7.5 变量配置

添加变量到监控时，可以配置：

| 设置 | 描述 |
|------|------|
| 标注 | 曲线的自定义显示名称 |
| 缩放系数 | 乘以原始值 (例如 0.001 将mV转换为V) |
| 数组索引 | 对于数组变量，指定监控哪个元素 |
| 曲线颜色 | 选择图表上的线条颜色 |

### 7.6 曲线列表

曲线列表显示当前所有监控的变量：

| 控件 | 描述 |
|------|------|
| 复选框 | 切换图表上曲线的可见性 |
| 颜色指示器 | 显示曲线的线条颜色 |
| 变量名 | 显示名称（或自定义标注） |
| 当前值 | 最新采样值 |
| 移除按钮 | 停止监控此变量 |

### 7.7 实时图表

图表区域提供高性能实时可视化：

**功能特性：**
- 平滑滚动显示
- 鼠标平移和缩放
- 多条曲线不同颜色
- 可配置时间窗口

**控制方式：**
- 鼠标拖动：平移视图
- 滚轮：放大/缩小
- 右键：重置视图

### 7.8 工具栏

工具栏提供常用操作的快速访问：

| 按钮 | 功能 |
|------|------|
| 采样频率 | 设置数据采集间隔(ms) |
| 开始 | 开始从MCU采集数据 |
| 停止 | 暂停数据采集 |
| 保存CSV | 导出采集的数据到CSV文件 |
| 打开工程 | 加载已保存的工程配置 |
| 保存工程 | 保存当前配置到工程文件 |

### 7.9 工程文件

工程文件(.ramproj)存储完整的工作配置：

- 连接设置（端口、波特率、字节序）
- 符号文件路径
- 监控变量列表及其设置
- 图表配置

**示例工作流：**

1. 启动GUI：`ramgs gui`
2. 配置连接并加载符号
3. 添加变量到监控并配置颜色/缩放
4. 保存工程：文件 → 保存工程
5. 下次使用：文件 → 打开工程 恢复所有设置

### 7.10 使用示例

```
1. 启动GUI：
   $ ramgs gui

2. 在连接面板中：
   - 从端口下拉框选择 "COM3"
   - 设置波特率为 "115200"
   - 点击 "连接"
   - 状态栏显示："已连接到 COM3"

3. 在符号管理器中：
   - 点击 "加载符号"
   - 选择 "symbols.json"
   - 状态显示："已加载 750 个符号"

4. 在变量列表中：
   - 展开 "main.c" → 找到 "temperature"
   - 双击添加到监控
   - 配置：标注="温度(°C)"，缩放=0.1，颜色=红色

5. 根据需要添加更多变量

6. 在工具栏中：
   - 设置采样频率为 100ms
   - 点击 "开始"
   - 图表开始显示实时数据

7. 保存设置：
   - 文件 → 另存工程为...
   - 保存为 "motor_debug.ramproj"
```

---

## 8. 变量语法

### 8.1 简单变量

通过名称访问全局变量:

```bash
ramgs get counter
ramgs set counter=100
```

### 8.2 结构体成员

使用点号访问结构体成员:

```bash
# 对于结构体: sensor_data.temperature
ramgs get sensor_data.temperature
ramgs set sensor_data.temperature=25.5

# 嵌套结构体
ramgs get system.config.timeout
```

### 8.3 数组元素

使用方括号访问数组元素:

```bash
# 对于数组: buffer[10]
ramgs get buffer[0]
ramgs set buffer[5]=0xFF

# 多维数组
ramgs get matrix[2][3]
```

### 8.4 组合访问

组合结构体和数组访问:

```bash
# 结构体数组
ramgs get sensors[0].temperature

# 包含数组的结构体
ramgs get config.thresholds[2]

# 复杂路径
ramgs get system.sensors[0].readings[5]
```

### 8.5 文件指定

当多个文件定义相同变量名时，使用 `@filename`:

```bash
# 获取 main.c 中的 'counter'
ramgs get counter@main

# 获取 uart.c 中的 'buffer'
ramgs get buffer@uart

# 结合成员访问
ramgs get sensor.value@sensors
```

### 8.6 变量名规则

| 规则 | 有效 | 无效 |
|------|------|------|
| 字母数字+下划线 | `my_var`, `var123` | `my-var`, `123var` |
| 区分大小写 | `Temp` != `temp` | - |
| 不含空格 | `my_var` | `my var` |

---

## 9. MCU库集成

### 9.1 需要的文件

将以下文件复制到MCU项目:

```
mcu_lib/
├── inc/
│   └── ramviewer.h
└── src/
    └── ramviewer.c
```

### 9.2 配置选项

在 `ramviewer.h` 中配置或通过编译器定义:

```c
/* 最大负载大小 (默认: 256) */
#define RV_MAX_PAYLOAD_SIZE     256

/* 每条命令最大变量数 (默认: 30) */
#define RV_MAX_VARIABLES        30
```

**内存使用:**

| 组件 | 大小(字节) |
|------|------------|
| 接收缓冲区 | RV_MAX_PAYLOAD_SIZE + 7 |
| 发送缓冲区 | RV_MAX_PAYLOAD_SIZE + 7 |
| 状态变量 | 约20 |
| **总计** | **约560** (默认配置) |

### 9.3 基本集成

```c
#include "ramviewer.h"

/* 你的UART发送字节函数 */
void uart_send_byte(uint8_t byte)
{
    /* 等待TX缓冲区就绪 */
    while (!(UART_STATUS & TX_READY));
    UART_DATA = byte;
}

/* UART接收中断处理函数 */
void UART_RX_IRQHandler(void)
{
    uint8_t byte = UART_DATA;
    rv_rx_byte(byte);
}

/* UART发送完成中断处理函数 */
void UART_TX_IRQHandler(void)
{
    rv_tx_complete();
}

int main(void)
{
    /* 初始化硬件 */
    uart_init(9600);

    /* 初始化RAMViewer */
    rv_init(uart_send_byte);

    /* 使能UART中断 */
    enable_uart_interrupts();

    /* 主循环 */
    while (1) {
        /* 你的应用代码 */
    }
}
```

### 9.4 平台特定示例

#### STM32 (HAL)

```c
#include "ramviewer.h"
#include "stm32f4xx_hal.h"

extern UART_HandleTypeDef huart2;
static uint8_t rx_byte;

void uart_send_byte(uint8_t byte)
{
    HAL_UART_Transmit_IT(&huart2, &byte, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart2) {
        rv_rx_byte(rx_byte);
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart2) {
        rv_tx_complete();
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_USART2_UART_Init();

    rv_init(uart_send_byte);
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);

    while (1) {
        /* 应用代码 */
    }
}
```

#### Arduino

```cpp
#include "ramviewer.h"

void uart_send_byte(uint8_t byte)
{
    Serial.write(byte);
}

void setup()
{
    Serial.begin(9600);
    rv_init(uart_send_byte);
}

void loop()
{
    while (Serial.available()) {
        rv_rx_byte(Serial.read());
    }

    /* 你的应用代码 */
}
```

#### Renesas RL78

```c
#include "ramviewer.h"
#include "r_cg_serial.h"

void uart_send_byte(uint8_t byte)
{
    R_UART0_Send(&byte, 1);
}

/* 在 r_cg_serial_user.c 中 */
void r_uart0_callback_receiveend(void)
{
    rv_rx_byte(g_uart0_rx_buffer[0]);
    R_UART0_Receive(&g_uart0_rx_buffer[0], 1);
}

void r_uart0_callback_sendend(void)
{
    rv_tx_complete();
}

void main(void)
{
    R_UART0_Start();
    rv_init(uart_send_byte);
    R_UART0_Receive(&g_uart0_rx_buffer[0], 1);

    while (1) {
        /* 应用代码 */
    }
}
```

### 9.5 API参考

#### rv_init

```c
void rv_init(rv_send_byte_fn send_fn);
```

初始化RAMViewer库。

| 参数 | 描述 |
|------|------|
| `send_fn` | 发送字节的回调函数 |

#### rv_rx_byte

```c
void rv_rx_byte(uint8_t byte);
```

处理接收到的字节。从UART接收中断调用。

| 参数 | 描述 |
|------|------|
| `byte` | 从UART接收的字节 |

#### rv_tx_complete

```c
void rv_tx_complete(void);
```

处理发送完成事件。从UART发送完成中断调用。

#### rv_is_tx_busy

```c
bool rv_is_tx_busy(void);
```

检查是否正在发送。

| 返回值 | 描述 |
|--------|------|
| `true` | 发送中 |
| `false` | 空闲 |

#### rv_get_version

```c
const char* rv_get_version(void);
```

获取库版本字符串。

---

## 10. 通信协议

### 10.1 帧格式

```
+------+------+------+-------+----------+------+------+
| SOF  | LEN  | CMD  | SEQ   | PAYLOAD  | CRC_L| CRC_H|
+------+------+------+-------+----------+------+------+
| 1B   | 2B   | 1B   | 1B    | 0-256B   | 1B   | 1B   |
```

| 字段 | 大小 | 描述 |
|------|------|------|
| SOF | 1字节 | 帧起始标志: `0xAA` |
| LEN | 2字节 | 负载长度 (小端) |
| CMD | 1字节 | 命令类型 |
| SEQ | 1字节 | 序列号 (0-255) |
| PAYLOAD | 0-256字节 | 命令数据 |
| CRC | 2字节 | CRC16-CCITT (小端) |

### 10.2 命令类型

| 代码 | 名称 | 方向 | 描述 |
|------|------|------|------|
| 0x01 | READ_VAR | PC -> MCU | 读变量请求 |
| 0x02 | WRITE_VAR | PC -> MCU | 写变量请求 |
| 0x81 | READ_RESP | MCU -> PC | 读响应 |
| 0x82 | WRITE_RESP | MCU -> PC | 写响应 |
| 0xFF | ERROR | MCU -> PC | 错误响应 |
| 0x10 | PING | PC -> MCU | 心跳探测 |
| 0x90 | PONG | MCU -> PC | 心跳响应 |

### 10.3 读变量命令 (0x01)

**请求负载:**

```
+-------+----------+----------+-----+
| COUNT | VAR_INFO | VAR_INFO | ... |
+-------+----------+----------+-----+
| 1B    | 8B       | 8B       |     |
```

**VAR_INFO结构 (8字节):**

```
+--------+------+---------+----------+
| ADDR   | SIZE | BIT_OFF | BIT_SIZE |
+--------+------+---------+----------+
| 4B     | 2B   | 1B      | 1B       |
```

| 字段 | 大小 | 描述 |
|------|------|------|
| ADDR | 4字节 | 内存地址 (小端) |
| SIZE | 2字节 | 变量字节大小 |
| BIT_OFF | 1字节 | 位偏移 (0xFF = 非位字段) |
| BIT_SIZE | 1字节 | 位大小 (0xFF = 非位字段) |

**响应负载:**

```
+-------+--------+--------+-----+
| COUNT | DATA   | DATA   | ... |
+-------+--------+--------+-----+
| 1B    | 变长   | 变长   |     |
```

### 10.4 写变量命令 (0x02)

**请求负载:**

```
+-------+-------+-------+-----+
| COUNT | ENTRY | ENTRY | ... |
+-------+-------+-------+-----+
| 1B    | 8B+N  | 8B+N  |     |
```

**ENTRY结构:**

```
+--------+------+---------+----------+------+
| ADDR   | SIZE | BIT_OFF | BIT_SIZE | DATA |
+--------+------+---------+----------+------+
| 4B     | 2B   | 1B      | 1B       | SIZE |
```

**响应负载:**

```
+--------+
| STATUS |
+--------+
| 1B     |
```

### 10.5 错误码

| 代码 | 名称 | 描述 |
|------|------|------|
| 0x00 | ERR_OK | 成功 |
| 0x01 | ERR_CRC | CRC校验失败 |
| 0x02 | ERR_ADDR | 无效地址 |
| 0x03 | ERR_SIZE | 无效大小 |
| 0x04 | ERR_CMD | 未知命令 |
| 0x05 | ERR_TIMEOUT | 操作超时 |

### 10.6 CRC16-CCITT

```
多项式: 0x1021
初始值: 0xFFFF
输入: 从SOF到PAYLOAD末尾的所有字节
```

**C语言实现:**

```c
uint16_t crc16_ccitt(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    while (len--) {
        crc ^= (uint16_t)(*data++) << 8;
        for (int i = 0; i < 8; i++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return crc;
}
```

### 10.7 时序参数

| 参数 | 值 |
|------|-----|
| 响应超时 | 500 ms |
| 最大重试次数 | 3 |
| 重试延迟 | 立即 |

---

## 11. 使用示例

### 11.1 温度监控

连续监控温度传感器:

```bash
# 连接
ramgs open --name COM3 --baud 115200

# 每500ms监控一次温度
ramgs get --interval 500 --count 0 temperature

# 输出 (直到Ctrl+C):
# temperature=25.3
# temperature=25.4
# temperature=25.5
# ...
```

### 11.2 电机控制

控制电机速度和方向:

```bash
# 设置电机参数
ramgs set motor.enabled=true,motor.speed=1500,motor.direction=1

# 读回确认
ramgs get motor.enabled,motor.speed,motor.direction
# 输出: motor.enabled=true,motor.speed=1500,motor.direction=1
```

### 11.3 传感器阵列

读取传感器阵列值:

```bash
# 读取各个传感器
ramgs get sensors[0].value,sensors[1].value,sensors[2].value

# 监控特定传感器
ramgs get --interval 100 --count 100 sensors[0].value
```

### 11.4 配置更新

更新设备配置:

```bash
# 写入新配置
ramgs set config.sample_rate=1000
ramgs set config.filter_enabled=true
ramgs set config.threshold=100

# 验证
ramgs get config.sample_rate,config.filter_enabled,config.threshold
```

### 11.5 调试输出脚本

创建批处理调试脚本:

```batch
@echo off
REM debug_monitor.bat

echo Starting debug monitor...
echo Press Ctrl+C to stop

ramgs open --name COM3 --baud 115200

:loop
ramgs get system.state,error_code,uptime_ms
timeout /t 1 >nul
goto loop
```

### 11.6 实时图表可视化

在图表窗口中可视化变量变化:

```bash
# 连接MCU
ramgs open --name COM3 --baud 115200

# 绘制单个变量
ramgs chart temperature -i 100

# 绘制多个变量进行对比
ramgs chart speed,target_speed -i 50

# 精确采集500个样本用于分析
ramgs chart sensor_value -i 20 -c 500
```

使用交互模式:

```bash
$ ramgs
ramgs> /open --name COM3 --baud 115200
Connected to COM3 at 115200 baud (little-endian)

[COM3] > /chart counter,timer -i 100
Chart window opened. Press ESC to stop data collection...
...

Data collection stopped. Chart window remains open for analysis.
Close the chart window to continue...
```

---

## 12. 故障排除

### 12.1 常见错误

#### "Not connected. Use 'ramgs open' first."

**原因:** 没有活动的串口连接。

**解决方案:**
```bash
ramgs open --name COM3 --baud 9600
```

#### "Failed to open COMx: Access denied"

**原因:** 端口被其他应用程序占用。

**解决方案:**
1. 关闭其他使用该端口的应用程序
2. 在设备管理器中检查端口冲突
3. 尝试不同的COM端口

#### "Variable not found: xxx"

**原因:** 变量名不在symbols.json中。

**解决方案:**
1. 检查变量名拼写
2. 重新生成符号: `ramgs create firmware.elf`
3. 检查变量是否为全局变量 (不支持局部变量)
4. 使用文件指定符: `variable@filename`

#### "Timeout: No response from MCU"

**原因:** MCU未响应命令。

**解决方案:**
1. 确认MCU固件包含RAMViewer库
2. 检查UART连接 (TX/RX)
3. 确认波特率与MCU设置匹配
4. 检查MCU是否在运行 (非复位/休眠状态)

#### "CRC mismatch"

**原因:** 传输过程中数据损坏。

**解决方案:**
1. 检查线缆连接
2. 降低波特率
3. 检查电气噪声
4. 确认正确接地

### 12.2 调试技巧

#### 验证串口

```bash
# 列出可用端口
ramgs ports

# 检查连接状态
ramgs status
```

#### 使用简单变量测试

先从简单的全局变量开始测试，再尝试复杂结构:

```bash
# 简单整数
ramgs get counter

# 在尝试
ramgs get complex_struct.nested.array[5].member
```

#### 检查符号文件

验证符号文件包含你的变量:

```bash
# 打开symbols.json搜索你的变量
# 或使用Python:
python -c "import json; d=json.load(open('symbols.json')); print([s['name'] for s in d['symbols'] if 'your_var' in s['name']])"
```

### 12.3 MCU端调试

在MCU代码中添加调试输出:

```c
/* 在 rv_process_frame() 中 */
static void rv_process_frame(void)
{
    /* 调试: 收到帧时切换LED */
    LED_TOGGLE();

    /* ... 函数其余部分 */
}
```

### 12.4 协议分析

使用串口监视器捕获原始数据:

```
PC -> MCU: AA 09 00 01 01 01 06 A0 FF 00 04 00 FF FF [CRC]
           ^  ^     ^  ^  ^  ^                       ^
           |  |     |  |  |  +-- 地址: 0x00FFA006    |
           |  |     |  |  +-- 数量: 1                |
           |  |     |  +-- 序列号: 1                 |
           |  |     +-- 命令: READ_VAR               |
           |  +-- 长度: 9                            |
           +-- SOF                                   +-- CRC16
```

---

## 附录A: symbols.json格式

```json
{
  "schemaVersion": "1.0",
  "toolVersion": "1.0.0.0",
  "exportTime": "2024-01-15T10:30:00Z",
  "sourceElfFile": "firmware.elf",
  "totalSymbols": 150,
  "symbols": [
    {
      "name": "counter",
      "dataType": "uint32_t",
      "baseDataType": "unsigned int",
      "sizeInBytes": 4,
      "memoryAddress": "0x20001000",
      "sourceFile": "main.c"
    },
    {
      "name": "sensor_data",
      "dataType": "SensorData",
      "baseDataType": "<struct>",
      "sizeInBytes": 12,
      "memoryAddress": "0x20001004",
      "sourceFile": "sensors.c",
      "isStruct": true,
      "members": [
        {
          "name": "temperature",
          "dataType": "float",
          "sizeInBytes": 4,
          "memoryAddress": "0x20001004",
          "memberOffset": 0
        }
      ]
    }
  ]
}
```

---

## 附录B: 版本历史

| 版本 | 日期 | 变更 |
|------|------|------|
| 1.3.0 | 2026-01 | 添加功能完整的GUI模式，支持实时图表 |
| 1.2.0 | 2026-01 | 添加image命令，生成静态图表图片 |
| 1.1.0 | 2026-01 | 添加交互模式(REPL)，实时图表命令 |
| 1.0.0 | 2024-01 | 初始发布 |

---

*文档版本: 1.3.0*
*最后更新: 2026-01*
