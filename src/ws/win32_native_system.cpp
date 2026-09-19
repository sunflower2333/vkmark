// SPDX-License-Identifier: LGPL-2.1-or-later
#include "win32_native_system.h"
#include "vulkan_state.h"
#include <stdexcept>
#include <string>

namespace
{
constexpr wchar_t window_class[] = L"VkMarkNativeWindow";
std::runtime_error win32_error(char const* operation)
{
    return std::runtime_error{std::string{operation} + ": Win32 error " + std::to_string(GetLastError())};
}
}

Win32NativeSystem::Win32NativeSystem(vk::Extent2D extent)
    : instance{GetModuleHandleW(nullptr)}
{
    if (!extent.width || !extent.height || extent.width > 32767 || extent.height > 32767)
        throw std::runtime_error{"Invalid Win32 client extent"};
    WNDCLASSW wc{};
    wc.lpfnWndProc = window_proc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = window_class;
    if (!RegisterClassW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        throw win32_error("RegisterClassW");

    // VkMark's common swapchain has no resize/recreation path. A fixed client
    // extent prevents a border drag from invalidating an in-flight benchmark.
    DWORD const style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    RECT rect{0, 0, static_cast<LONG>(extent.width), static_cast<LONG>(extent.height)};
    if (!AdjustWindowRect(&rect, style, FALSE))
        throw win32_error("AdjustWindowRect");
    window = CreateWindowExW(0, window_class, L"vkmark - Vulkan benchmark", style,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, instance, this);
    if (!window)
        throw win32_error("CreateWindowExW");
    ShowWindow(window, SW_SHOW);
}

Win32NativeSystem::~Win32NativeSystem()
{
    // The owning SwapchainWindowSystem releases Vulkan surface/swapchain first.
    if (window)
        DestroyWindow(window);
}

LRESULT CALLBACK Win32NativeSystem::window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    auto self = reinterpret_cast<Win32NativeSystem*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (msg == WM_NCCREATE)
    {
        self = static_cast<Win32NativeSystem*>(reinterpret_cast<CREATESTRUCTW*>(lp)->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    if (self && (msg == WM_CLOSE || (msg == WM_KEYDOWN && wp == VK_ESCAPE)))
    {
        // Request orderly GPU teardown; never destroy a live surface here.
        self->quit = true;
        return 0;
    }
    if (msg == WM_NCDESTROY)
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
    return DefWindowProcW(hwnd, msg, wp, lp);
}

VulkanWSI::Extensions Win32NativeSystem::required_extensions()
{
    return {{VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME}, {}};
}

uint32_t Win32NativeSystem::get_presentation_queue_family_index(vk::PhysicalDevice const& pd)
{
    auto const families = pd.getQueueFamilyProperties();
    for (uint32_t i = 0; i < families.size(); ++i)
        if (families[i].queueCount && pd.getWin32PresentationSupportKHR(i))
            return i;
    return invalid_queue_family_index;
}

bool Win32NativeSystem::should_quit()
{
    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
    {
        if (message.message == WM_QUIT)
            quit = true;
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return quit;
}

vk::Extent2D Win32NativeSystem::get_vk_extent()
{
    RECT rect{};
    if (!GetClientRect(window, &rect))
        throw win32_error("GetClientRect");
    return {static_cast<uint32_t>(rect.right - rect.left), static_cast<uint32_t>(rect.bottom - rect.top)};
}

ManagedResource<vk::SurfaceKHR> Win32NativeSystem::create_vk_surface(VulkanState& vulkan)
{
    auto info = vk::Win32SurfaceCreateInfoKHR{}.setHinstance(instance).setHwnd(window);
    return {vulkan.instance().createWin32SurfaceKHR(info),
        [vptr = &vulkan](vk::SurfaceKHR& surface) { vptr->instance().destroySurfaceKHR(surface); }};
}
