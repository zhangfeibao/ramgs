# RAMViewer User Manual

## Table of Contents

1. [Introduction](#1-introduction)
2. [System Requirements](#2-system-requirements)
3. [Installation](#3-installation)
4. [Quick Start](#4-quick-start)
5. [CLI Command Reference](#5-cli-command-reference)
6. [Interactive Mode](#6-interactive-mode)
7. [GUI Mode](#7-gui-mode)
8. [Variable Syntax](#8-variable-syntax)
9. [MCU Library Integration](#9-mcu-library-integration)
10. [Communication Protocol](#10-communication-protocol)
11. [Examples](#11-examples)
12. [Troubleshooting](#12-troubleshooting)
13. [API Reference](#13-api-reference)

---

## 1. Introduction

### 1.1 Overview

RAMViewer (ramgs) is a serial-based MCU RAM read/write tool designed for embedded systems debugging and development. It allows developers to read and write MCU memory variables in real-time through a serial port connection.

### 1.2 Key Features

- **Real-time Variable Access**: Read and write MCU RAM variables by name
- **Symbol Resolution**: Automatically resolves variable names to memory addresses using ELF debug symbols
- **Complex Type Support**: Supports structures, arrays, bitfields, and enums
- **Batch Operations**: Read/write multiple variables in a single command
- **Periodic Monitoring**: Continuous variable monitoring with configurable intervals
- **GUI Application**: Full-featured graphical interface with realtime charts
- **Cross-platform**: Works on Windows (with Linux/macOS support possible)
- **Lightweight MCU Library**: Minimal RAM footprint (~560 bytes)

### 1.3 Architecture

```
+----------------+      Serial      +------------------+
|   PC (ramgs)   | <-------------> |   MCU (library)  |
+----------------+                  +------------------+
        |                                   |
        v                                   v
  symbols.json                        RAM Variables
  (from ELF file)                    (in MCU memory)
```

### 1.4 Workflow

1. Compile your MCU firmware with debug symbols
2. Use `ramgs create` to extract symbols from ELF file
3. Connect to MCU via serial port using `ramgs open`
4. Read/write variables using `ramgs get` and `ramgs set`

---

## 2. System Requirements

### 2.1 PC Requirements

| Component | Requirement |
|-----------|-------------|
| OS | Windows 10/11 (64-bit recommended) |
| Python | 3.8 or higher |
| Dependencies | click, pyserial, PySide6, pyqtgraph |
| Disk Space | ~100 MB (with dependencies) |

### 2.2 MCU Requirements

| Component | Requirement |
|-----------|-------------|
| RAM | ~560 bytes for library buffers |
| UART | Any UART peripheral with RX/TX interrupts |
| Compiler | Any C compiler (GCC, IAR, Keil, etc.) |

### 2.3 Supported ELF Formats

- Standard ELF with DWARF debug info
- Renesas RL78 (.abs)
- ARM Cortex-M (.elf, .axf)
- Microchip PIC (.elf)
- Generic GCC output (.out)

---

## 3. Installation

### 3.1 Install from Source

```bash
# Clone or download the project
cd mcu-terminal/ramgs

# Install dependencies
pip install -r requirements.txt

# Verify installation
python -m ramgs --version
```

### 3.2 Build Standalone Executable

```bash
cd mcu-terminal/ramgs

# Run build script
build.bat

# Output: dist/ramgs.exe
```

### 3.3 Install as Python Package

```bash
cd mcu-terminal/ramgs

# Install in development mode
pip install -e .

# Now you can run directly
ramgs --version
```

### 3.4 Directory Structure

```
mcu-terminal/
├── ramgs/                  # PC CLI tool
│   ├── ramgs/              # Python package
│   │   ├── __init__.py
│   │   ├── __main__.py     # Entry point
│   │   ├── cli.py          # Command line interface
│   │   ├── config.py       # Configuration constants
│   │   ├── protocol.py     # Communication protocol
│   │   ├── serial_manager.py
│   │   ├── state_manager.py
│   │   ├── symbol_resolver.py
│   │   ├── type_converter.py
│   │   ├── variable_parser.py
│   │   ├── chart.py        # Realtime chart display
│   │   ├── repl/           # Interactive mode
│   │   │   ├── __init__.py
│   │   │   ├── repl.py     # REPL main loop
│   │   │   ├── session.py  # Session management
│   │   │   ├── commands.py # Command handlers
│   │   │   └── completer.py# Auto-completion
│   │   └── gui/            # GUI mode
│   │       ├── __init__.py
│   │       ├── main_window.py      # Main application window
│   │       ├── connection_panel.py # Serial connection widget
│   │       ├── symbol_manager.py   # Symbol file management
│   │       ├── variable_list.py    # Variable tree view
│   │       ├── variable_config_dialog.py # Variable settings dialog
│   │       ├── curve_list.py       # Monitored curves list
│   │       ├── chart_widget.py     # Realtime chart (pyqtgraph)
│   │       ├── data_collector.py   # Background sampling thread
│   │       └── project_manager.py  # Project save/load
│   ├── requirements.txt
│   ├── setup.py
│   └── build.bat
├── mcu_lib/                # MCU C library
│   ├── inc/
│   │   └── ramviewer.h
│   └── src/
│       └── ramviewer.c
├── elfsymbol/              # ELF symbol extractor tool
│   └── elfsym.exe
├── test/                   # Test files
│   ├── test.abs
│   └── symbols.json
└── docs/                   # Documentation
    ├── user_manual.md
    └── user_manual_zh.md
```

---

## 4. Quick Start

### 4.1 Step 1: Generate Symbol File

First, extract symbols from your ELF file:

```bash
ramgs create firmware.elf
```

This creates `symbols.json` containing all global variables with their addresses and types.

### 4.2 Step 2: Connect to MCU

Open a serial port connection:

```bash
ramgs open --name COM3 --baud 115200
```

### 4.3 Step 3: Read Variables

Read a single variable:

```bash
ramgs get temperature
```

Read multiple variables:

```bash
ramgs get temperature,pressure,humidity
```

### 4.4 Step 4: Write Variables

Set a single variable:

```bash
ramgs set motor_speed=1500
```

Set multiple variables:

```bash
ramgs set motor_speed=1500,direction=1,enabled=true
```

### 4.5 Step 5: Close Connection

```bash
ramgs close
```

---

## 5. CLI Command Reference

### 5.1 Global Options

```bash
ramgs [OPTIONS] COMMAND [ARGS]...

Options:
  --version  Show version and exit
  --help     Show help message and exit
```

### 5.2 open - Connect to Serial Port

**Syntax:**
```bash
ramgs open --name <PORT> [--baud <RATE>] [--endian <ORDER>]
```

**Options:**

| Option | Required | Default | Description |
|--------|----------|---------|-------------|
| `--name` | Yes | - | Serial port name (COM1, COM3, etc.) |
| `--baud` | No | 9600 | Baud rate |
| `--endian` | No | little | Byte order: `little` or `big` |

**Examples:**
```bash
# Basic connection
ramgs open --name COM3 --baud 9600

# High-speed connection
ramgs open --name COM3 --baud 115200

# Big-endian MCU
ramgs open --name COM3 --baud 9600 --endian big
```

**Notes:**
- Connection state is saved to `~/.ramgs/state.json`
- Subsequent commands use the saved connection settings

---

### 5.3 close - Disconnect

**Syntax:**
```bash
ramgs close
```

**Example:**
```bash
ramgs close
# Output: Disconnected from COM3
```

---

### 5.4 create - Generate Symbol File

**Syntax:**
```bash
ramgs create <ELF_FILE> [-o <OUTPUT>]
```

**Arguments:**

| Argument | Description |
|----------|-------------|
| `ELF_FILE` | Path to ELF/AXF/ABS file with debug symbols |

**Options:**

| Option | Default | Description |
|--------|---------|-------------|
| `-o, --output` | symbols.json | Output file path |

**Examples:**
```bash
# Default output
ramgs create firmware.elf

# Custom output path
ramgs create firmware.elf -o project_symbols.json

# Absolute paths
ramgs create C:\Projects\firmware.elf -o C:\Projects\symbols.json
```

**Notes:**
- Requires `elfsymbol/elfsym.exe` to be present
- ELF file must contain DWARF debug information
- Output is a JSON file with all global variable symbols

---

### 5.5 get - Read Variables

**Syntax:**
```bash
ramgs get [OPTIONS] <VARIABLES>
```

**Arguments:**

| Argument | Description |
|----------|-------------|
| `VARIABLES` | Comma-separated list of variable names |

**Options:**

| Option | Description |
|--------|-------------|
| `-i, --interval <MS>` | Interval between reads in milliseconds |
| `-c, --count <N>` | Number of reads (0 = infinite) |

**Output Format:**
```
var1=value1,var2=value2,var3=value3
```

**Examples:**

```bash
# Single variable
ramgs get temperature
# Output: temperature=25.5

# Multiple variables
ramgs get temperature,pressure,humidity
# Output: temperature=25.5,pressure=101325,humidity=65

# Periodic reading (every 100ms, 10 times)
ramgs get --interval 100 --count 10 temperature

# Continuous monitoring (every 500ms, until Ctrl+C)
ramgs get --interval 500 --count 0 temperature,pressure

# Shorthand
ramgs get -i 100 -c 50 sensor_value
```

**Execution Modes:**

| Interval | Count | Behavior |
|----------|-------|----------|
| Not set | Not set | Single read |
| Set | Not set | Infinite loop (press ESC to stop) |
| Set | 0 | Infinite loop (press ESC to stop) |
| Set | N>0 | Read N times (can press ESC to stop early) |

---

### 5.6 set - Write Variables

**Syntax:**
```bash
ramgs set [OPTIONS] <ASSIGNMENTS>
```

**Arguments:**

| Argument | Description |
|----------|-------------|
| `ASSIGNMENTS` | Comma-separated list of var=value pairs |

**Options:**

| Option | Description |
|--------|-------------|
| `-i, --interval <MS>` | Interval between writes in milliseconds |
| `-c, --count <N>` | Number of writes (0 = infinite) |

**Value Formats:**

| Format | Example | Description |
|--------|---------|-------------|
| Decimal | `123` | Integer value |
| Hexadecimal | `0xFF` | Hex value |
| Binary | `0b1010` | Binary value |
| Float | `3.14` | Floating point |
| Boolean | `true`/`false` | Boolean value |

**Examples:**

```bash
# Single variable
ramgs set motor_speed=1500

# Multiple variables
ramgs set motor_speed=1500,direction=1,enabled=true

# Hex values
ramgs set register=0xFF

# Periodic write (every 100ms, 50 times)
ramgs set --interval 100 --count 50 test_counter=1

# Continuous write
ramgs set --interval 100 --count 0 heartbeat=1
```

---

### 5.7 ports - List Serial Ports

**Syntax:**
```bash
ramgs ports
```

**Example:**
```bash
ramgs ports
# Output:
# COM1: Communications Port (COM1)
# COM3: USB Serial Device (COM3)
# COM4: Arduino Uno (COM4)
```

---

### 5.8 status - Show Connection Status

**Syntax:**
```bash
ramgs status
```

**Example:**
```bash
ramgs status
# Output:
# Status: Connected
#   Port: COM3
#   Baud: 115200
#   Endian: little
#   Symbols: C:\Projects\symbols.json
```

---

### 5.9 chart - Display Realtime Chart

**Syntax:**
```bash
ramgs chart [OPTIONS] <VARIABLES>
```

**Description:**

Display realtime chart of variable values in a graphical window. This command is useful for visualizing variable changes over time.

**Arguments:**

| Argument | Description |
|----------|-------------|
| `VARIABLES` | Comma-separated list of variable names (max 8 variables) |

**Options:**

| Option | Required | Description |
|--------|----------|-------------|
| `-i, --interval <MS>` | Yes | Sampling interval in milliseconds |
| `-c, --count <N>` | No | Number of samples (0 = infinite, default: 0) |

**Examples:**

```bash
# Chart a single variable at 100ms interval
ramgs chart counter -i 100

# Chart multiple variables
ramgs chart temperature,pressure -i 50

# Collect 200 samples then stop
ramgs chart speed,position -i 100 -c 200
```

**Notes:**
- Maximum 8 variables can be charted simultaneously
- Press ESC to stop data collection (chart window remains open for analysis)
- Close the chart window to return to command prompt
- Requires matplotlib dependency (included in requirements.txt)

---

### 5.10 image - Generate Static Chart Image

**Syntax:**
```bash
ramgs image [OPTIONS] <VARIABLES>
```

**Description:**

Collect variable data and generate a static PNG chart image. Unlike the `chart` command which shows a realtime interactive window, this command produces a file that can be used for reports, automation, or further processing.

**Arguments:**

| Argument | Description |
|----------|-------------|
| `VARIABLES` | Comma-separated list of variable names (max 8 variables) |

**Options:**

| Option | Required | Description |
|--------|----------|-------------|
| `-i, --interval <MS>` | Yes | Sampling interval in milliseconds |
| `-c, --count <N>` | Yes | Number of samples (must be > 0, infinite not allowed) |

**Output:**
- Image file is saved to `ramgs_tmp_imgs/` directory in the current working directory
- Filename format: `image_<varnames>_YYYYMMDD_HHMMSS_fff.png`
- The full path to the generated image is output to stdout

**Examples:**

```bash
# Generate image with 50 samples at 100ms interval
ramgs image counter -i 100 -c 50
# Output: Collected 50 samples. Image: C:\myproject\ramgs_tmp_imgs\image_counter_20260116_143052_123.png
# C:\myproject\ramgs_tmp_imgs\image_counter_20260116_143052_123.png

# Multiple variables
ramgs image temperature,pressure -i 50 -c 100

# Use in automation scripts
for /f "delims=" %i in ('ramgs image sensor -i 100 -c 50') do set IMG_PATH=%i
```

**Differences from `chart` command:**

| Feature | chart | image |
|---------|-------|-------|
| Display | Realtime GUI window | Static PNG file |
| Interaction | Pause, scroll, export | None |
| -c option | Optional (default: infinite) | Required (must be > 0) |
| Output | Window | File path |
| Use case | Realtime monitoring | Automation, reports |

**Notes:**
- Press ESC to interrupt data collection early (image is still generated with collected data)
- The `ramgs_tmp_imgs/` directory is created automatically if it doesn't exist
- Maximum 8 variables can be included in a single image

---

### 5.11 gui - Launch GUI Application

**Syntax:**
```bash
ramgs gui
```

**Description:**

Launch the full-featured graphical user interface application. The GUI provides an integrated environment for MCU variable monitoring with realtime chart visualization.

**Features:**

| Feature | Description |
|---------|-------------|
| Connection Panel | Select serial port, baud rate, endianness; connect/disconnect |
| Symbol Manager | Load symbols from JSON file or generate from ELF file |
| Variable Browser | Hierarchical tree view of all symbols with search filtering |
| Curve List | Add/remove variables to monitor, toggle visibility |
| Variable Config | Set label, scale factor, array index, curve color |
| Realtime Chart | High-performance scrolling chart using pyqtgraph |
| Toolbar | Sampling rate, start/stop, save CSV, open/save project |
| Project Files | Save/load all settings to JSON project files |

**Example:**

```bash
# Launch GUI application
ramgs gui
```

**Notes:**
- Requires PySide6 and pyqtgraph dependencies (included in requirements.txt)
- The GUI operates independently from CLI state
- Project files (.ramproj) store connection settings, loaded symbols, and monitored variables

---

## 6. Interactive Mode

RAMViewer provides an interactive REPL (Read-Eval-Print Loop) mode for convenient real-time debugging and variable manipulation.

### 6.1 Entering Interactive Mode

Run `ramgs` without any subcommand to enter interactive mode:

```bash
ramgs
```

**Output:**
```
RAMViewer Interactive Mode
Type /help for available commands, /quit to exit

ramgs>
```

### 6.2 Features

- **Command History**: Automatically saved to `~/.ramgs/history` and persisted across sessions
- **Auto-completion**: Tab completion for commands and variable names
- **Auto-suggest**: Suggestions from command history as you type
- **State Restoration**: Automatically restores previous connection settings on startup
- **Shortcut Syntax**: Quick variable access without command prefixes

### 6.3 Interactive Commands

All commands in interactive mode start with `/`:

| Command | Description |
|---------|-------------|
| `/help` | Show help message |
| `/quit`, `/exit` | Exit interactive mode (state preserved for next session) |
| `/quit -f` | Force exit (close connection, clear state) |
| `/ports` | List available serial ports |
| `/status` | Show connection and symbol status |
| `/open` | Open serial port connection |
| `/close` | Close serial port connection |
| `/create <elf>` | Generate symbols.json from ELF file |
| `/load <file>` | Load symbols file |
| `/get <vars>` | Read variable values |
| `/set <assigns>` | Write variable values |
| `/chart <vars>` | Display realtime chart |
| `/image <vars>` | Generate static chart image |

### 6.4 Shortcut Syntax

Interactive mode supports shortcuts for common operations:

```bash
# Reading a variable (equivalent to /get counter)
ramgs> counter
counter=42

# Writing a variable (equivalent to /set counter=0)
ramgs> counter=0
OK

# Reading struct members
ramgs> sensor.temperature
sensor.temperature=25.5

# Multiple variables
ramgs> temp,pressure,humidity
temp=25.5,pressure=101325,humidity=65
```

### 6.5 Interactive Mode Examples

**Basic Session:**
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

**Notes:**
- The prompt changes to `[COMx] >` when connected
- Use Ctrl+D or `/quit` to exit (state preserved, next session auto-restores)
- Use `/quit -f` to force exit (clears state)
- Press Ctrl+C to cancel current input (not to exit)
- Repeated operations can be stopped by pressing ESC

---

## 7. GUI Mode

RAMViewer provides a full-featured graphical user interface for variable monitoring and realtime chart visualization. Launch the GUI with `ramgs gui`.

### 7.1 Main Window Layout

The GUI window is organized into the following areas:

```
+------------------------------------------------------------------+
|  Toolbar (sampling rate, start/stop, CSV, project open/save)     |
+------------------+-----------------------------------------------+
|  Connection      |                                               |
|  Panel           |                                               |
+------------------+            Realtime Chart Area                |
|  Symbol          |                                               |
|  Manager         |           (pyqtgraph-based)                   |
+------------------+                                               |
|  Variable        |                                               |
|  List            +-----------------------------------------------+
|  (tree view)     |  Curve List (monitored variables)             |
+------------------+-----------------------------------------------+
|  Status Bar                                                      |
+------------------------------------------------------------------+
```

### 7.2 Connection Panel

The connection panel allows you to manage the serial port connection:

| Control | Description |
|---------|-------------|
| Port Dropdown | Select from available serial ports |
| Baud Rate | Choose baud rate (9600, 115200, etc.) |
| Endianness | Select little-endian or big-endian |
| Connect Button | Connect to or disconnect from MCU |

### 7.3 Symbol Manager

The symbol manager handles symbol file operations:

| Button | Function |
|--------|----------|
| Load Symbols | Load an existing symbols.json file |
| Generate from ELF | Create symbols.json from ELF file using elfsym.exe |
| Symbol Count | Shows number of loaded symbols |

### 7.4 Variable Browser

The variable list displays all symbols in a hierarchical tree structure:

- **Files**: Top-level nodes represent source files
- **Variables**: Second level shows global variables from each file
- **Members**: Nested levels for struct members and array elements

**Features:**
- Search filter to find variables by name
- Double-click to add a variable to the monitoring list
- Right-click context menu for variable operations

### 7.5 Variable Configuration

When adding a variable to monitor, you can configure:

| Setting | Description |
|---------|-------------|
| Label | Custom display name for the curve |
| Scale Factor | Multiply raw value (e.g., 0.001 for mV to V) |
| Array Index | For array variables, specify which element to monitor |
| Curve Color | Choose the line color on the chart |

### 7.6 Curve List

The curve list shows all currently monitored variables:

| Control | Description |
|---------|-------------|
| Checkbox | Toggle curve visibility on the chart |
| Color Indicator | Shows the curve's line color |
| Variable Name | Display name (or custom label) |
| Current Value | Latest sampled value |
| Remove Button | Stop monitoring this variable |

### 7.7 Realtime Chart

The chart area provides high-performance realtime visualization:

**Features:**
- Smooth scrolling display
- Pan and zoom with mouse
- Multiple curves with different colors
- Configurable time window

**Controls:**
- Mouse drag: Pan the view
- Scroll wheel: Zoom in/out
- Right-click: Reset view

### 7.8 Toolbar

The toolbar provides quick access to common operations:

| Button | Function |
|--------|----------|
| Sampling Rate | Set the data collection interval (ms) |
| Start | Begin collecting data from MCU |
| Stop | Pause data collection |
| Save CSV | Export collected data to CSV file |
| Open Project | Load a saved project configuration |
| Save Project | Save current configuration to project file |

### 7.9 Project Files

Project files (.ramproj) store your complete working configuration:

- Connection settings (port, baud rate, endianness)
- Symbol file path
- List of monitored variables with their settings
- Chart configuration

**Example workflow:**

1. Launch GUI: `ramgs gui`
2. Configure connection and load symbols
3. Add variables to monitor and configure colors/scales
4. Save project: File → Save Project
5. Next time: File → Open Project to restore everything

### 7.10 Example Session

```
1. Launch the GUI:
   $ ramgs gui

2. In Connection Panel:
   - Select "COM3" from port dropdown
   - Set baud rate to "115200"
   - Click "Connect"
   - Status bar shows: "Connected to COM3"

3. In Symbol Manager:
   - Click "Load Symbols"
   - Select "symbols.json"
   - Status shows: "Loaded 750 symbols"

4. In Variable List:
   - Expand "main.c" → find "temperature"
   - Double-click to add to monitoring
   - Configure: Label="Temp (°C)", Scale=0.1, Color=Red

5. Add more variables as needed

6. In Toolbar:
   - Set sampling rate to 100ms
   - Click "Start"
   - Chart begins showing realtime data

7. To save your setup:
   - File → Save Project As...
   - Save as "motor_debug.ramproj"
```

---

## 8. Variable Syntax

### 8.1 Simple Variables

Access global variables by name:

```bash
ramgs get counter
ramgs set counter=100
```

### 8.2 Structure Members

Access structure members using dot notation:

```bash
# For struct: sensor_data.temperature
ramgs get sensor_data.temperature
ramgs set sensor_data.temperature=25.5

# Nested structures
ramgs get system.config.timeout
```

### 8.3 Array Elements

Access array elements using bracket notation:

```bash
# For array: buffer[10]
ramgs get buffer[0]
ramgs set buffer[5]=0xFF

# Multi-dimensional arrays
ramgs get matrix[2][3]
```

### 8.4 Combined Access

Combine structure and array access:

```bash
# Array of structures
ramgs get sensors[0].temperature

# Structure containing array
ramgs get config.thresholds[2]

# Complex path
ramgs get system.sensors[0].readings[5]
```

### 8.5 File Specifier

When multiple files define the same variable name, use `@filename`:

```bash
# Get 'counter' from main.c
ramgs get counter@main

# Get 'buffer' from uart.c
ramgs get buffer@uart

# Combined with member access
ramgs get sensor.value@sensors
```

### 8.6 Variable Name Rules

| Rule | Valid | Invalid |
|------|-------|---------|
| Alphanumeric + underscore | `my_var`, `var123` | `my-var`, `123var` |
| Case sensitive | `Temp` != `temp` | - |
| No spaces | `my_var` | `my var` |

---

## 9. MCU Library Integration

### 9.1 Files to Include

Copy these files to your MCU project:

```
mcu_lib/
├── inc/
│   └── ramviewer.h
└── src/
    └── ramviewer.c
```

### 9.2 Configuration Options

Configure in `ramviewer.h` or via compiler defines:

```c
/* Maximum payload size (default: 256) */
#define RV_MAX_PAYLOAD_SIZE     256

/* Maximum variables per command (default: 30) */
#define RV_MAX_VARIABLES        30
```

**Memory Usage:**

| Component | Size (bytes) |
|-----------|--------------|
| RX Buffer | RV_MAX_PAYLOAD_SIZE + 7 |
| TX Buffer | RV_MAX_PAYLOAD_SIZE + 7 |
| State Variables | ~20 |
| **Total** | **~560** (default config) |

### 9.3 Basic Integration

```c
#include "ramviewer.h"

/* Your UART send byte function */
void uart_send_byte(uint8_t byte)
{
    /* Wait for TX buffer ready */
    while (!(UART_STATUS & TX_READY));
    UART_DATA = byte;
}

/* UART RX interrupt handler */
void UART_RX_IRQHandler(void)
{
    uint8_t byte = UART_DATA;
    rv_rx_byte(byte);
}

/* UART TX complete interrupt handler */
void UART_TX_IRQHandler(void)
{
    rv_tx_complete();
}

int main(void)
{
    /* Initialize hardware */
    uart_init(9600);

    /* Initialize RAMViewer */
    rv_init(uart_send_byte);

    /* Enable UART interrupts */
    enable_uart_interrupts();

    /* Main loop */
    while (1) {
        /* Your application code */
    }
}
```

### 9.4 Platform-Specific Examples

#### STM32 (HAL)

```c
#include "ramviewer.h"
#include "stm32f4xx_hal.h"

extern UART_HandleTypeDef huart2;

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
```

#### Arduino

```c
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

    /* Your application code */
}

void serialEvent()
{
    while (Serial.available()) {
        rv_rx_byte(Serial.read());
    }
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

/* In r_cg_serial_user.c */
void r_uart0_callback_receiveend(void)
{
    rv_rx_byte(g_uart0_rx_buffer[0]);
    R_UART0_Receive(&g_uart0_rx_buffer[0], 1);
}

void r_uart0_callback_sendend(void)
{
    rv_tx_complete();
}
```

### 9.5 API Reference

#### rv_init

```c
void rv_init(rv_send_byte_fn send_fn);
```

Initialize the RAMViewer library.

| Parameter | Description |
|-----------|-------------|
| `send_fn` | Callback function for sending a byte |

#### rv_rx_byte

```c
void rv_rx_byte(uint8_t byte);
```

Process a received byte. Call from UART RX interrupt.

| Parameter | Description |
|-----------|-------------|
| `byte` | Received byte from UART |

#### rv_tx_complete

```c
void rv_tx_complete(void);
```

Handle TX complete event. Call from UART TX interrupt.

#### rv_is_tx_busy

```c
bool rv_is_tx_busy(void);
```

Check if transmission is in progress.

| Return | Description |
|--------|-------------|
| `true` | TX in progress |
| `false` | TX idle |

#### rv_get_version

```c
const char* rv_get_version(void);
```

Get library version string.

---

## 10. Communication Protocol

### 10.1 Frame Format

```
+------+------+------+-------+----------+------+------+
| SOF  | LEN  | CMD  | SEQ   | PAYLOAD  | CRC_L| CRC_H|
+------+------+------+-------+----------+------+------+
| 1B   | 2B   | 1B   | 1B    | 0-256B   | 1B   | 1B   |
```

| Field | Size | Description |
|-------|------|-------------|
| SOF | 1 byte | Start of frame: `0xAA` |
| LEN | 2 bytes | Payload length (little-endian) |
| CMD | 1 byte | Command type |
| SEQ | 1 byte | Sequence number (0-255) |
| PAYLOAD | 0-256 bytes | Command-specific data |
| CRC | 2 bytes | CRC16-CCITT (little-endian) |

### 10.2 Command Types

| Code | Name | Direction | Description |
|------|------|-----------|-------------|
| 0x01 | READ_VAR | PC -> MCU | Read variable request |
| 0x02 | WRITE_VAR | PC -> MCU | Write variable request |
| 0x81 | READ_RESP | MCU -> PC | Read response |
| 0x82 | WRITE_RESP | MCU -> PC | Write response |
| 0xFF | ERROR | MCU -> PC | Error response |
| 0x10 | PING | PC -> MCU | Heartbeat ping |
| 0x90 | PONG | MCU -> PC | Heartbeat pong |

### 10.3 Read Variable Command (0x01)

**Request Payload:**

```
+-------+----------+----------+-----+
| COUNT | VAR_INFO | VAR_INFO | ... |
+-------+----------+----------+-----+
| 1B    | 8B       | 8B       |     |
```

**VAR_INFO Structure (8 bytes):**

```
+--------+------+---------+----------+
| ADDR   | SIZE | BIT_OFF | BIT_SIZE |
+--------+------+---------+----------+
| 4B     | 2B   | 1B      | 1B       |
```

| Field | Size | Description |
|-------|------|-------------|
| ADDR | 4 bytes | Memory address (little-endian) |
| SIZE | 2 bytes | Variable size in bytes |
| BIT_OFF | 1 byte | Bit offset (0xFF = not bitfield) |
| BIT_SIZE | 1 byte | Bit size (0xFF = not bitfield) |

**Response Payload:**

```
+-------+--------+--------+-----+
| COUNT | DATA   | DATA   | ... |
+-------+--------+--------+-----+
| 1B    | varies | varies |     |
```

### 10.4 Write Variable Command (0x02)

**Request Payload:**

```
+-------+-------+-------+-----+
| COUNT | ENTRY | ENTRY | ... |
+-------+-------+-------+-----+
| 1B    | 8B+N  | 8B+N  |     |
```

**ENTRY Structure:**

```
+--------+------+---------+----------+------+
| ADDR   | SIZE | BIT_OFF | BIT_SIZE | DATA |
+--------+------+---------+----------+------+
| 4B     | 2B   | 1B      | 1B       | SIZE |
```

**Response Payload:**

```
+--------+
| STATUS |
+--------+
| 1B     |
```

### 10.5 Error Codes

| Code | Name | Description |
|------|------|-------------|
| 0x00 | ERR_OK | Success |
| 0x01 | ERR_CRC | CRC mismatch |
| 0x02 | ERR_ADDR | Invalid address |
| 0x03 | ERR_SIZE | Invalid size |
| 0x04 | ERR_CMD | Unknown command |
| 0x05 | ERR_TIMEOUT | Operation timeout |

### 10.6 CRC16-CCITT

```
Polynomial: 0x1021
Initial Value: 0xFFFF
Input: All bytes from SOF to end of PAYLOAD
```

**C Implementation:**

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

### 10.7 Timing

| Parameter | Value |
|-----------|-------|
| Response Timeout | 500 ms |
| Max Retries | 3 |
| Retry Delay | Immediate |

---

## 11. Examples

### 11.1 Temperature Monitoring

Continuously monitor temperature sensor:

```bash
# Connect
ramgs open --name COM3 --baud 115200

# Monitor temperature every 500ms
ramgs get --interval 500 --count 0 temperature

# Output (until Ctrl+C):
# temperature=25.3
# temperature=25.4
# temperature=25.5
# ...
```

### 11.2 Motor Control

Control a motor with speed and direction:

```bash
# Set motor parameters
ramgs set motor.enabled=true,motor.speed=1500,motor.direction=1

# Read back
ramgs get motor.enabled,motor.speed,motor.direction
# Output: motor.enabled=true,motor.speed=1500,motor.direction=1

# Ramp up speed
for /L %i in (500,100,2000) do @ramgs set motor.speed=%i
```

### 11.3 Sensor Array

Read sensor array values:

```bash
# Read individual sensors
ramgs get sensors[0].value,sensors[1].value,sensors[2].value

# Monitor specific sensor
ramgs get --interval 100 --count 100 sensors[0].value
```

### 11.4 Configuration Update

Update device configuration:

```bash
# Write new configuration
ramgs set config.sample_rate=1000
ramgs set config.filter_enabled=true
ramgs set config.threshold=100

# Verify
ramgs get config.sample_rate,config.filter_enabled,config.threshold
```

### 11.5 Debug Output Script

Create a batch script for debugging:

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

### 11.6 Realtime Chart Visualization

Visualize variable changes in a chart window:

```bash
# Connect to MCU
ramgs open --name COM3 --baud 115200

# Chart a single variable
ramgs chart temperature -i 100

# Chart multiple variables for comparison
ramgs chart speed,target_speed -i 50

# Collect exactly 500 samples for analysis
ramgs chart sensor_value -i 20 -c 500
```

Using interactive mode:

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

## 12. Troubleshooting

### 12.1 Common Errors

#### "Not connected. Use 'ramgs open' first."

**Cause:** No active serial port connection.

**Solution:**
```bash
ramgs open --name COM3 --baud 9600
```

#### "Failed to open COMx: Access denied"

**Cause:** Port is in use by another application.

**Solution:**
1. Close other applications using the port
2. Check Device Manager for port conflicts
3. Try a different COM port

#### "Variable not found: xxx"

**Cause:** Variable name not in symbols.json.

**Solution:**
1. Verify variable name spelling
2. Regenerate symbols: `ramgs create firmware.elf`
3. Check if variable is global (local variables not supported)
4. Use file specifier: `variable@filename`

#### "Timeout: No response from MCU"

**Cause:** MCU not responding to commands.

**Solution:**
1. Verify MCU firmware has RAMViewer library
2. Check UART connections (TX/RX)
3. Verify baud rate matches MCU setting
4. Check if MCU is running (not in reset/sleep)

#### "CRC mismatch"

**Cause:** Data corruption during transmission.

**Solution:**
1. Check cable connections
2. Reduce baud rate
3. Check for electrical noise
4. Verify proper grounding

### 12.2 Debug Tips

#### Verify Serial Port

```bash
# List available ports
ramgs ports

# Check connection status
ramgs status
```

#### Test with Simple Variables

Start with simple global variables before complex structures:

```bash
# Simple integer
ramgs get counter

# Before attempting
ramgs get complex_struct.nested.array[5].member
```

#### Check Symbol File

Verify the symbol file contains your variables:

```bash
# Open symbols.json and search for your variable
# Or use Python:
python -c "import json; d=json.load(open('symbols.json')); print([s['name'] for s in d['symbols'] if 'your_var' in s['name']])"
```

### 12.3 MCU-Side Debugging

Add debug output in MCU code:

```c
/* In rv_process_frame() */
static void rv_process_frame(void)
{
    /* Debug: toggle LED on frame receive */
    LED_TOGGLE();

    /* ... rest of function */
}
```

### 12.4 Protocol Analysis

Use a serial monitor to capture raw traffic:

```
PC -> MCU: AA 09 00 01 01 01 06 A0 FF 00 04 00 FF FF [CRC]
           ^  ^     ^  ^  ^  ^                       ^
           |  |     |  |  |  +-- Address: 0x00FFA006 |
           |  |     |  |  +-- Count: 1               |
           |  |     |  +-- Seq: 1                    |
           |  |     +-- Cmd: READ_VAR                |
           |  +-- Len: 9                             |
           +-- SOF                                   +-- CRC16
```

---

## 13. API Reference

### 13.1 Python Package Structure

```
ramgs/
├── cli.py          # Command-line interface
├── config.py       # Configuration constants
├── protocol.py     # Communication protocol
├── serial_manager.py
├── state_manager.py
├── symbol_resolver.py
├── type_converter.py
├── variable_parser.py
├── chart.py        # Realtime chart display
└── repl/           # Interactive mode
    ├── __init__.py
    ├── repl.py     # REPL main loop
    ├── session.py  # Session management
    ├── commands.py # Command handlers
    └── completer.py# Auto-completion
```

### 13.2 Key Classes

#### SymbolResolver

```python
from ramgs.symbol_resolver import SymbolResolver

resolver = SymbolResolver('symbols.json')

# List all symbols
names = resolver.list_symbols()

# Find symbols matching pattern
matches = resolver.list_symbols(pattern='temp')

# Resolve variable path
from ramgs.variable_parser import parse_variables
var_path = parse_variables('sensor.temperature')[0]
resolved = resolver.resolve(var_path)

print(f"Address: 0x{resolved.address:08X}")
print(f"Size: {resolved.size}")
print(f"Type: {resolved.data_type}")
```

#### TypeConverter

```python
from ramgs.type_converter import TypeConverter

converter = TypeConverter(little_endian=True)

# Encode value to bytes
data = converter.encode(3.14, 'float', 4)

# Decode bytes to value
value = converter.decode(b'\xc3\xf5\x48\x40', 'float')

# Parse string value
value = converter.parse_value('0xFF', 'unsigned char')
```

#### Protocol

```python
from ramgs.protocol import Protocol, VarInfo
import serial

port = serial.Serial('COM3', 9600)
protocol = Protocol(port, little_endian=True)

# Read variable
var_info = VarInfo(address=0x20001000, size=4)
success, data_list, error = protocol.read_variables([var_info])

# Write variable
data = b'\x00\x01\x00\x00'
success, error = protocol.write_variables([var_info], [data])
```

### 13.3 Configuration Constants

```python
from ramgs.config import *

SOF = 0xAA              # Start of frame
CMD_READ_VAR = 0x01     # Read command
CMD_WRITE_VAR = 0x02    # Write command
NO_BITFIELD = 0xFF      # Not a bitfield marker
DEFAULT_TIMEOUT_MS = 500
MAX_RETRIES = 3
```

---

## Appendix A: symbols.json Format

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

## Appendix B: Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.3.0 | 2026-01 | Added full-featured GUI mode with realtime chart |
| 1.2.0 | 2026-01 | Added image command for static chart generation |
| 1.1.0 | 2026-01 | Added interactive mode (REPL), realtime chart command |
| 1.0.0 | 2024-01 | Initial release |

---

## Appendix C: License

This software is provided as-is for embedded systems development and debugging purposes.

---

*Document Version: 1.3.0*
*Last Updated: 2026-01*
