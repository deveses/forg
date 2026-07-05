// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2005-2008 Slawomir Strumecki

#pragma once
#include "base.h"
#include "rendering/IRenderDevice.h"

#include <cstddef>
#include <cstdint>

namespace forg {

struct RENDER_PARAMETERS
{
    u32 BackBufferWidth;
    u32 BackBufferHeight;
    u32 PresentationInterval;
};

class IRenderer
{
  public:
    virtual ~IRenderer() {}

    virtual IRenderDevice*
    CreateDevice(HWIN hWindow, RENDER_PARAMETERS* pPresentationParameters) = 0;

    virtual LPCTSTR get_Name() = 0;
};

using LPRENDERER = IRenderer*;

using PFCREATERENDERER = IRenderer* (*)(void);
using PFDESTROYRENDERER = int (*)(IRenderer*);

inline constexpr std::uint32_t RendererPluginApiVersion1 = 1;
inline constexpr std::uint32_t RendererPluginApiVersion2 = 2;
inline constexpr std::uint32_t RendererPluginApiVersion =
    RendererPluginApiVersion2;

struct RendererPluginDescriptorV1
{
    std::uint32_t Size;
    std::uint32_t ApiVersion;
    PFCREATERENDERER CreateRenderer;
};

struct RendererPluginDescriptor
{
    std::uint32_t Size;
    std::uint32_t ApiVersion;
    PFCREATERENDERER CreateRenderer;
    PFDESTROYRENDERER DestroyRenderer;
    const char* Name;
};

using PFGETRENDERERPLUGINDESCRIPTOR = const RendererPluginDescriptor* (*)(void);

enum class RendererPluginStatus : std::uint32_t
{
    Ok = 0,
    MissingSymbols,
    NullDescriptor,
    TruncatedDescriptor,
    UnsupportedVersion,
    MissingName,
    MissingCreate,
    MissingDestroy,
    FactoryFailed,
    DestroyFailed,
};

struct RendererPluginBinding
{
    PFCREATERENDERER CreateRenderer = nullptr;
    PFDESTROYRENDERER DestroyRenderer = nullptr;
    const char* Name = nullptr;
    std::uint32_t ApiVersion = 0;
    bool UsesLegacyFactory = false;
    bool UsesPluginDestroy = false;
};

inline const char*
RendererPluginStatusName(RendererPluginStatus status) noexcept
{
    switch (status)
    {
    case RendererPluginStatus::Ok:
        return "ok";
    case RendererPluginStatus::MissingSymbols:
        return "missing renderer plugin symbols";
    case RendererPluginStatus::NullDescriptor:
        return "null renderer plugin descriptor";
    case RendererPluginStatus::TruncatedDescriptor:
        return "truncated renderer plugin descriptor";
    case RendererPluginStatus::UnsupportedVersion:
        return "unsupported renderer plugin version";
    case RendererPluginStatus::MissingName:
        return "missing renderer plugin name";
    case RendererPluginStatus::MissingCreate:
        return "missing renderer creation callback";
    case RendererPluginStatus::MissingDestroy:
        return "missing renderer destruction callback";
    case RendererPluginStatus::FactoryFailed:
        return "renderer factory failed";
    case RendererPluginStatus::DestroyFailed:
        return "renderer destruction failed";
    }
    return "unknown renderer plugin error";
}

inline RendererPluginStatus
BindRendererPluginDescriptor(const RendererPluginDescriptor* descriptor,
                             RendererPluginBinding& binding) noexcept
{
    binding = {};

    if (descriptor == nullptr)
        return RendererPluginStatus::NullDescriptor;

    if (descriptor->ApiVersion == RendererPluginApiVersion1)
    {
        if (descriptor->Size < sizeof(RendererPluginDescriptorV1))
            return RendererPluginStatus::TruncatedDescriptor;

        if (descriptor->CreateRenderer == nullptr)
            return RendererPluginStatus::MissingCreate;

        binding.CreateRenderer = descriptor->CreateRenderer;
        binding.ApiVersion = descriptor->ApiVersion;
        return RendererPluginStatus::Ok;
    }

    if (descriptor->ApiVersion != RendererPluginApiVersion2)
        return RendererPluginStatus::UnsupportedVersion;

    if (descriptor->Size < sizeof(RendererPluginDescriptor))
        return RendererPluginStatus::TruncatedDescriptor;

    if (descriptor->Name == nullptr || descriptor->Name[0] == '\0')
        return RendererPluginStatus::MissingName;

    if (descriptor->CreateRenderer == nullptr)
        return RendererPluginStatus::MissingCreate;

    if (descriptor->DestroyRenderer == nullptr)
        return RendererPluginStatus::MissingDestroy;

    binding.CreateRenderer = descriptor->CreateRenderer;
    binding.DestroyRenderer = descriptor->DestroyRenderer;
    binding.Name = descriptor->Name;
    binding.ApiVersion = descriptor->ApiVersion;
    binding.UsesPluginDestroy = true;
    return RendererPluginStatus::Ok;
}

inline RendererPluginStatus
ProbeRendererPlugin(PFGETRENDERERPLUGINDESCRIPTOR getDescriptor,
                    PFCREATERENDERER legacyCreateRenderer,
                    RendererPluginBinding& binding) noexcept
{
    binding = {};

    if (getDescriptor != nullptr)
        return BindRendererPluginDescriptor(getDescriptor(), binding);

    if (legacyCreateRenderer == nullptr)
        return RendererPluginStatus::MissingSymbols;

    binding.CreateRenderer = legacyCreateRenderer;
    binding.UsesLegacyFactory = true;
    return RendererPluginStatus::Ok;
}

inline RendererPluginStatus
CreateRendererFromPlugin(const RendererPluginBinding& binding,
                         IRenderer*& renderer) noexcept
{
    renderer = nullptr;

    if (binding.CreateRenderer == nullptr)
        return RendererPluginStatus::MissingCreate;

    renderer = binding.CreateRenderer();
    return renderer != nullptr ? RendererPluginStatus::Ok
                               : RendererPluginStatus::FactoryFailed;
}

inline RendererPluginStatus
DestroyRendererFromPlugin(const RendererPluginBinding& binding,
                          IRenderer* renderer) noexcept
{
    if (renderer == nullptr)
        return RendererPluginStatus::Ok;

    if (binding.UsesPluginDestroy)
    {
        if (binding.DestroyRenderer == nullptr)
            return RendererPluginStatus::MissingDestroy;

        return binding.DestroyRenderer(renderer) == FORG_OK
                   ? RendererPluginStatus::Ok
                   : RendererPluginStatus::DestroyFailed;
    }

    delete renderer;
    return RendererPluginStatus::Ok;
}

inline bool
IsRendererPluginCompatible(const RendererPluginDescriptor* descriptor) noexcept
{
    RendererPluginBinding binding;
    return BindRendererPluginDescriptor(descriptor, binding) ==
           RendererPluginStatus::Ok;
}

} // namespace forg
