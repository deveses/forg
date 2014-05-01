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

#ifndef _GL_RENDERER_H_
#define _GL_RENDERER_H_

#if _MSC_VER > 1000
#pragma once
#endif

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


#ifdef SWRENDERER_EXPORTS
//#   warning(exporting symbols)
#   define SWRENDERER_API DLLEXPORT
#else
#   ifdef SWRENDERER_STATIC
//#       warning(static library)
#       define SWRENDERER_API
#   else
//#       warning(importing symbols)
#       define SWRENDERER_API DLLIMPORT
#   endif
#endif

extern "C" {

SWRENDERER_API forg::IRenderer* forgCreateRenderer();

}


#endif  //_GL_RENDERER_H_
