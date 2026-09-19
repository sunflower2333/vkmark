// SPDX-License-Identifier: LGPL-2.1-or-later
#pragma once

#include "native_system.h"
#include <windows.h>

class Win32NativeSystem : public NativeSystem
{
public:
    explicit Win32NativeSystem(vk::Extent2D extent);
    ~Win32NativeSystem() override;
    VulkanWSI::Extensions required_extensions() override;
    uint32_t get_presentation_queue_family_index(vk::PhysicalDevice const& pd) override;
    bool should_quit() override;
    vk::Extent2D get_vk_extent() override;
    ManagedResource<vk::SurfaceKHR> create_vk_surface(VulkanState& vulkan) override;

private:
    static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
    HINSTANCE instance;
    HWND window = nullptr;
    bool quit = false;
};
