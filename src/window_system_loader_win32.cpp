// SPDX-License-Identifier: LGPL-2.1-or-later
#include "window_system_loader.h"
#include "window_system.h"
#include "ws/win32_native_system.h"
#include "ws/swapchain_window_system.h"
#include "options.h"
#include <stdexcept>

// Built into the executable: no ELF-style unresolved plugin symbols and no
// additional private Vulkan loader or ICD beside the benchmark executable.
WindowSystemLoader::WindowSystemLoader(Options& options)
    : options{options}, lib_handle{nullptr, [](void*) {}}
{
}
WindowSystemLoader::~WindowSystemLoader() = default;

void WindowSystemLoader::load_window_system_options()
{
    options.add_window_system_help("Win32: fixed-size native Vulkan window; Escape closes the benchmark.\n");
}

WindowSystem& WindowSystemLoader::load_window_system()
{
    if (window_system)
        return *window_system;
    if (!options.window_system.empty() && options.window_system != "win32")
        throw std::runtime_error{"This Windows build supports --winsys win32"};
    if (!options.window_system_options.empty())
        throw std::runtime_error{"Unsupported Win32 window-system options"};
    if (options.size.first <= 0 || options.size.second <= 0)
        throw std::runtime_error{"Win32 benchmark requires a positive --size, not fullscreen"};
    window_system = std::make_unique<SwapchainWindowSystem>(
        std::make_unique<Win32NativeSystem>(vk::Extent2D{
            static_cast<uint32_t>(options.size.first), static_cast<uint32_t>(options.size.second)}),
        options.present_mode, options.pixel_format);
    return *window_system;
}
