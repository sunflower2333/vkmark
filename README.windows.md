# Native Windows benchmark candidate

The Windows CMake build retains upstream VkMark scenes, shaders, measurements
and Vulkan swapchain code. A built-in Win32 backend creates a real HWND and
VK_KHR_win32_surface. It uses the system Vulkan loader and registered ICD at
runtime. No private driver is packaged. Linux continues to use Meson/plugins.

Build with MSVC and the pinned vcpkg toolchain:

```
cmake -S . -B build -A ARM64 "-DCMAKE_TOOLCHAIN_FILE=external/vcpkg/scripts/buildsystems/vcpkg.cmake" "-DVCPKG_TARGET_TRIPLET=arm64-windows"
cmake --build build --config Release
cmake --install build --config Release --prefix package
```

For x64 use `-A x64` and `x64-windows`; for x86 use `-A Win32` and
`x86-windows`. CI assembles the executable, dependencies, data and hashed source
receipt, excluding its build-time Vulkan loader DLL. Run from the package
directory, or point `--data-dir` to its data directory. The guest must already
have the corresponding system Vulkan loader and registered hardware ICD.

Start validation with a bounded clear or cube scene, for example:

```
vkmark.exe --winsys win32 --size 800x600 -b clear:duration=2
```

Escape or window close requests ordered Vulkan teardown. The initial backend
has fixed window dimensions because the common upstream swapchain does not
recreate on resize. Fullscreen and window-system-specific options reject rather
than silently changing the test. Run real visible-window validation on the
target before treating an artifact as a passing GPU benchmark. Compilation,
`--help`, and Linux unit tests do not establish Windows GPU/presentation success.
