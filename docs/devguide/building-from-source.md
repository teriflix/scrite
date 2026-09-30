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
your system. It is recommended that you go sign up for a trail to check if all
features are working. If you are already a Scrite user and hold an active
subscription, then you can check with those credentials.

## Qt Development Environment Setup
The Qt Online Installer asks you to log in with a free Qt Account. You can
create one while running the installer, but you'll need to verify your email
address, so allow a few minutes for that.

Qt Online Installer (open source):
https://www.qt.io/development/download-qt-installer-oss

In the installer, choose the open-source option and select Qt > Qt 6.11.2, plus
the component for your platform listed below. Qt Creator is installed
automatically.

## Linux
1. Install the compiler and OpenGL development headers:
   - Debian/Ubuntu:
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
     Packages by category:
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
2. Download the Linux installer (x64 or ARM64) from the link above, make it
   executable with `chmod +x qt-online-installer-*.run`, and run it.
3. Select Qt 6.11.2 > Desktop (gcc 64-bit).
4. Note: Qt 6.11 requires glibc 2.34 or newer (Ubuntu 22.04+, Debian 12, RHEL 9+
   are fine).
5. Reference: https://doc.qt.io/qt-6/linux.html

> NOTE: Dependencies for each GNU Linux Distribution may vary. Please search for
> appropriate instructions.

## Windows
1. Install Visual Studio 2022 Community using Microsoft's direct installer link:
   https://aka.ms/vs/17/release/vs_community.exe (The main Visual Studio website
   now offers VS 2026, so please use this link.)
2. In the Visual Studio Installer, tick the Desktop development with C++
   workload. This includes the MSVC compiler and a Windows SDK.
3. (Optional) If you want a newer standalone Windows SDK:
   https://developer.microsoft.com/windows/downloads/windows-sdk/
4. Run the Qt Online Installer and select Qt 6.11.2 > MSVC 2022 64-bit (or MSVC
   2022 ARM64 on ARM laptops).

## macOS
1. Install Xcode (version 15 or newer) from the Mac App Store:
   https://apps.apple.com/app/xcode/id497799835
2. Open Xcode once after installing so it can finish setting up its components
   and you can accept the license.
3. Run the Qt Online Installer and select Qt 6.11.2 > macOS.

> NOTE: We recommend that you install all components in `Additional Libraries`
> section, and the `Desktop` component of `Qt PDF` and `Qt WebEngine` for the
> version of Qt you install.

Check that it works. Open Qt Creator, create a new "Qt Widgets Application" or
"Qt Quick Application" project, and click Run. If a window appears, you're all
set.

# Getting the Source Code

## Folder Structure
We recommend that you prepare the following folder structure
```
+ Scrite
|-- Code
|-- Release
```

The commands to do so in your home directory on Linux would be

```bash
# cd ~
# mkdir Scrite
# cd Scrite
# mkdir Code
# mkdir Release
```

## Cloning the repository
Shown below are commands to clone from the official repository of Scrite on a
GNU/Linux desktop. You can do something similar on Windows & macOS as well.

```bash
# cd ~/Scrite/Code
# git clone https://github.com/teriflix/scrite.git ~/Scrite/Code
```

The Scrite code makes use of several third party dependencies. You will now need
to init submodules related to those.

```bash
# cd ~/Scrite/Code
# git submodule update --init --recursive
```

That's it. You should have all the code needed for building Scrite.

# Building from Code

- Launch Qt Creator
- Click on File -> Open File or Project
- Select `~/Scrite/Code/CMakeLists.txt` in the file dialog and click `Open`
- We recommend checking Release target and selecting that as the build target.
- Hit the Play/Run button on Qt Creator and let it build the whole app.

> NOTE: Building Scrite from code might take 10-30 minutes depending on CPU
> speed and available RAM. So, please be patient.

Once built, the Scrite app will launch and would have activated itself using a
standard email-id. You cannot change that or make use of our production servers.
This means you will not be able to access templates and pre-packaged scripts
from Scriptalay. Other than that, you will be able to make use of all features.

> NOTE: Production builds may come bundled with certain extra features, the
> source for which may not be available in the public repository.

## Runtime environment

Consider having the following environment variables configured against Run
settings for the Scrite project in Qt Creator.

```
LIBGL_ALWAYS_SOFTWARE=1
QT_QPA_PLATFORM=xcb
```

Optionally, you could also set the following additionally:
```
GTK_IM_MODULE=ibus
XMODIFIERS=@im=ibus
QT_IM_MODULE=ibus
XIM_PROGRAM="/usr/bin/ibus-daemon -drx"
```