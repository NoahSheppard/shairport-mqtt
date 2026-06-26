# shairport-mqtt

A C++ bridge between a custom macropad (QMK or custom firmware) and
[shairport-sync](https://github.com/mikebrady/shairport-sync), using MQTT as the
transport.

shairport-sync is the AirPlay *receiver* — it doesn't broker anything itself. When
built with `--with-mqtt-client`, it connects out to an MQTT broker (e.g.
[Mosquitto](https://mosquitto.org/)), publishes now-playing metadata, and
subscribes to command topics (play/pause/next/prev/volume) which it forwards to
the source device over Apple's DACP remote-control protocol. This app is the
other MQTT client: it turns macropad input into commands on those topics, and can
later show now-playing metadata read back from shairport-sync's published topics.
Because everything goes through the broker, this app doesn't need to run on the
same machine or even the same OS as shairport-sync.

## Prerequisites

- CMake >= 3.21
- Ninja
- A C++20 compiler (GCC/Clang on Linux)
- Git
- For Windows cross-compilation: `mingw-w64`
- [vcpkg](https://github.com/microsoft/vcpkg) (manages dependencies, e.g.
  `paho-mqttpp3`)

On Debian/Ubuntu:

```sh
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build git mingw-w64 mingw-w64-tools
```

## First-time setup

```sh
scripts/setup-env.sh
```

This clones and bootstraps vcpkg (to `~/vcpkg` by default), and adds `VCPKG_ROOT`
to your `~/.bashrc`. Open a new shell (or `source ~/.bashrc`) afterwards so
`VCPKG_ROOT` is set. If `mingw-w64` isn't installed, the script will tell you the
command to install it.

To use a vcpkg checkout you already have, set `VCPKG_ROOT` before running the
script:

```sh
VCPKG_ROOT=/path/to/your/vcpkg scripts/setup-env.sh
```

## Build

```sh
scripts/build.sh linux-debug          # native Linux, debug
scripts/build.sh linux-release        # native Linux, release
scripts/build.sh windows-mingw-cross  # cross-compiled Windows .exe (static-linked)
```

Equivalent raw CMake commands, if you'd rather not use the scripts:

```sh
cmake --preset <preset>
cmake --build --preset <preset>
```

The first build of any preset will take a few minutes while vcpkg builds
dependencies (OpenSSL, Paho MQTT C/C++) for that target; later builds reuse the
cache and are fast.

## Run

```sh
scripts/run.sh linux-debug
```

For `windows-mingw-cross`, the script runs the `.exe` under `wine` if installed,
otherwise it just tells you where the binary is so you can copy it to a real
Windows machine. Note: some Windows binaries that statically link OpenSSL can hit
Wine-specific threading quirks that don't occur on real Windows — Wine is a
convenience smoke test here, not the actual target environment.

## Project layout

```
CMakeLists.txt                       Build definition
CMakePresets.json                    linux-debug / linux-release / windows-mingw-cross
vcpkg.json                           Dependency manifest (paho-mqttpp3)
cmake/toolchains/mingw-w64-x86_64.cmake  Windows cross-compile toolchain
src/main.cpp                         Entry point
scripts/                             setup-env.sh, build.sh, run.sh
```

## Roadmap notes

- MQTT client library: [Eclipse Paho C++](https://github.com/eclipse/paho.mqtt.cpp),
  built without TLS for now (easy to add later by dropping `default-features:
  false` from `paho-mqttpp3` in `vcpkg.json` — its OpenSSL dependency is already
  pulled in transitively either way).
- The macropad-side transport (raw USB HID, serial/CDC, or global hotkeys from
  QMK-sent keycodes) is still an open decision and will land as its own module.
- Core logic and any future GUI front-end will be split into a separate library
  target once the GUI work actually starts — not done speculatively now.
