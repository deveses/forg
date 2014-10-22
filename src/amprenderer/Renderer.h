/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2005  Slawomir Strumecki

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

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
