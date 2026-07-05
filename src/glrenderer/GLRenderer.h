// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2005 Slawomir Strumecki

#pragma once
#include "GLRenderDevice.h"
#include "base.h"
#include "rendering/IRenderer.h"

#ifdef _WIN32
// #   warning(dllexport in MSVC style)
#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)
#else
#define DLLEXPORT
#define DLLIMPORT
#endif

#ifdef GLRENDERER_EXPORTS
// #   warning(exporting symbols)
#define GLRENDERER_API DLLEXPORT
#else
#ifdef GLRENDERER_STATIC
// #       warning(static library)
#define GLRENDERER_API
#else
// #       warning(importing symbols)
#define GLRENDERER_API DLLIMPORT
#endif
#endif

extern "C"
{
    GLRENDERER_API forg::IRenderer* forgCreateRenderer();
    GLRENDERER_API int forgDestroyRenderer(forg::IRenderer* renderer);
    GLRENDERER_API const forg::RendererPluginDescriptor*
    forgGetRendererPluginDescriptor();
}
