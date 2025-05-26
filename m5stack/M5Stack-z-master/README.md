# M5Stack Library

[![Arduino Lint](https://github.com/m5stack/M5Stack/actions/workflows/Arduino-Lint-Check.yml/badge.svg)](https://github.com/m5stack/M5Stack/actions/workflows/Arduino-Lint-Check.yml)
[![Clang Format](https://github.com/m5stack/M5Stack/actions/workflows/clang-format-check.yml/badge.svg)](https://github.com/m5stack/M5Stack/actions/workflows/clang-format-check.yml)

English | [中文](docs/getting_started_cn.md) | [日本語](docs/getting_started_ja.md)

The M5Stack library provides access to the built-in hardware of the M5Stack Core (including Basic and Gray) controllers.

## Recommendation
**<span style="font-size: larger">This library is not recommended for new projects, nor for projects using M5Stack products other than _Basic_ and _Gray_.  See [M5Unified](https://github.com/M5Stack/M5Unified) and [M5GFX](https://github.com/M5Stack/M5GFX) for libraries that are more up-to-date and with support for many newer M5Stack products.</span>**

## Usage
This library's functionality is available through the M5Stack class, referenced as `M5`, including:
- LCD display via `M5.Lcd`, a customized version of the [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) LCD library, adapted for Basic and Gray.  The display resolution is 320x240 supporting 262K colors.  Several fonts, graphics primitives, QR code rendering, and PNG/BMP/JPEG support are included.
- Power control, including device shutoff and battery level detection via `M5.Power` and the IP5306 power management chip
- Hardware buttons via `M5.BtnA`, `M5.BtnB`, `M5.BtnC`
- Speaker, via `M5.Speaker`, supporting tones, volume control, and basic wave audio playback using ESP32's DAC function
- Accelerometer, via `M5.Imu`, `M5.Mpu6886`, or `M5.Sh200Q` (accelerometer type depends on version of Basic/Gray) 

To use the M5Stack library, `#include "M5Stack.h"` must appear at the top of your sketch.

The most productive way to become acquainted with the class is through the numerous included examples.  Additional documentation within this library:
- [docs/Power_capabilities.md](docs/Power_capabilities.md)

Additional optional include files:
- `M5Faces.h` for access to M5Stack Faces accessories (EOL)
- `LoRaWan.h`

<img src="https://static-cdn.m5stack.com/resource/docs/products/core/basic/basic_01.webp" alt="basic" width="350" height="350"><img src="https://static-cdn.m5stack.com/resource/docs/products/core/gray/gray_01.webp" alt="gray" width="350" height="350">

* **Hardware documentation for [Gray v1.0 (EOL)](https://docs.m5stack.com/en/core/gray)**

* **Hardware documentation for [Basic v1.0 (EOL)](https://docs.m5stack.com/en/core/basic)  [Basic v2.6 (EOL)](https://docs.m5stack.com/en/core/basic_v2.6)  [Basic v2.7](https://docs.m5stack.com/en/core/basic_v2.7)**.  The version number may be printed on the main circuit board next to the SD card slot.

* **[Buy Basic](https://shop.m5stack.com/products/esp32-basic-core-iot-development-kit-v2-7)**

*[Comparison of M5Stack controllers](https://docs.m5stack.com/en/products_selector/m5core_compare)*

### M-BUS
<img src="docs/M-BUS.jpg" width="300" />

## More Information

**UIFlow Quick Start**: [Click Here](https://docs.m5stack.com/en/quick_start/m5core/uiflow)

**MicroPython API**: [Click Here](https://docs.m5stack.com/en/mpy/display/m5stack_lvgl)

**Arduino IDE Development**: [Click Here](https://docs.m5stack.com/en/quick_start/m5core/arduino)

**Gray PinMap**: [Click Here](https://docs.m5stack.com/en/core/gray)

**Basic PinMap**: [v1.0 (EOL)](https://docs.m5stack.com/en/core/basic)  [v2.6 (EOL)](https://docs.m5stack.com/en/core/basic_v2.6)  [v2.7](https://docs.m5stack.com/en/core/basic_v2.7)**

## USER CASES

* [M5Stack-SD-Updater](https://github.com/tobozo/M5Stack-SD-Updater) - Customizable menu system for M5Stack - loads apps from the Micro SD
  card

* [M5StackSAM](https://github.com/tomsuch/M5StackSAM) - Simple Applications Menu Arduino Library for M5Stack

* [cfGUI](https://github.com/JF002/cfGUI) - A simple GUI library for M5Stack (ESP32)

* [GUIslice](https://github.com/ImpulseAdventure/GUIslice) - A lightweight GUI framework suitable for embedded displays

* [M5ez](https://github.com/M5ez/M5ez) - The easy way to program on the M5Stack

* [M5Stack MultiApp Advanced](https://github.com/botofancalin/M5Stack-MultiApp-Advanced) - A M5Stack firmware made on PlatformIO

* [M5Stack ESP32 Oscilloscope](https://github.com/botofancalin/M5Stack-ESP32-Oscilloscope) - A fully functional oscilloscope based on ESP32 M5Stack

* [M5Stack-Avatar](https://github.com/meganetaaan/m5stack-avatar) - An M5Stack library for rendering avatar faces

* [M5Stack_CrackScreen](https://github.com/nomolk/M5Stack_CrackScreen) - Crack your M5Stack

* [M5_Shuttle_Run](https://github.com/n0bisuke/M5_Shuttle_Run) - M5_Shuttle_Run

* [nixietubeM5](https://github.com/drayde/nixietubeM5) - (Fake) Nixie Tube Display on a M5Stack

* [M5Stack_BTCTicker](https://github.com/dankelley2/M5Stack_BTCTicker) - A small Bitcoin price ticker using an M5Stack (ESP32) and the Coindesk API

* [M5Stack_ETHPrice](https://github.com/donma/M5StackWifiSettingWithETHPrice) - Dependence on example Wifi Setting to get ETH Price from Maicoin

* [M5Stack-PacketMonitor](https://github.com/tobozo/M5Stack-PacketMonitor) - M5Stack ESP32 Packet Monitor

* [M5-FFT](https://github.com/ElectroMagus/M5-FFT) - Graphic Equalizer on the M5Stack platform

* [M5Stack_ESP32_radio](https://github.com/anton-b/M5Stack_ESP32_radio) - Playing mp3 stream out of internet using M5Stack prototype

* [mp3-player-m5stack](https://github.com/dsiberia9s/mp3-player-m5stack) - MP3 player for M5Stack

* [ArduinoWiFiPhotoBackup](https://github.com/moononournation/ArduinoWiFiPhotoBackup) - M5STACK Arduino WiFi Photo Backup device

* [M5StackHIDCtrlAltDel](https://github.com/mhama/M5StackHIDCtrlAltDel) - You can send ctrl+alt+del to your PC via Bluetooth from M5Stack

* [M5Stack Markdown Web Server](https://github.com/PartsandCircuits/M5Stack-MarkdownWebServer) - Markdown & icons loaded from an Micro SD card/TF card to run a web page

* [M5Stack-Tetris](https://github.com/PartsandCircuits/M5Stack-Tetris) - Tetris for M5Stack Ported to M5Stack by macsbug - https://macsbug.wordpress.com/

* [M5Stack_FlappyBird_game](https://github.com/pcelli85/M5Stack_FlappyBird_game) - M5Stack FlappyBird Playable

* [M5Stack-SpaceShooter](https://github.com/PartsandCircuits/M5Stack-SpaceShooter) - Space Invaders knock-off for M5Stack

* [M5Stack-Pacman-JoyPSP](https://github.com/tobozo/M5Stack-Pacman-JoyPSP) - Pacman on M5Stack/PSP Joypad, with sounds

* [M5Stack-Thermal-Camera](https://github.com/hkoffer/M5Stack-Thermal-Camera-) - M5Stack Thermal Camera with AMG8833 thermal sensor

* [truetype2gfx](https://github.com/ropg/truetype2gfx) - Converting fonts from TrueType to Adafruit GFX

* [m5stack-onscreen-keyboard](https://github.com/yellowelise/m5stack-onscreen-keyboard) - Full size qwerty keyboard for M5Stack
