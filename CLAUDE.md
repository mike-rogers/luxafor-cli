# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A single-file C command-line utility that sets the color of a Luxafor Flag USB busylight (vendor 0x04d8, product 0xf372) via hidapi. All logic lives in `main.c`: it parses options with `getopt_long` plus one positional color argument (a named color from `colorMap` or a hex value like `0x043f2c`), then writes a single 9-byte HID report — byte 0 is the HID report ID (0x00, required by hidapi on Windows), byte 1 the command (`CMD_STATIC`/`CMD_FADE`/`CMD_STROBE`/`CMD_WAVE`/`CMD_PATTERN`), remaining bytes per-command (see the `switch (mode)` block for each layout). LED targeting: 1–6 individual, 0x41 front, 0x42 back, 0xFF all. Targets macOS, Linux, and Windows.

## Building

```
cmake -B build
cmake --build build --config Release
```

Dependency handling: if a system hidapi with CMake config files is found (`brew install hidapi` on macOS), `find_package(hidapi)` uses it; otherwise hidapi is fetched and statically linked via FetchContent. Force the vendored path with `-DLUXAFOR_VENDORED_HIDAPI=ON`. Building hidapi from source on Linux requires `libudev-dev`. On Windows, FetchContent also pulls in kimgr/getopt_port (compiled directly into the target, bypassing its CMakeLists via a dummy `SOURCE_SUBDIR`) because the MSVC runtime has no `getopt`.

The target is named `LuxaforCLI` but the binary is output as `luxafor` (`build/luxafor`, or `build/Release/luxafor.exe` with Visual Studio).

Warnings are treated seriously: `-Wall -Wextra` (or `/W4` on MSVC) is enabled and the code currently compiles clean — keep it that way.

## Testing

There are no unit tests. Verifying a change means building and running against a physically connected Luxafor device (`./build/luxafor blue`). Compiling with `-DDEBUG` enables an HID device enumeration dump at startup, useful when the device isn't found. CI (`.github/workflows/ci.yml`) builds on Linux/macOS/Windows; pushing a `v*` tag triggers `release.yml`, which builds statically-linked binaries for all three platforms and attaches them to a GitHub Release.
