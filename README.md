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
scripts/run-cli.sh linux-debug   # runs shairport-mqtt (CLI)
scripts/run-gui.sh linux-debug   # runs shairport-mqtt-gui
```

Both are thin wrappers around `scripts/run.sh <preset> <target>` (target defaults
to the CLI if omitted).

For `windows-mingw-cross`, the script runs the `.exe` under `wine` if installed,
otherwise it just tells you where the binary is so you can copy it to a real
Windows machine. Note: some Windows binaries that statically link OpenSSL can hit
Wine-specific threading quirks that don't occur on real Windows — Wine is a
convenience smoke test here, not the actual target environment.

## Project layout

```
CMakeLists.txt                       Build definition (three targets, see below)
CMakePresets.json                    linux-debug / linux-release / windows-mingw-cross
vcpkg.json                           Dependency manifest
cmake/toolchains/mingw-w64-x86_64.cmake  Windows cross-compile toolchain
include/shairport-mqtt/              Public headers for the shared core
src/core/                            Shared logic (target: shairport-mqtt-core, static lib)
src/cli/main.cpp                     CLI entry point (target: shairport-mqtt)
src/gui/main.cpp                     GUI entry point (target: shairport-mqtt-gui)
scripts/                             setup-env.sh, build.sh, run.sh, run-cli.sh, run-gui.sh
```

`.cpp` files under `src/core/` are compiled into the `shairport-mqtt-core`
static library, which both `shairport-mqtt` (CLI) and `shairport-mqtt-gui`
link against. `.cpp` files under `src/cli/` go into the CLI target;
`.cpp` files under `src/gui/` go into the GUI target. All three pick up new
files automatically (no need to edit `CMakeLists.txt` — just re-run
`cmake --preset <preset>` so it re-globs).

### Shared core

- `NowPlayingStore` (`now_playing.hpp`/`.cpp`) — a mutex-guarded snapshot of
  artist/album/title/genre/playing state plus playback progress. Written to
  from the MQTT callback thread, read by the CLI print loop or the GUI render
  loop.
- `ShairportMqttClient` (`mqtt_client.hpp`/`.cpp`) — connects to the broker,
  subscribes to `shairport-sync/#`, and updates a `NowPlayingStore` as
  messages arrive.

shairport-sync publishes both human-readable topics (`shairport-sync/artist`,
`/album`, `/title`, `/genre`, `/playing`) and raw DACP/sync topics
(`shairport-sync/core/*`, `shairport-sync/ssnc/*` — encoded codes, progress
markers, and binary cover art). Only `artist`/`album`/`title`/`genre`/`playing`
and `shairport-sync/ssnc/prgr` (progress) are parsed; the rest is ignored
since the decoded values already cover what the CLI/GUI display.

#### Playback progress

`shairport-sync/ssnc/prgr` is published as `start/current/end`, three RTP
timestamps (sample counts at AirPlay's fixed 44100Hz). Elapsed/duration are
derived as `(current-start)/44100` and `(end-start)/44100` and stored in
`NowPlayingStore`. Because shairport-sync only sends this occasionally (not
every second), `live_elapsed_seconds()` in `now_playing.hpp` extrapolates
forward using real elapsed wall-clock time since the last update (only while
`playing` is true) so the CLI/GUI progress bar ticks smoothly between updates
instead of jumping.

## GUI stack

The GUI uses [Dear ImGui](https://github.com/ocornut/imgui) with the GLFW +
OpenGL3 backend — chosen for minimal boilerplate (immediate-mode, no
signal/slot wiring or layout files) and because that combination cross-compiles
to Windows via mingw cleanly. It renders live artist/album/title/genre/playing
state and a playtime/duration progress bar (`ImGui::ProgressBar`) from
`NowPlayingStore` each frame. The Prev/Play-Pause/Next buttons publish
remote-control commands over MQTT (see below).

### Sending commands

`ShairportMqttClient::send_command()` publishes to `<topic>/remote`
(`shairport-sync/remote` here), which is the topic shairport-sync's
remote-control feature subscribes to. Accepted payloads are exact strings —
see `ShairportCommand` in `mqtt_client.hpp` (`playpause`, `nextitem`,
`previtem`, `volumeup`, `volumedown`, ...).

**This requires `enable_remote = "yes";` in the `mqtt` section of
shairport-sync.conf — it's `"no"` by default**, so the buttons will silently
do nothing until that's set and shairport-sync is restarted.

## Roadmap notes

- MQTT client library: [Eclipse Paho C++](https://github.com/eclipse/paho.mqtt.cpp),
  built without TLS for now (easy to add later by dropping `default-features:
  false` from `paho-mqttpp3` in `vcpkg.json` — its OpenSSL dependency is already
  pulled in transitively either way).
- The macropad-side transport (raw USB HID, serial/CDC, or global hotkeys from
  QMK-sent keycodes) is still an open decision and will land as its own module.
- Volume up/down commands exist in `ShairportCommand` but have no buttons yet.
- Album art (`shairport-sync/ssnc/PICT`, assembled between `pcst`/`pcen`
  markers) is not parsed yet.
