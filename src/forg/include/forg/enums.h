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

#ifndef _FORG_ENUMS_H_
#define _FORG_ENUMS_H_

#if _MSC_VER > 1000
#pragma once
#endif

namespace forg {

/// View types enum
/**
* Types of camera views
*/
enum CameraView
{
	/// Perspective view
	Perspective = 0,
	Orthogonal,
};

/// Defines the light type.
namespace LightType {
enum
{
    /// Light is a point source. The light has a position in space and radiates light in all directions.
    Point = 1,
    /**
    * Light is a spotlight source. This light is like a point light, except that the illumination is limited to a cone.
    * This light type has a direction and several other parameters that determine the shape of the cone it produces.
    */
    Spot = 2,
    /// Light is a directional light source. This is equivalent to using a point light source at an infinite distance.
    Directional = 3,
};
};

/// Defines various types of surface formats.
namespace Format {
enum EType
{
	/// Unknown surface format.
	Unknown = 0,
	/// A 24-bit RGB pixel format that uses 8 bits per channel.
	R8G8B8 = 0x14,
	/// A 32-bit ARGB pixel format, with alpha, that uses 8 bits per channel.
	A8R8G8B8 = 0x15,
	/// A 32-bit RGB pixel format that reserves 8 bits for each color.
	X8R8G8B8 = 0x16,
	/// A 16-bit RGB pixel format that uses 5 bits for red, 6 bits for green, and 5 bits for blue.
	R5G6B5 = 0x17,
    /// 8-bit alpha only.
	A8 = 0x1c,
    /// 32-bit ARGB pixel format with alpha, using 8 bits per channel.
	A8B8G8R8 = 0x20,
    /// 8-bit color indexed.
	P8 = 0x29,
    /// 8-bit luminance only.
	L8 = 0x32,
    A8L8 = 0x33,

	//A16B16G16R16 = 0x24,
	//A16B16G16R16F = 0x71,
	//A1R5G5B5 = 0x19,
	//A2B10G10R10 = 0x1f,
	//A2R10G10B10 = 0x23,
	//A2W10V10U10 = 0x43,
	//A32B32G32R32F = 0x74,
	//A4L4 = 0x34,
	//A4R4G4B4 = 0x1a,
	//A8L8 = 0x33,
	//A8P8 = 40,
	//A8R3G3B2 = 0x1d,
	//CxV8U8 = 0x75,
	//D15S1 = 0x49,
	//D16 = 80,
	//D16Lockable = 70,
	//D24S8 = 0x4b,
	//D24SingleS8 = 0x53,
	//D24X4S4 = 0x4f,
	//D24X8 = 0x4d,
	//D32 = 0x47,
	//D32SingleLockable = 0x52,
	//Dxt1 = 0x31545844,
	//Dxt2 = 0x32545844,
	//Dxt3 = 0x33545844,
	//Dxt4 = 0x34545844,
	//Dxt5 = 0x35545844,
	//G16R16 = 0x22,
	//G16R16F = 0x70,
	//G32R32F = 0x73,
	//G8R8G8B8 = 0x42475247,
	//L16 = 0x51,
	//L6V5U5 = 0x3d,
	//Multi2Argb8 = 0x3154454d,
	//Q16W16V16U16 = 110,
	//Q8W8V8U8 = 0x3f,
	//R16F = 0x6f,
	//R32F = 0x72,
	//R3G3B2 = 0x1b,
	//R8G8B8G8 = 0x47424752,
	//Uyvy = 0x59565955,
	//V16U16 = 0x40,
	//V8U8 = 60,
	//VertexData = 100,
	//X1R5G5B5 = 0x18,
	//X4R4G4B4 = 30,
	//X8B8G8R8 = 0x21,
	//X8L8V8U8 = 0x3e,
	//Yuy2 = 0x32595559
};
}


/// Declaration type.
enum DeclarationType
{
	DeclarationType_Float1 = 0,
	DeclarationType_Float2 = 1,
	DeclarationType_Float3 = 2,
	DeclarationType_Float4 = 3,
	DeclarationType_Color = 4,
	DeclarationType_Ubyte4 = 5,
	DeclarationType_Short2 = 6,
	DeclarationType_Short4 = 7,
	DeclarationType_Ubyte4N = 8,
	DeclarationType_Short2N = 9,
	DeclarationType_Short4N = 10,
	DeclarationType_UShort2N = 11,
	DeclarationType_UShort4N = 12,
	DeclarationType_UDec3 = 13,
	DeclarationType_Dec3N = 14,
	DeclarationType_Float16Two = 15,
	DeclarationType_Float16Four = 0x10,
	DeclarationType_Unused = 0x11
};

/// Declaration Usage
enum DeclarationUsage
{
	DeclarationUsage_Position = 0,
	DeclarationUsage_BlendWeight = 1,
	DeclarationUsage_BlendIndices = 2,
	DeclarationUsage_Normal = 3,
	DeclarationUsage_PointSize = 4,
	DeclarationUsage_TextureCoordinate = 5,
	DeclarationUsage_Tangent = 6,
	DeclarationUsage_BiNormal = 7,
	DeclarationUsage_TessellateFactor = 8,
	DeclarationUsage_PositionTransformed = 9,
	DeclarationUsage_Color = 10,
	DeclarationUsage_Fog = 11,
	DeclarationUsage_Depth = 12,
	DeclarationUsage_Sample = 13
};

/// These flags identify a surface to reset when calling Clear.
enum ClearFlags
{
	/// Clear a render target, or all targets in a multiple render target.
	ClearFlags_Target = 1,
	/// Clear the depth buffer.
	ClearFlags_ZBuffer = 2,
	/// Clear the stencil buffer.
	ClearFlags_Stencil = 4
};

/// Defines the primitives supported.
enum PrimitiveType
{
	PrimitiveType_PointList = 1,
	PrimitiveType_LineList,
	PrimitiveType_LineStrip,
	PrimitiveType_TriangleList,
	PrimitiveType_TriangleStrip,
	PrimitiveType_TriangleFan
};

/// Transformation Type
enum TransformType
{
	TransformType_View = 2,
	TransformType_Projection = 3,

	TransformType_Texture0 = 0x10,
	TransformType_Texture1 = 0x11,
	TransformType_Texture2 = 0x12,
	TransformType_Texture3 = 0x13,
	TransformType_Texture4 = 20,
	TransformType_Texture5 = 0x15,
	TransformType_Texture6 = 0x16,
	TransformType_Texture7 = 0x17,

	TransformType_World = 0x100,
	TransformType_World1 = 0x101,
	TransformType_World2 = 0x102,
	TransformType_World3 = 0x103
};

enum Cull
{
	Cull_None = 1,
	Cull_Clockwise = 2,
	Cull_CounterClockwise = 3
};

enum FillMode
{
	FillMode_Point = 1,
	FillMode_WireFrame = 2,
	FillMode_Solid = 3
};

enum ShadeMode
{
	ShadeMode_Flat = 1,
	ShadeMode_Gouraud = 2
};

enum Blend
{
    Blend_BlendFactor = 14,
    Blend_BothInvSourceAlpha = 13,
    Blend_BothSourceAlpha = 12,
    Blend_DestinationAlpha = 7,
    Blend_DestinationColor = 9,
    Blend_InvBlendFactor = 15,
    Blend_InvDestinationAlpha = 8,
    Blend_InvDestinationColor = 10,
    Blend_InvSourceAlpha = 6,
    Blend_InvSourceColor = 4,
    Blend_One = 2,
    Blend_SourceAlpha = 5,
    Blend_SourceAlphaSat = 11,
    Blend_SourceColor = 3,
    Blend_Zero = 1
};

/// Defines device render states.
enum RenderStates
{
    RenderStates_AlphaBlendEnable = 0x1b,
    RenderStates_AlphaFunction = 0x19,
    RenderStates_AlphaTestEnable = 15,
    RenderStates_BlendFactor = 0xc1,
    RenderStates_BlendOperation = 0xab,
    RenderStates_BlendOperationAlpha = 0xd1,
    RenderStates_SourceBlend = 0x13,
    RenderStates_DestinationBlend = 20,
    RenderStates_CullMode = 0x16,
	RenderStates_FillMode = 8,
    RenderStates_Lighting = 0x89,
    RenderStates_NormalizeNormals = 0x8f,
	RenderStates_ShadeMode = 9,

	//AdaptiveTessellateW = 0xb7,
	//AdaptiveTessellateX = 180,
	//AdaptiveTessellateY = 0xb5,
	//AdaptiveTessellateZ = 0xb6,
	//AlphaBlendEnable = 0x1b,
	//AlphaFunction = 0x19,
	//AlphaTestEnable = 15,
	//Ambient = 0x8b,
	//AmbientMaterialSource = 0x93,
	//AntialiasedLineEnable = 0xb0,
	//BlendFactor = 0xc1,
	//BlendOperation = 0xab,
	//BlendOperationAlpha = 0xd1,
	//Clipping = 0x88,
	//ClipPlaneEnable = 0x98,
	//ColorVertex = 0x8d,
	//ColorWriteEnable = 0xa8,
	//ColorWriteEnable1 = 190,
	//ColorWriteEnable2 = 0xbf,
	//ColorWriteEnable3 = 0xc0,
	//CounterClockwiseStencilFail = 0xba,
	//CounterClockwiseStencilFunction = 0xbd,
	//CounterClockwiseStencilPass = 0xbc,
	//CounterClockwiseStencilZBufferFail = 0xbb,
	//CullMode = 0x16,
	//DebugMonitorToken = 0xa5,
	//DepthBias = 0xc3,
	//DestinationBlend = 20,
	//DestinationBlendAlpha = 0xd0,
	//DiffuseMaterialSource = 0x91,
	//DitherEnable = 0x1a,
	//EmissiveMaterialSource = 0x94,
	//EnableAdaptiveTessellation = 0xb8,
	//FillMode = 8,
	//FogColor = 0x22,
	//FogDensity = 0x26,
	//FogEnable = 0x1c,
	//FogEnd = 0x25,
	//FogStart = 0x24,
	//FogTableMode = 0x23,
	//FogVertexMode = 140,
	//IndexedVertexBlendEnable = 0xa7,
	//LastPixel = 0x10,
	//Lighting = 0x89,
	//LocalViewer = 0x8e,
	//MaxTessellationLevel = 0xb3,
	//MinTessellationLevel = 0xb2,
	//MultisampleAntiAlias = 0xa1,
	//MultisampleMask = 0xa2,
	//NormalDegree = 0xad,
	//NormalizeNormals = 0x8f,
	//PatchEdgeStyle = 0xa3,
	//PointScaleA = 0x9e,
	//PointScaleB = 0x9f,
	//PointScaleC = 160,
	//PointScaleEnable = 0x9d,
	//PointSize = 0x9a,
	//PointSizeMax = 0xa6,
	//PointSizeMin = 0x9b,
	//PointSpriteEnable = 0x9c,
	//PositionDegree = 0xac,
	//RangeFogEnable = 0x30,
	//ReferenceAlpha = 0x18,
	//ReferenceStencil = 0x39,
	//ScissorTestEnable = 0xae,
	//SeparateAlphaBlendEnable = 0xce,
	//ShadeMode = 9,
	//SlopeScaleDepthBias = 0xaf,
	//SourceBlend = 0x13,
	//SourceBlendAlpha = 0xcf,
	//SpecularEnable = 0x1d,
	//SpecularMaterialSource = 0x92,
	//SrgbWriteEnable = 0xc2,
	//StencilEnable = 0x34,
	//StencilFail = 0x35,
	//StencilFunction = 0x38,
	//StencilMask = 0x3a,
	//StencilPass = 0x37,
	//StencilWriteMask = 0x3b,
	//StencilZBufferFail = 0x36,
	//TextureFactor = 60,
	//TweenFactor = 170,
	//TwoSidedStencilMode = 0xb9,
	//VertexBlend = 0x97,
	//Wrap0 = 0x80,
	//Wrap1 = 0x81,
	//Wrap10 = 200,
	//Wrap11 = 0xc9,
	//Wrap12 = 0xca,
	//Wrap13 = 0xcb,
	//Wrap14 = 0xcc,
	//Wrap15 = 0xcd,
	//Wrap2 = 130,
	//Wrap3 = 0x83,
	//Wrap4 = 0x84,
	//Wrap5 = 0x85,
	//Wrap6 = 0x86,
	//Wrap7 = 0x87,
	//Wrap8 = 0xc6,
	//Wrap9 = 0xc7,
	//ZBufferFunction = 0x17,
	//ZBufferWriteEnable = 14,
	//ZEnable = 7
};

/// Defines the memory class that holds buffers for a resource.
enum Pool
{
	Pool_Default,
	Pool_Managed,
	Pool_SystemMemory,
	Pool_Scratch
};

/// Defines supported usage types for the current resource.
namespace Usage
{
enum
{
	Usage_None = 0,
	//Usage_RenderTarget = 1,
	//Usage_DepthStencil = 2,
	WriteOnly = 8,
	//Usage_SoftwareProcessing = 0x10,
	//Usage_DoNotClip = 0x20,
	//Usage_Points = 0x40,
	//Usage_RTPatches = 0x80,
	//Usage_NPatches = 0x100,
	//Usage_Dynamic = 0x200,
	AutoGenerateMipMap = 0x400,
	//Usage_QueryDisplacementMap = 0x4000,
	//Usage_QueryLegacyBumpMap = 0x8000,
	//Usage_QuerySrgbRead = 0x10000,
	//Usage_QueryFilter = 0x20000,
	//Usage_QuerySrgbWrite = 0x40000,
	//Usage_QueryPostPixelShaderBlending = 0x80000,
	//Usage_QueryVertexTexture = 0x100000,
	//Usage_QueryWrapAndMip = 0x200000,
};
}

namespace LockFlags
{
enum
{
	LockFlags_None = 0,
	ReadOnly = 0x10,
	NoSystemLock = 0x800,
	NoOverwrite = 0x1000,
	Discard = 0x2000,
	DoNotWait = 0x4000,
	NoDirtyUpdate = 0x8000
};
}

namespace MeshFlags {
enum
{
    Use32Bit = 1,

//     DoNotClip = 2,
//     Dynamic = 0x880,
//     IbDynamic = 0x800,
//     IbManaged = 0x200,
//     IbSoftwareProcessing = 0x10000,
//     IbSystemMem = 0x100,
//     IbWriteOnly = 0x400,
//     Managed = 0x220,
//     NPatches = 0x4000,
//     OptimizeAttributeSort = 0x2000000,
//     OptimizeCompact = 0x1000000,
//     OptimizeDeviceIndependent = 0x400000,
//     OptimizeDoNotSplit = 0x20000000,
//     OptimizeIgnoreVerts = 0x10000000,
//     OptimizeStripeReorder = 0x8000000,
//     OptimizeVertexCache = 0x4000000,
//     Points = 4,
//     RtPatches = 8,
//     SimplifyFace = 2,
//     SimplifyVertex = 1,
//     SoftwareProcessing = 0x18000,
//     SystemMemory = 0x110,
//     UseHardwareOnly = 0x2000,
//     VbDynamic = 0x80,
//     VbManaged = 0x20,
//     VbShare = 0x1000,
//     VbSoftwareProcessing = 0x8000,
//     VbSystemMem = 0x10,
//     VbWriteOnly = 0x40,
//     WriteOnly = 0x440
};
};

namespace TangentOptions {
enum {
    WrapU = 1,
    WrapV = 2,
    WrapUV = 3,
    DontNormalizePartials = 4,
    DontOrthogonalize = 8,
    OrthogonalizeFromV = 0x10,
    OrthogonalizeFromU = 0x20,
    WeightByArea = 0x40,
    WeightEqual = 0x80,
    WindClockwise = 0x100,
    CalculateNormals = 0x200,
    GenerateInPlace = 0x400,
};
};

/// The following flags are used to specify sprite rendering options
namespace SpriteFlags {
enum
{
    SpriteFlags_None = 0,
    /// The device state is not to be saved or restored
    DoNotSaveState = 1,
    /// The device render state is not to be changed
    DoNotModifyRenderState = 2,
    /** The world, view, and projection transforms are not modified.
    * The transforms currently set to the device are used to transform the sprites
    * when the batched sprites are drawn. If this flag is not specified,
    * then world, view, and projection transforms are modified so that sprites
    * are drawn in screen-space coordinates.
    */
    ObjectSpace = 4,
    /// Each sprite will be rotated about its center so that it is facing the viewer
    Billboard = 8,
    /// Enables alpha blending
    AlphaBlend = 0x10,
    /// Sort sprites by texture prior to drawing
    SortTexture = 0x20,
    /// Sprites are sorted by depth in front-to-back order prior to drawing
    SortDepthFrontToBack = 0x40,
    /// Sprites are sorted by depth in back-to-front order prior to drawing
    SortDepthBackToFront = 0x80,
};
}


}

#endif //_FORG_ENUMS_H_
