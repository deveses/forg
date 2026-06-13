/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2007  Slawomir Strumecki

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

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef XFILE_XSTDTEMPLATE_INCLUDED
#define XFILE_XSTDTEMPLATE_INCLUDED

#include "base.h"
#include "mesh/xfile/xdefs.h"
#include "mesh/xfile/xtemplate.h"

/**
*
*
*/

#define DECLARE_XSTDTEMPLATE(x) class XTemplate##x : public XTemplate\
    {\
    public:\
    XTemplate##x();\
        static const xguid GUID;\
        static const char* NAME;\
    }



namespace forg { namespace xfile {

    DECLARE_XSTDTEMPLATE(Animation);
    DECLARE_XSTDTEMPLATE(AnimationKey);
    DECLARE_XSTDTEMPLATE(AnimationOptions);
    DECLARE_XSTDTEMPLATE(AnimationSet);
    DECLARE_XSTDTEMPLATE(AnimTicksPerSecond);
    DECLARE_XSTDTEMPLATE(Boolean);
    DECLARE_XSTDTEMPLATE(Boolean2d);
    DECLARE_XSTDTEMPLATE(ColorRGB);
    DECLARE_XSTDTEMPLATE(ColorRGBA);
    DECLARE_XSTDTEMPLATE(CompressedAnimationSet);
    DECLARE_XSTDTEMPLATE(Coords2d);
    DECLARE_XSTDTEMPLATE(DeclData);
    DECLARE_XSTDTEMPLATE(EffectDWord);
    DECLARE_XSTDTEMPLATE(EffectFloats);
    DECLARE_XSTDTEMPLATE(EffectInstance);
    DECLARE_XSTDTEMPLATE(EffectParamDWord);
    DECLARE_XSTDTEMPLATE(EffectParamFloats);
    DECLARE_XSTDTEMPLATE(EffectParamString);
    DECLARE_XSTDTEMPLATE(EffectString);
    DECLARE_XSTDTEMPLATE(FaceAdjacency);
    DECLARE_XSTDTEMPLATE(FloatKeys);
    DECLARE_XSTDTEMPLATE(Frame);
    DECLARE_XSTDTEMPLATE(FrameTransformMatrix);
    DECLARE_XSTDTEMPLATE(FVFData);
    DECLARE_XSTDTEMPLATE(Guid);
    DECLARE_XSTDTEMPLATE(IndexedColor);
    DECLARE_XSTDTEMPLATE(Material);
    DECLARE_XSTDTEMPLATE(MaterialWrap);
    DECLARE_XSTDTEMPLATE(Matrix4x4);
    DECLARE_XSTDTEMPLATE(Mesh);
    DECLARE_XSTDTEMPLATE(MeshFace);
    DECLARE_XSTDTEMPLATE(MeshFaceWraps);
    DECLARE_XSTDTEMPLATE(MeshMaterialList);
    DECLARE_XSTDTEMPLATE(MeshNormals);
    DECLARE_XSTDTEMPLATE(MeshTextureCoords);
    DECLARE_XSTDTEMPLATE(MeshVertexColors);
    DECLARE_XSTDTEMPLATE(Patch);
    DECLARE_XSTDTEMPLATE(PatchMesh);
    DECLARE_XSTDTEMPLATE(PatchMesh9);
    DECLARE_XSTDTEMPLATE(PMAttributeRange);
    DECLARE_XSTDTEMPLATE(PMInfo);
    DECLARE_XSTDTEMPLATE(PMVSplitRecord);
    DECLARE_XSTDTEMPLATE(SkinWeights);
    DECLARE_XSTDTEMPLATE(TextureFilename);
    DECLARE_XSTDTEMPLATE(TimedFloatKeys);
    DECLARE_XSTDTEMPLATE(Vector);
    DECLARE_XSTDTEMPLATE(VertexDuplicationIndices);
    DECLARE_XSTDTEMPLATE(VertexElement);
    DECLARE_XSTDTEMPLATE(XSkinMeshHeader);

}}

#endif
