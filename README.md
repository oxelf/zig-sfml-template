# Simple SFML game template that uses Zig as the build system

This template supports compiling to windows, macOS, linux, android and iOS.

Only dependencies is having a zig compiler installed.

Cross compilation is supported for:
- Windows from Linux and macOS
- iOS from macOS
- Android from Linux, Windows and macOS(requires Android NDK)

Linux is the only platform that requires additional dependencies to be installed,
as those are not feasible to be build from source.

## Linux Dependencies

```bash
apt update
apt install \
    libx11-dev \
    libxrandr-dev \
    libxcursor-dev \
    libxi-dev \
    libudev-dev
```

# Quick Start
Running
```bash
zig build run
```
should build and run the game on your host platform.

# Building for windows
From Linux or macOS, you can build for windows by running:
```bash
zig build -Dtarget=x86_64-windows-gnu
```
This will output a `game.exe` file in the `zig-out/bin` directory.
