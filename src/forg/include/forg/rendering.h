/*******************************************************************************
This source file is part of FORG library (http://forg.googlecode.com)
Copyright (C) 2005-2008  Slawomir Strumecki

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

#ifndef _FORG_RENDERING_H_
#define _FORG_RENDERING_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "rendering/Camera.h"
#include "rendering/Color.h"
#include "rendering/Light.h"
#include "rendering/Material.h"
#include "rendering/IIndexBuffer.h"
#include "rendering/IRenderer.h"
#include "rendering/IRenderDevice.h"
#include "rendering/ISurface.h"
#include "rendering/ITexture.h"
#include "rendering/IVertexBuffer.h"
#include "rendering/Sprite.h"
#include "rendering/VertexDeclaration.h"
#include "rendering/VertexElement.h"
#include "rendering/Vertex.h"

#include "rendering/Mesh.h"
#include "rendering/Font.h"

#include "rendering/reference/SWBuffers.h"
#include "rendering/reference/SWRenderDevice.h"

#endif  //_FORG_RENDERING_H_