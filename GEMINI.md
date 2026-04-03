# Subpixel Renderer - Environment Setup Guide

This document outlines the steps required to recreate the development environment for the Subpixel Renderer project on a Windows system.

## 1. Toolchain Installation (via winget)

The project requires a C compiler (GCC), the Make build utility, and the GitHub CLI.

```powershell
# Install MinGW-w64 (WinLibs distribution with UCRT)
winget install BrechtSanders.WinLibs.POSIX.UCRT

# Install GNU Make
winget install ezwinports.make

# Install GitHub CLI
winget install GitHub.cli

# Install Vulkan SDK (For future GPU acceleration on Intel Lunar Lake/Xe2)
winget install KhronosGroup.VulkanSDK
```

## 2. Environment Configuration

### Path Setup
Ensure the following paths are added to your System PATH environment variable:
- `C:\Users\<YourUser>\AppData\Local\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin` (or equivalent WinGet path for `gcc.exe`)
- `C:\VulkanSDK\<Version>\Bin`

### Git Configuration
Set your identity for commits:
```powershell
git config --global user.name "Your Name"
git config --global user.email "mitchrybczynski@gmail.com"
```

### GitHub Authentication
Authenticate the GitHub CLI to allow repository creation and pushing:
```powershell
gh auth login
```

## 3. Building the Project

The project uses a standard `Makefile`. To build the executable:

```powershell
# Compile the project
make

# Run the project (Software Rendering)
.\subpixel_renderer.exe

# Run the project (GPU Flag enabled)
.\subpixel_renderer.exe -g
```

## 4. Project Structure
- `main.c`: Win32 windowing, message loop, and high-precision FPS timing.
- `renderer.c`: Subpixel-accurate drawing algorithms (Wu's Lines, AA Circles, Text).
- `renderer.h` / `types.h`: API declarations and shared data structures.
- `Makefile`: Build instructions linking `gdi32` and `user32`.
