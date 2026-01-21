# RAMViewer (ramgs)

**让 AI 真正理解硬件 —— 打通 AIGC 与嵌入式系统的最后一公里**
> **观看演示视频**：https://www.bailing.design/video/

本仓库只包含MCU端协议实现代码。ramgs.exe命令行工具请在release页面下载；下载后解压缩并将ramgs.exe所在的目录添加到环境变量path中。

[English](#english) | [中文](#中文)

---

<a name="中文"></a>

## 为什么需要 RAMViewer？

### AIGC 在嵌入式开发中的困境

当前 AI 辅助编程工具在不同领域的应用效果存在巨大差异：

| 开发领域 | AI 采纳程度 | 原因 |
|:---:|:---:|:---|
| Web 前端开发 | ★★★★★ 优秀 | 完整的反馈回路：代码→运行→结果→优化 |
| 后端服务开发 | ★★★★☆ 良好 | AI 可直接运行和测试代码 |
| **嵌入式开发** | **★★☆☆☆ 有限** | **反馈回路阻塞：AI 无法与硬件交互** |

**核心问题**：在嵌入式开发中，AI 只能生成代码，却无法：
- 验证代码是否正确编译
- 观测程序在真实硬件上的运行状态
- 读取传感器数据、显示内容
- 模拟按键输入、旋钮操作
- 实现"生成-测试-修正"的自动化闭环

### RAMViewer 的解决方案

**RAMViewer** 通过命令行工具打通反馈回路，让 AI 能够与真实硬件进行交互：

```
需求描述 → AI 代码生成 → 自动编译 → 自动烧录 → 自动测试 → 结果反馈 → 迭代优化
                                                    ↑
                                              RAMViewer
```

通过操作与硬件状态关联的变量，AI 可以间接实现与真实物理硬件的交互：

| 操作类型 | 实现方式 |
|:---|:---|
| 模拟按键输入 | 写入按键状态变量 |
| 模拟旋钮操作 | 修改编码器计数值 |
| 读取显示内容 | 获取显示缓冲区数据 |
| 注入传感器数据 | 写入 ADC 采样值变量 |
| 观测输出状态 | 读取 GPIO 输出寄存器 |

> **观看演示视频**：https://www.bailing.design/video/

---

## 特性

### 极致轻量的 MCU 端库

| 特性 | 说明 |
|:---|:---|
| 程序侵入性 | 极低，仅需 3 行初始化代码 |
| RAM 占用 | 约 560 字节（可配置） |
| Flash 占用 | 约 1KB |
| 硬件依赖 | 仅需一个串口 |

### 强大的 PC 端工具

- **CLI 模式**：命令行操作，完美适配 AI 工具调用
- **交互模式**：REPL 环境，支持自动补全和历史记录
- **GUI 模式**：图形界面，实时曲线显示

### 核心能力

- **变量名访问**：通过变量名直接读写 MCU 内存
- **符号解析**：自动从 ELF 文件提取变量信息
- **复杂类型支持**：结构体、数组、位域、枚举
- **批量操作**：单命令读写多个变量
- **周期监控**：可配置间隔的持续监控
- **实时图表**：数据可视化和趋势分析

---

## 快速开始

### 安装

```bash
在release页面下载；下载后解压缩并将ramgs.exe所在的目录添加到环境变量path中。
```

### MCU 端集成

将 `mcu_lib/` 目录下的文件添加到你的 MCU 项目：

```c
#include "ramviewer.h"

// 你的串口发送函数
void uart_send_byte(uint8_t byte) {
    UART_DATA = byte;
}

// 串口接收中断
void UART_RX_IRQHandler(void) {
    rv_rx_byte(UART_DATA);
}

// 串口发送完成中断
void UART_TX_IRQHandler(void) {
    rv_tx_complete();
}

int main(void) {
    rv_init(uart_send_byte);  // 仅需一行初始化
    // ... 你的应用代码
}
```

### 基本使用

```bash
# 1. 从 ELF 文件生成符号表
ramgs create firmware.elf

# 2. 连接串口
ramgs open --name COM3 --baud 115200

# 3. 读取变量
ramgs get temperature
# 输出: temperature=25.5

# 4. 写入变量
ramgs set motor_speed=1500

# 5. 周期监控
ramgs get --interval 100 temperature,pressure

# 6. 实时图表
ramgs chart temperature -i 100
```

---

## 与 AI 工具集成

RAMViewer 的设计目标是与 AI 编程助手无缝集成。以下是与主流 AI 工具的集成方式：

### Claude Code / Cursor / GitHub Copilot

AI 可以直接调用 ramgs 命令来与你的 MCU 交互：

```bash
# AI 读取当前温度值
ramgs get sensor.temperature
# 返回: sensor.temperature=25.5

# AI 模拟按键按下
ramgs set button_pressed=1

# AI 检查 PID 控制器输出
ramgs get pid.output,pid.error,pid.integral
```

### MCP (Model Context Protocol) 集成

RAMViewer 可配置为 MCP 工具，使 AI 具备硬件交互能力。

### 自动化测试场景

```bash
# AI 自动执行测试用例
ramgs set test_input=100
ramgs get test_output
# AI 分析结果并迭代优化
```

---

## 应用场景

### 1. AI 辅助调试

让 AI 读取运行时变量，分析程序状态，定位问题根源。

### 2. 自动化测试

AI 自动生成测试用例，执行测试，验证结果，生成报告。

### 3. 端到端开发

完整的自动化流程：需求 → 代码生成 → 编译 → 烧录 → 测试 → 反馈 → 优化。

### 4. 实时监控

GUI 模式提供实时曲线显示，适合长时间运行监控和数据分析。

---

## 架构

```
+----------------+      Serial      +------------------+
|   PC (ramgs)   | <-------------> |   MCU (library)  |
+----------------+                  +------------------+
        |                                   |
        v                                   v
  symbols.json                        RAM Variables
  (from ELF file)                    (in MCU memory)
```

### 项目结构

```
ramviewer/
├── ramgs/              # PC 端 Python 工具
│   ├── ramgs/
│   │   ├── cli.py          # 命令行接口
│   │   ├── protocol.py     # 通信协议
│   │   ├── symbol_resolver.py
│   │   ├── repl/           # 交互模式
│   │   └── gui/            # 图形界面
│   └── requirements.txt
├── mcu_lib/            # MCU 端 C 库
│   ├── inc/ramviewer.h
│   └── src/ramviewer.c
├── elfsymbol/          # ELF 符号提取工具
│   └── elfsym.exe
└── docs/               # 文档
    ├── user_manual.md
    └── user_manual_zh.md
```

---

## 支持的平台

### MCU 平台

- STM32 (HAL/LL)
- Renesas RL78
- Arduino
- 任何支持 UART 的 MCU

### ELF 格式

- 标准 ELF + DWARF 调试信息
- Renesas RL78 (.abs)
- ARM Cortex-M (.elf, .axf)
- Microchip PIC (.elf)
- GCC 输出 (.out)

---

## 文档

- [用户手册 (English)](docs/user_manual.md)
- [用户手册 (中文)](docs/user_manual_zh.md)
- [通信协议](docs/user_manual.md#10-communication-protocol)
- [MCU 集成指南](docs/user_manual.md#9-mcu-library-integration)

---

## 参与贡献

我们欢迎各种形式的贡献！

### 贡献方式

1. **报告 Bug**：提交 Issue 描述问题
2. **功能建议**：分享你的想法和需求
3. **代码贡献**：提交 Pull Request
4. **文档改进**：完善文档和示例
5. **MCU 适配**：添加对新 MCU 平台的支持

---

## 许可证

本项目采用 MIT 许可证。详见 [LICENSE](LICENSE) 文件。

---

## 致谢

感谢所有为嵌入式 AI 开发做出贡献的开发者们！

---

<a name="english"></a>

# RAMViewer (ramgs)

**Empowering AI to Truly Understand Hardware — Bridging the Last Mile Between AIGC and Embedded Systems**

## Why RAMViewer?

### The AIGC Dilemma in Embedded Development

Current AI programming assistants show vastly different effectiveness across development domains:

| Domain | AI Adoption | Reason |
|:---:|:---:|:---|
| Web Frontend | ★★★★★ Excellent | Complete feedback loop: code→run→result→optimize |
| Backend Services | ★★★★☆ Good | AI can directly run and test code |
| **Embedded** | **★★☆☆☆ Limited** | **Feedback loop blocked: AI cannot interact with hardware** |

**Core Problem**: In embedded development, AI can only generate code but cannot:
- Verify if code compiles correctly
- Observe program behavior on real hardware
- Read sensor data or display content
- Simulate button presses or knob rotations
- Achieve automated "generate-test-fix" cycles

### RAMViewer's Solution

**RAMViewer** unblocks the feedback loop through CLI tools, enabling AI to interact with real hardware:

```
Requirements → AI Code Gen → Compile → Flash → Test → Feedback → Iterate
                                                  ↑
                                            RAMViewer
```

By manipulating variables associated with hardware states, AI can indirectly interact with physical hardware:

| Operation | Implementation |
|:---|:---|
| Simulate button press | Write to button state variable |
| Simulate knob rotation | Modify encoder count value |
| Read display content | Get display buffer data |
| Inject sensor data | Write to ADC sample variable |
| Observe output state | Read GPIO output register |

> **Watch Demo Videos**: https://www.bailing.design/video/

---

## Features

### Ultra-lightweight MCU Library

| Feature | Description |
|:---|:---|
| Code Intrusiveness | Minimal, only 3 lines of init code |
| RAM Usage | ~560 bytes (configurable) |
| Flash Usage | ~1KB |
| Hardware Dependency | Only requires one UART |

### Powerful PC Tool

- **CLI Mode**: Command-line operation, perfect for AI tool invocation
- **Interactive Mode**: REPL environment with auto-completion
- **GUI Mode**: Graphical interface with real-time charting

### Core Capabilities

- **Variable Name Access**: Read/write MCU memory by variable name
- **Symbol Resolution**: Auto-extract variable info from ELF files
- **Complex Type Support**: Structs, arrays, bitfields, enums
- **Batch Operations**: Read/write multiple variables in one command
- **Periodic Monitoring**: Continuous monitoring with configurable intervals
- **Real-time Charts**: Data visualization and trend analysis

---

## Quick Start

### Installation

```bash
# Clone repository
git clone https://github.com/your-repo/ramviewer.git
cd ramviewer/ramgs

# Install dependencies
pip install -r requirements.txt

# Or build standalone executable
build.bat  # Output: dist/ramgs.exe
```

### MCU Integration

Add files from `mcu_lib/` to your MCU project:

```c
#include "ramviewer.h"

void uart_send_byte(uint8_t byte) {
    UART_DATA = byte;
}

void UART_RX_IRQHandler(void) {
    rv_rx_byte(UART_DATA);
}

void UART_TX_IRQHandler(void) {
    rv_tx_complete();
}

int main(void) {
    rv_init(uart_send_byte);  // Just one line of init
    // ... your application code
}
```

### Basic Usage

```bash
# 1. Generate symbol table from ELF
ramgs create firmware.elf

# 2. Connect to serial port
ramgs open --name COM3 --baud 115200

# 3. Read variable
ramgs get temperature
# Output: temperature=25.5

# 4. Write variable
ramgs set motor_speed=1500

# 5. Periodic monitoring
ramgs get --interval 100 temperature,pressure

# 6. Real-time chart
ramgs chart temperature -i 100
```

---

## AI Tool Integration

RAMViewer is designed for seamless integration with AI programming assistants.

### Claude Code / Cursor / GitHub Copilot

AI can directly invoke ramgs commands to interact with your MCU:

```bash
# AI reads current temperature
ramgs get sensor.temperature

# AI simulates button press
ramgs set button_pressed=1

# AI checks PID controller output
ramgs get pid.output,pid.error,pid.integral
```

---

## Contributing

We welcome contributions of all kinds!

- **Bug Reports**: Submit issues describing problems
- **Feature Requests**: Share your ideas and needs
- **Code Contributions**: Submit pull requests
- **Documentation**: Improve docs and examples
- **MCU Ports**: Add support for new MCU platforms

---

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

---

**Star this project if you find it useful!**

*让 AI 成为嵌入式开发的真正助手 / Making AI a true assistant for embedded development*
