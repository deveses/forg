// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2005 Slawomir Strumecki

#pragma once

#include "base.h"
#include "rendering/IRenderer.h"

#ifdef _WIN32
//#   warning(dllexport in MSVC style)
#   define DLLEXPORT __declspec( dllexport )
#   define DLLIMPORT __declspec( dllimport )
#else
#   define DLLEXPORT
#   define DLLIMPORT
#endif


#ifdef RENDERER_EXPORTS
//#   warning(exporting symbols)
#   define AMPRENDERER_API DLLEXPORT
#else
#   ifdef AMPRENDERER_STATIC
//#       warning(static library)
#       define AMPRENDERER_API
#   else
//#       warning(importing symbols)
#       define AMPRENDERER_API DLLIMPORT
#   endif
#endif

extern "C" {

    AMPRENDERER_API forg::IRenderer* forgCreateRenderer();

}
