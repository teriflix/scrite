# Building From Source
This guide outlines the process of building Scrite from source. Starting with
prerequisites, the guide explains how to install dependencies, pull the source
code of Scrite from this Git repository and build it from source.

## Prerequisites
A desktop or a laptop with 
- 8+ GB RAM
- Graphics Card with 2+ GB VRAM
- 50+ GB of HDD space
- Linux, Windows or macOS installed
- Only 64bit CPUs are supported, both Intel and ARM.

In general it is a good idea to [download the latest production
build](https://www.scrite.io/downloads) of Scrite and verify that it works on
your system. It is recommended that you sign up for a trial to check if all
features are working. If you are already a Scrite user with an active
subscription, use those credentials.

## Qt Development Environment Setup

The Qt Online Installer requires a free Qt Account. You can create one during
installation, but allow a few minutes to verify your email address.

**Qt Online Installer (open source):**
https://www.qt.io/development/download-qt-installer-oss

In the installer, select the open-source option and choose the latest Qt 6 release
(6.11.2 or newer) for your platform (see below). Qt Creator is installed automatically.

### Linux

1. Install the compiler and OpenGL development headers:
   - **Debian/Ubuntu:**
     ```bash
     sudo apt-get install \
       build-essential git \
       libgl1-mesa-dev libgl1 libxcb-glx0 \
       libx11-xcb1 \
       libxcb-cursor0 libxcb-cursor-dev libxcb-keysyms1 libxcb-xkb1 libxkbcommon-x11-0 \
       ibus ibus-m17n libibus-1.0-dev \
       libxcb-icccm4 libxcb-randr0 libxcb-xinerama0 \
       libxcb-render-util0 libxcb-image0 libxcb-shape0 libxcb-sync1 libxcb-xfixes0 \
       libfontconfig1 libglib2.0-dev \
       libminizip-dev zlib1g-dev \
       hunspell libhunspell-dev
     ```
   - **Packages by category:**
     - **Build Essentials:** `build-essential`, `git`
     - **OpenGL & Graphics:** `libgl1-mesa-dev`, `libgl1`, `libxcb-glx0`
     - **X11 Bridge:** `libx11-xcb1`
     - **Input & Keyboard:** `libxcb-cursor0`, `libxcb-cursor-dev`,
       `libxcb-keysyms1`, `libxcb-xkb1`, `libxkbcommon-x11-0`, `ibus`,
       `libibus-1.0-dev`
     - **Display & Window Management:** `libxcb-icccm4`, `libxcb-randr0`,
       `libxcb-xinerama0`
     - **Rendering & Sync:** `libxcb-render-util0`, `libxcb-image0`,
       `libxcb-shape0`, `libxcb-sync1`, `libxcb-xfixes0`
     - **System Libraries:** `libfontconfig1`, `libglib2.0-dev`
     - **Compression & Archives:** `libminizip-dev`, `zlib1g-dev`
     - **Spell Checking:** `hunspell`, `libhunspell-dev`

   > NOTE: Dependencies vary by distribution. Search for distribution-specific instructions if needed.

2. Download the Linux installer (x64 or ARM64), make it executable, and run:
   ```bash
   chmod +x qt-online-installer-*.run
   ./qt-online-installer-*.run
   ```
3. In the installer, select **Qt 6.x > Desktop (gcc 64-bit)** (latest 6.11+ recommended)
4. Note: Qt 6 requires glibc 2.34 or newer (Ubuntu 22.04+, Debian 12, RHEL 9+ are supported)
5. Reference: https://doc.qt.io/qt-6/linux.html

### Windows

1. Install Visual Studio 2022 Community using this direct link:
   https://aka.ms/vs/17/release/vs_community.exe
   
   > Note: The main Visual Studio website now offers VS 2026; use the link above for 2022.

2. In the Visual Studio Installer, enable the **Desktop development with C++** workload
3. (Optional) For a newer Windows SDK:
   https://developer.microsoft.com/windows/downloads/windows-sdk/
4. Run the Qt Online Installer and select **Qt 6.x > MSVC 2022 64-bit** (latest 6.11+ recommended; or ARM64 on ARM laptops)

### macOS

1. Install Xcode 15 or newer from the Mac App Store:
   https://apps.apple.com/app/xcode/id497799835
2. After installation, open Xcode once to complete setup and accept the license
3. Run the Qt Online Installer and select **Qt 6.x > macOS** (latest 6.11+ recommended)

### Verify Installation

Open Qt Creator and create a new "Qt Widgets Application" or "Qt Quick Application"
project. Click Run — if a window appears, your setup is complete.

# Getting the Source Code

## Cloning the Repository

Clone the official repository into a folder of your choice:

```bash
git clone https://github.com/teriflix/scrite.git
cd scrite
```

Initialize submodules for third-party dependencies:

```bash
git submodule update --init --recursive
```

You now have all the source code needed to build Scrite.

# Building from Code

Before building, ensure you have completed the [Qt Development Environment Setup](#qt-development-environment-setup) above with the latest Qt 6 release.

Using Qt's `MaintenanceTool`, we recommend installing:
- All components in the **Additional Libraries** section
- The **Desktop** component of **Qt PDF**
- The **Desktop** component of **Qt WebEngine**

## Building the Project

1. Launch Qt Creator
2. Click on File → Open File or Project
3. Navigate to your cloned scrite folder and select `CMakeLists.txt`, then click `Open`
4. We recommend selecting the Release target as the build target
5. Hit the Play/Run button to build and run the app

> NOTE: Building Scrite from code might take 10-30 minutes depending on CPU
> speed and available RAM.

Once built, the Scrite app will launch and activate itself using a standard
email address. You cannot change that or use the production servers. This means
you will not have access to templates and pre-packaged scripts from Scriptalay.
All other features are fully functional.

> NOTE: Production builds may include extra features not in the public repository.

## Platform-Specific Notes

### Linux

Consider configuring these environment variables in Qt Creator's Run settings:

```
LIBGL_ALWAYS_SOFTWARE=1
QT_QPA_PLATFORM=xcb
```

Optionally, for better input method support:
```
GTK_IM_MODULE=ibus
XMODIFIERS=@im=ibus
QT_IM_MODULE=ibus
XIM_PROGRAM="/usr/bin/ibus-daemon -drx"
```

### Windows

No additional platform-specific configuration needed beyond the standard build steps above.

### macOS

No additional platform-specific configuration needed beyond the standard build steps above.
