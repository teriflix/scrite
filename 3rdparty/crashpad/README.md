# Setting up Crashpad for use with Scrite

Crashpad is part of Google's Chromium project. It is used for detecting application
crashes, so that appropriate log files may be sent back to us for further investigation.

The source code and build system for crashpad is not bundled as a part of Scrite's 
code itself, and it must built or installed separately. 

In this file we describe the simplest way to setup crashpad for use with Scrite for 
each of the supported platforms.

If you face any issues, please post a topic on the #questions channel in our Discord 
community. **Please DO NOT** send an email, or post a question on any of our YouTube videos,
or on X by tagging us. The only place we will respond to questions is if its posted on
the #questions tab in our Discord channel.

## Setting up crashpad on Windows

### Requirements

- Windows 10 or Windows 11
- Visual Studio 2019
- Windows SDK 10.0.22000.0 or later
- Latest Qt 5.15 LTS (which is 5.15.17 as of writing) for Visual Studio 2019

### Step 1: Install crashpad-binaries

Prebuilt binaries for Visual Studio 2019 are already available at https://github.com/gavv/crashpad-binaries.
We are simply going to clone this repository and use the binaries provided it in.

Open a Command Prompt window and execute the following commands

    mkdir C:\Crashpad
    cd C:\Crashpad
    git clone https://github.com/gavv/crashpad-binaries.git
    
Once the cloning completes, you will notice a `C:\Crashpad\crashpad-binaries` folder. But it will not have
any binaries, since they are available in a separate branch.

    cd C:\Crashpad\crashpad-binaries
    git switch -c VS2019_MD remotes/origin/VS2019_MD
    
This time, when you view the contents of the the `C:\Crashpad\crashpad-binaries` folder, you will notice 
three subfolders `bin`, `lib`, and `include`.

### Step 2: Create SCRITE_CRASHPAD_ROOT environment variable

Open PowerShell as an administrator.

- Hit Win+X
- Select Terminal (Admin) or Windows PowerShell (Admin)
- Execute the following command
    
    `[Environment]::SetEnvironmentVariable("SCRITE_CRASHPAD_ROOT", "C:\Crashpad\crashpad-binaries", "User")`
    
This will cause `crashpad.pri` folder to automatically pick up crashpad binaries for Windows from the 
`C:\Crashpad\crashpad-binaries` folder, and link it with Scrite.

### Step 3: Rebuild Scrite from scratch

Restart Qt Creator, rebuild Scrite from scratch. This time the Scrite project will detect
the availability of crashpad and build accordingly.

## Setting up crashpad on macOS

### Requirements

- macOS Sonoma 14.5 or higher
- Xcode 15.4 or higher
- Latest Qt 5.15 LTS (which is 5.15.17 as of writing)

### Step 1: Install crashpad-binaries

Prebuilt binaries for macOS are already available at https://github.com/gavv/crashpad-binaries.
We are simply going to clone this repository and use the binaries provided it in.

Open a Command Prompt window and execute the following commands

    mkdir $HOME/Crashpad
    cd $HOME/Crashpad
    git clone https://github.com/gavv/crashpad-binaries.git
    
Once the cloning completes, you will notice a `$HOME/Crashpad/crashpad-binaries` folder. But it will not have
any binaries, since they are available in a separate branch.

    cd $HOME/Crashpad/crashpad-binaries
    git switch -c macos remotes/origin/macos
    
This time, when you view the contents of the the `$HOME/Crashpad/crashpad-binaries` folder, you will notice 
three subfolders `bin`, `lib`, and `include`.

### Step 2: Create SCRITE_CRASHPAD_ROOT environment variable

- Open Terminal
- Execute the following command

    `echo "export SCRITE_CRASHPAD_ROOT=$HOME/Crashpad/crashpad-binaries" >> ~/.zshrc`
    
This will cause `crashpad.pri` folder to automatically pick up crashpad binaries for macOS from the 
`$HOME/Crashpad/crashpad-binaries` folder, and link it with Scrite.

### Step 3: Rebuild Scrite from scratch

Restart your Mac (or atleast logout and log back in), Start Qt Creator, rebuild Scrite from scratch. 
This time the Scrite project will detect the availability of crashpad and build accordingly.

## Setting up crashpad on Linux


