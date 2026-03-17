# VulkanPBR Framework

A Vulkan-based physically based rendering (PBR) learning project built with modern C++.

## Requirements

| Tool | Version |
|------|---------|
| Visual Studio | 2019 or higher |
| CMake | 3.20 or higher |
| Vulkan SDK | Latest |

## Project Structure

```
VulkanPBRFrameWork/
├── Code/          ← CMakeLists.txt lives here
└── ThirdParty/
    ├── GLFW/
    └── GLM/
```

## Getting Started

### 1. Install Vulkan SDK

1. Download the latest SDK from [lunarg.com/sdk/home](https://vulkan.lunarg.com/sdk/home)
2. Run the installer
3. Confirm that `VULKAN_SDK` is set as a system environment variable

### 2. Build

1. Open Visual Studio → **Open a Local Folder** → select the `Code/` directory
2. Visual Studio will auto-detect the CMake project
3. Set the configuration to **x64-Debug**
4. Press `Ctrl+Shift+B` to build

### 3. Set Up Runtime Resources

On first run, the executable won't find shaders or assets unless you copy them manually.

1. Locate the build output directory:
   ```
   out/build/x64-Debug/
   ```
2. Copy the following folders from `Code/` into that directory:
   ```
   out/build/x64-Debug/
   ├── VulkanLearning.exe
   ├── shaders/          ← copied from Code/shaders/
   └── assets/           ← copied from Code/assets/
   ```
3. Run the executable

## Troubleshooting

**`Vulkan headers not found` at compile time**
Ensure the Vulkan SDK is installed and the `VULKAN_SDK` environment variable is set correctly.

**Black screen or crash on launch**
The `shaders/` and `assets/` folders are missing from the executable directory. See [Set Up Runtime Resources](#3-set-up-runtime-resources).

**Linker errors**
Make sure you're building with the **x64** configuration. The 32-bit build is not supported.

## Tech Stack

- **Graphics API**: Vulkan
- **Math**: GLM
- **Windowing**: GLFW
- **Build System**: CMake
- **Compiler**: MSVC
- **Platform**: Windows x64

## License

MIT License — see [LICENSE](./LICENSE) for details.

## Contributing

Issues and pull requests are welcome!