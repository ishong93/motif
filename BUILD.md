# Building Motif

## Prerequisites

- CMake 3.20+
- C++17 compiler (GCC 8+, Clang 7+)
- X11 development libraries
- libXpm (optional, for XPM image support)

### Debian/Ubuntu

```bash
sudo apt install build-essential cmake libx11-dev libxpm-dev
```

### Fedora/RHEL

```bash
sudo dnf install gcc-c++ cmake libX11-devel libXpm-devel
```

### Arch Linux

```bash
sudo pacman -S base-devel cmake libx11 libxpm
```

## Build

```bash
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
```

## Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `MOTIF_BUILD_TESTS` | ON | Build unit tests (requires internet for GoogleTest) |
| `MOTIF_BUILD_EXAMPLES` | ON | Build example programs |
| `MOTIF_BUILD_MRM` | ON | Build Motif Resource Manager library |
| `MOTIF_BUILD_UIL` | ON | Build UIL compiler (`motif-uil`) |
| `MOTIF_BUILD_WM` | ON | Build Motif Window Manager (`motif-wm`) |

Example with options:

```bash
cmake .. -DMOTIF_BUILD_TESTS=OFF -DMOTIF_BUILD_WM=OFF
```

## Install

```bash
sudo cmake --install .
```

Default install prefix is `/usr/local`. Override with:

```bash
cmake .. -DCMAKE_INSTALL_PREFIX=/opt/motif
```

## Using from another CMake project

After installing:

```cmake
find_package(Motif REQUIRED)
target_link_libraries(myapp PRIVATE Motif::motif)
```

Or with pkg-config:

```bash
pkg-config --cflags --libs motif
```

## Run Tests

```bash
cd build
ctest --output-on-failure
```

## Project Structure

```
motif/
  include/motif/       Public headers
    core/              Widget, Primitive, Manager, Shell, Resource, ...
    widgets/           Label, PushButton, Text, List, Scale, ...
    containers/        Form, RowColumn, Frame, PanedWindow, Notebook, ...
    dialogs/           MessageBox, FileSelectionBox, SelectionBox, ...
    menus/             MenuBar, PopupMenu, MenuShell
    clipboard/         CutPaste, Transfer
    dnd/               DragContext, DropSiteManager
    graphics/          Color, Draw, Pixmap, ImageCache
    input/             KeyBinding, VirtualKey, Traversal
    mrm/               ResourceLoader (UID binary format)
  src/                 Library implementation
  mrm/                 Resource Manager (MRM) library
  uil/                 UIL compiler
  wm/                  Window Manager
  tests/               GoogleTest unit tests
  examples/            Demo programs
  legacy/              Original OpenMotif C source (reference)
```
