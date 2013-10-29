#include "forg_pch.h"

#include "mesh/xfile/xstdtemplates.h"

#define MAKE_GUID(data1, data2, data3, a, b, c, d, e, f, g, h) {data1, data2, data3, {a,b,c,d,e,f,g,h}}
#define IMPLEMENTATION_STDTEMPLATE_BEGIN(name, guid, open) const xguid XTemplate##name::GUID = guid;\
    const char* XTemplate##name::NAME = #name;\
    XTemplate##name::XTemplate##name () : XTemplate(NAME, GUID, open) {

#define IMPLEMENTATION_STDTEMPLATE_END() }

namespace forg { namespace xfile {
/*
xguid xtemplate_standard_guids[] =
{
	{0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},	//Unknown
	{0x3D82AB4F, 0x62DA, 0x11cf, {0xab, 0x39, 0x00, 0x20, 0xaf, 0x71, 0xe4, 0x33}},	//Animation
	{0x10DD46A8, 0x775B, 0x11CF, {0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3}},	//AnimationKey
	{0xE2BF56C0, 0x840F, 0x11cf, {0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3}},	//AnimationOptions
	{0x3D82AB50, 0x62DA, 0x11cf, {0xAB, 0x39, 0x00, 0x20, 0xAF, 0x71, 0xE4, 0x33}},	//AnimationSet
	{0x9E415A43, 0x7BA6, 0x4a73, {0x87, 0x43, 0xB7, 0x3D, 0x47, 0xE8, 0x84, 0x76}},	//AnimTicksPerSecond
	{0x537da6a0, 0xca37, 0x11d0, {0x94, 0x1c, 0x00, 0x80, 0xc8, 0x0c, 0xfa, 0x7b}},	//Boolean
	{0x4885AE63, 0x78E8, 0x11cf, {0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3}},	//Boolean2d
	{0xD3E16E81, 0x7835, 0x11cf, {0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3}},	//ColorRGB
	{0x35FF44E0, 0x6C7C, 0x11cf, {0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3}},	//ColorRGBA
	{0x7F9B00B3, 0xF125, 0x4890, {0x87, 0x6E, 0x1C, 0x42, 0xBF, 0x69, 0x7C, 0x4D}},	//CompressedAnimationSet
	{0xF6F23F44, 0x7686, 0x11cf, {0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3}},	//Coords2d
	{0xBF22E553, 0x292C, 0x4781, {0x9F, 0xEA, 0x62, 0xBD, 0x55, 0x4B, 0xDD, 0x93}},	//DeclData
	{0x622C0ED0, 0x956E, 0x4da9, {0x90, 0x8A, 0x2A, 0xF9, 0x4F, 0x3C, 0xE7, 0x16}},	//EffectDWord
	{0xF1CFE2B3, 0x0DE3, 0x4e28, {0xAF, 0xA1, 0x15, 0x5A, 0x75, 0x0A, 0x28, 0x2D}},	//EffectFloats
	{0xE331F7E4, 0x0559, 0x4cc2, {0x8E, 0x99, 0x1C, 0xEC, 0x16, 0x57, 0x92, 0x8F}},	//EffectInstance
	{0xE13963BC, 0xAE51, 0x4c5d, {0xB0, 0x0F, 0xCF, 0xA3, 0xA9, 0xD9, 0x7C, 0xE5}},	//EffectParamDWord
	{0x3014B9A0, 0x62F5, 0x478c, {0x9B, 0x86, 0xE4, 0xAC, 0x9F, 0x4E, 0x41, 0x8B}},	//EffectParamFloats
	{0x1DBC4C88, 0x94C1, 0x46ee, {0x90, 0x76, 0x2C, 0x28, 0x81, 0x8C, 0x94, 0x81}},	//EffectParamString
	{0xD55B097E, 0xBDB6, 0x4c52, {0xB0, 0x3D, 0x60, 0x51, 0xC8, 0x9D, 0x0E, 0x42}},	//EffectString
	{0xA64C844A, 0xE282, 0x4756, {0x8B, 0x80, 0x25, 0x0C, 0xDE, 0x04, 0x39, 0x8C}},	//FaceAdjacency
	{0x10DD46A9, 0x775B, 0x11cf, {0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3}},	//FloatKeys
	{0x3d82ab46, 0x62da, 0x11cf, {0xab, 0x39, 0x00, 0x20, 0xaf, 0x71, 0xe4, 0x33}},	//Frame
	{0xF6F23F41, 0x7686, 0x11cf, {0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3}},	//FrameTransformMatrix
	{0xB6E70A0E, 0x8EF9, 0x4e83, {0x94, 0xAD, 0xEC, 0xC8, 0xB0, 0xC0, 0x48, 0x97}},	//FVFData
	{0xa42790e0, 0x7810, 0x11cf, {0x8f, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xa3}},	//Guid
	{0x1630B820, 0x7842, 0x11cf, {0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3}},	//IndexedColor
	{0x3D82AB4D, 0x62DA, 0x11CF, {0xAB, 0x39, 0x00, 0x20, 0xAF, 0x71, 0xE4, 0x33}},	//Material
	{0x4885ae60, 0x78e8, 0x11cf, {0x8f, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xa3}},	//MaterialWrap
	{0xF6F23F45, 0x7686, 0x11cf, {0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3}},	//Matrix4x4
	{0x3D82AB44, 0x62DA, 0x11CF, {0xAB, 0x39, 0x00, 0x20, 0xAF, 0x71, 0xE4, 0x33}},	//Mesh
	{0x3D82AB5F, 0x62DA, 0x11cf, {0xAB, 0x39, 0x00, 0x20, 0xAF, 0x71, 0xE4, 0x33}},	//MeshFace
	{0xED1EC5C0, 0xC0A8, 0x11D0, {0x94, 0x1C, 0x00, 0x80, 0xC8, 0x0C, 0xFA, 0x7B}},	//MeshFaceWraps
	{0xF6F23F42, 0x7686, 0x11CF, {0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3}},	//MeshMaterialList
	{0xF6F23F43, 0x7686, 0x11cf, {0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3}},	//MeshNormals
	{0xF6F23F40, 0x7686, 0x11cf, {0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3}},	//MeshTextureCoords
	{0x1630B821, 0x7842, 0x11cf, {0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3}},	//MeshVertexColors
	{0xA3EB5D44, 0xFC22, 0x429D, {0x9A, 0xFB, 0x32, 0x21, 0xCB, 0x97, 0x19, 0xA6}},	//Patch
	{0xD02C95CC, 0xEDBA, 0x4305, {0x9B, 0x5D, 0x18, 0x20, 0xD7, 0x70, 0x4B, 0xBF}},	//PatchMesh
	{0xB9EC94E1, 0xB9A6, 0x4251, {0xBA, 0x18, 0x94, 0x89, 0x3F, 0x02, 0xC0, 0xEA}},	//PatchMesh9
	{0x917E0427, 0xC61E, 0x4a14, {0x9C, 0x64, 0xAF, 0xE6, 0x5F, 0x9E, 0x98, 0x44}},	//PMAttributeRange
	{0xB6C3E656, 0xEC8B, 0x4b92, {0x9B, 0x62, 0x68, 0x16, 0x59, 0x52, 0x29, 0x47}},	//PMInfo
	{0x574CCC14, 0xF0B3, 0x4333, {0x82, 0x2D, 0x93, 0xE8, 0xA8, 0xA0, 0x8E, 0x4C}},	//PMVSplitRecord
	{0x6F0D123B, 0xBAD2, 0x4167, {0xA0, 0xD0, 0x80, 0x22, 0x4F, 0x25, 0xFA, 0xBB}},	//SkinWeights
	{0xA42790E1, 0x7810, 0x11cf, {0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3}},	//TextureFilename
	{0xF406B180, 0x7B3B, 0x11cf, {0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3}},	//TimedFloatKeys
	{0x3D82AB5E, 0x62DA, 0x11cf, {0xAB, 0x39, 0x00, 0x20, 0xAF, 0x71, 0xE4, 0x33}},	//Vector
	{0xB8D65549, 0xD7C9, 0x4995, {0x89, 0xCF, 0x53, 0xA9, 0xA8, 0xB0, 0x31, 0xE3}},	//VertexDuplicationIndices
	{0xF752461C, 0x1E23, 0x48f6, {0xB9, 0xF8, 0x83, 0x50, 0x85, 0x0F, 0x33, 0x6F}},	//VertexElement
	{0x3CF169CE, 0xFF7C, 0x44ab, {0x93, 0xC0, 0xF7, 0x8F, 0x62, 0xD1, 0x72, 0xE2}}	//XSkinMeshHeader
};
*/


//////////////////////////////////////////////////////////////////////////
// Animation
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( Animation, MAKE_GUID(0x3D82AB4F, 0x62DA, 0x11cf, 0xab, 0x39, 0x00, 0x20, 0xaf, 0x71, 0xe4, 0x33), true )
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// AnimationKey
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( AnimationKey, MAKE_GUID(0x10DD46A8, 0x775B, 0x11CF, 0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "keyType") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nKeys") );
    AddMember( new XTemplateArray("keys", "TimedFloatKeys", "nKeys") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// AnimationOptions
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( AnimationOptions, MAKE_GUID(0xE2BF56C0, 0x840F, 0x11cf, 0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword , "openclosed") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword , "positionquality") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// AnimationSet
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( AnimationSet, MAKE_GUID(0x3D82AB50, 0x62DA, 0x11cf, 0xAB, 0x39, 0x00, 0x20, 0xAF, 0x71, 0xE4, 0x33), false )
    AddOption("AnimationSet", XTemplateAnimation::GUID);
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// Boolean
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( Boolean, MAKE_GUID(0x537da6a0, 0xca37, 0x11d0, 0x94, 0x1c, 0x00, 0x80, 0xc8, 0x0c, 0xfa, 0x7b), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword , "truefalse") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// Boolean2d
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( Boolean2d, MAKE_GUID(0x4885AE63, 0x78E8, 0x11cf, 0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3), false )
    AddMember( new XTemplateReference("Boolean", "u") );
    AddMember( new XTemplateReference("Boolean", "v") );
IMPLEMENTATION_STDTEMPLATE_END()


//////////////////////////////////////////////////////////////////////////
// AnimTicksPerSecond
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( AnimTicksPerSecond, MAKE_GUID(0x9E415A43, 0x7BA6, 0x4a73, 0x87, 0x43, 0xB7, 0x3D, 0x47, 0xE8, 0x84, 0x76), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword , "AnimTicksPerSecond") );
IMPLEMENTATION_STDTEMPLATE_END()


//////////////////////////////////////////////////////////////////////////
// ColorRGB
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( ColorRGB, MAKE_GUID(0xD3E16E81, 0x7835, 0x11cf, 0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Float, "red") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Float, "green") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Float, "blue") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// ColorRGBA
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( ColorRGBA, MAKE_GUID(0x35FF44E0, 0x6C7C, 0x11cf, 0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Float, "red") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Float, "green") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Float, "blue") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Float, "alpha") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// CompressedAnimationSet
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( CompressedAnimationSet, MAKE_GUID(0x7F9B00B3, 0xF125, 0x4890, 0x87, 0x6E, 0x1C, 0x42, 0xBF, 0x69, 0x7C, 0x4D), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "CompressedBlockSize") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Float, "TicksPerSec") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "PlaybackType") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "BufferLength") );
    AddMember( new XTemplateArray("CompressedData", ETemplatePrimitiveType::Dword, "BufferLength") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// Coords2d
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( Coords2d, MAKE_GUID(0xF6F23F44, 0x7686, 0x11cf, 0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Float, "u") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Float, "v") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// DeclData
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( DeclData, MAKE_GUID(0xBF22E553, 0x292C, 0x4781, 0x9F, 0xEA, 0x62, 0xBD, 0x55, 0x4B, 0xDD, 0x93), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nElements") );
    AddMember( new XTemplateArray("Elements", "VertexElement", "nElements") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nDWords") );
    AddMember( new XTemplateArray("data", ETemplatePrimitiveType::Dword, "nDWords") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// EffectDWord
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( EffectDWord, MAKE_GUID(0x622C0ED0, 0x956E, 0x4da9, 0x90, 0x8A, 0x2A, 0xF9, 0x4F, 0x3C, 0xE7, 0x16), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword , "Value") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// EffectFloats
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( EffectFloats, MAKE_GUID(0xF1CFE2B3, 0x0DE3, 0x4e28, 0xAF, 0xA1, 0x15, 0x5A, 0x75, 0x0A, 0x28, 0x2D), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword , "nFloats") );
    AddMember( new XTemplateArray("Floats", ETemplatePrimitiveType::Float, "nFloats") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// EffectInstance
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( EffectInstance, MAKE_GUID(0xE331F7E4, 0x0559, 0x4cc2, 0x8E, 0x99, 0x1C, 0xEC, 0x16, 0x57, 0x92, 0x8F), true )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Lpstr, "EffectFilename") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// EffectParamDWord
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( EffectParamDWord, MAKE_GUID(0x622C0ED0, 0x956E, 0x4da9, 0x90, 0x8A, 0x2A, 0xF9, 0x4F, 0x3C, 0xE7, 0x16), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Lpstr, "ParamName") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "Value") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// EffectParamFloats
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( EffectParamFloats, MAKE_GUID(0x3014B9A0, 0x62F5, 0x478c, 0x9B, 0x86, 0xE4, 0xAC, 0x9F, 0x4E, 0x41, 0x8B), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Lpstr, "ParamName") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nFloats") );
    AddMember( new XTemplateArray("Floats", ETemplatePrimitiveType::Float, "nFloats") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// EffectParamString
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( EffectParamString, MAKE_GUID(0x1DBC4C88, 0x94C1, 0x46ee, 0x90, 0x76, 0x2C, 0x28, 0x81, 0x8C, 0x94, 0x81), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Lpstr, "ParamName") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Lpstr, "Value") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// EffectString
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( EffectString, MAKE_GUID(0xD55B097E, 0xBDB6, 0x4c52, 0xB0, 0x3D, 0x60, 0x51, 0xC8, 0x9D, 0x0E, 0x42), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Lpstr, "Value") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// FaceAdjacency
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( FaceAdjacency, MAKE_GUID(0xA64C844A, 0xE282, 0x4756, 0x8B, 0x80, 0x25, 0x0C, 0xDE, 0x04, 0x39, 0x8C), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nIndices") );
    AddMember( new XTemplateArray("indices", ETemplatePrimitiveType::Dword, "nIndices") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// FloatKeys
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( FloatKeys, MAKE_GUID(0x10DD46A9, 0x775B, 0x11cf, 0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nValues") );
    AddMember( new XTemplateArray("values", ETemplatePrimitiveType::Float, "nValues") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// Frame
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( Frame, MAKE_GUID(0x3d82ab46, 0x62da, 0x11cf, 0xab, 0x39, 0x00, 0x20, 0xaf, 0x71, 0xe4, 0x33), true )
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// FrameTransformMatrix
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( FrameTransformMatrix, MAKE_GUID(0xF6F23F41, 0x7686, 0x11cf, 0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3), false )
    AddMember( new XTemplateReference("Matrix4x4", "frameMatrix") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// FVFData
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( FVFData, MAKE_GUID(0xB6E70A0E, 0x8EF9, 0x4e83, 0x94, 0xAD, 0xEC, 0xC8, 0xB0, 0xC0, 0x48, 0x97), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "dwFVF") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nDWords") );
    AddMember( new XTemplateArray("data", ETemplatePrimitiveType::Dword, "nDWords") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// Guid
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( Guid, MAKE_GUID(0xa42790e0, 0x7810, 0x11cf, 0x8f, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xa3), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "data1") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Word, "data2") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Word, "data3") );
    AddMember( new XTemplateArray("data4", ETemplatePrimitiveType::UChar, 8) );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// IndexedColor
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( IndexedColor, MAKE_GUID(0x1630B820, 0x7842, 0x11cf, 0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "index") );
    AddMember( new XTemplateReference("ColorRGBA", "indexColor") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// Material
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( Material, MAKE_GUID(0x3D82AB4D, 0x62DA, 0x11CF, 0xAB, 0x39, 0x00, 0x20, 0xAF, 0x71, 0xE4, 0x33), true )
    AddMember( new XTemplateReference("ColorRGBA", "faceColor") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Float, "power") );
    AddMember( new XTemplateReference("ColorRGB", "specularColor") );
    AddMember( new XTemplateReference("ColorRGB", "emissiveColor") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// MaterialWrap
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( MaterialWrap, MAKE_GUID(0x4885ae60, 0x78e8, 0x11cf, 0x8f, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xa3), false )
    AddMember( new XTemplateReference("Boolean", "u") );
    AddMember( new XTemplateReference("Boolean", "v") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// Matrix4x4
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( Matrix4x4, MAKE_GUID(0xF6F23F45, 0x7686, 0x11cf, 0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3), false )
    AddMember( new XTemplateArray("matrix", ETemplatePrimitiveType::Float, 16) );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// Mesh
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( Mesh, MAKE_GUID(0x3D82AB44, 0x62DA, 0x11CF, 0xAB, 0x39, 0x00, 0x20, 0xAF, 0x71, 0xE4, 0x33), true )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nVertices") );
    AddMember( new XTemplateArray("vertices", "Vector", "nVertices") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nFaces") );
    AddMember( new XTemplateArray("faces", "MeshFace", "nFaces") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// MeshFace
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( MeshFace, MAKE_GUID(0x3D82AB5F, 0x62DA, 0x11cf, 0xAB, 0x39, 0x00, 0x20, 0xAF, 0x71, 0xE4, 0x33), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nFaceVertexIndices") );
    AddMember( new XTemplateArray("faceVertexIndices", ETemplatePrimitiveType::Dword, "nFaceVertexIndices") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// MeshFaceWraps
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( MeshFaceWraps, MAKE_GUID(0xED1EC5C0, 0xC0A8, 0x11D0, 0x94, 0x1C, 0x00, 0x80, 0xC8, 0x0C, 0xFA, 0x7B), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nFaceWrapValues") );
    AddMember( new XTemplateArray("faceWrapValues", "Boolean2d", "nFaceWrapValues") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// MeshMaterialList
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( MeshMaterialList, MAKE_GUID(0xF6F23F42, 0x7686, 0x11CF, 0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nMaterials") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nFaceIndexes") );
    AddMember( new XTemplateArray("faceIndexes", ETemplatePrimitiveType::Dword, "nFaceIndexes") );

    AddOption("Material", XTemplateMaterial::GUID);
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// MeshNormals
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( MeshNormals, MAKE_GUID(0xF6F23F43, 0x7686, 0x11cf, 0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nNormals") );
    AddMember( new XTemplateArray("normals", "Vector", "nNormals") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nFaceNormals") );
    AddMember( new XTemplateArray("faceNormals", "MeshFace", "nFaceNormals") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// MeshTextureCoords
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( MeshTextureCoords, MAKE_GUID(0xF6F23F40, 0x7686, 0x11cf, 0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nTextureCoords") );
    AddMember( new XTemplateArray("textureCoords", "Coords2d", "nTextureCoords") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// MeshVertexColors
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( MeshVertexColors, MAKE_GUID(0x1630B821, 0x7842, 0x11cf, 0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nVertexColors") );
    AddMember( new XTemplateArray("vertexColors", "IndexedColor", "nVertexColors") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// Patch
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( Patch, MAKE_GUID(0xA3EB5D44, 0xFC22, 0x429D, 0x9A, 0xFB, 0x32, 0x21, 0xCB, 0x97, 0x19, 0xA6), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nControlIndices") );
    AddMember( new XTemplateArray("controlIndices", ETemplatePrimitiveType::Dword, "nControlIndices") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// PatchMesh
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( PatchMesh, MAKE_GUID(0xD02C95CC, 0xEDBA, 0x4305, 0x9B, 0x5D, 0x18, 0x20, 0xD7, 0x70, 0x4B, 0xBF), true )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nVertices") );
    AddMember( new XTemplateArray("vertices", "Vector", "nVertices") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nPatches") );
    AddMember( new XTemplateArray("patches", "Patch", "nPatches") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// PatchMesh9
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( PatchMesh9, MAKE_GUID(0xB9EC94E1, 0xB9A6, 0x4251, 0xBA, 0x18, 0x94, 0x89, 0x3F, 0x02, 0xC0, 0xEA), true )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "Type") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "Degree") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "Basis") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nVertices") );
    AddMember( new XTemplateArray("vertices", "Vector", "nVertices") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nPatches") );
    AddMember( new XTemplateArray("patches", "Patch", "nPatches") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// PMAttributeRange
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( PMAttributeRange, MAKE_GUID(0x917E0427, 0xC61E, 0x4a14, 0x9C, 0x64, 0xAF, 0xE6, 0x5F, 0x9E, 0x98, 0x44), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "iFaceOffset") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nFacesMin") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nFacesMax") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "iVertexOffset") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nVerticesMin") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nVerticesMax") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// PMInfo
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( PMInfo, MAKE_GUID(0xB6C3E656, 0xEC8B, 0x4b92, 0x9B, 0x62, 0x68, 0x16, 0x59, 0x52, 0x29, 0x47), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nAttributes") );
    AddMember( new XTemplateArray("attributeRanges", "PMAttributeRange", "nAttributes") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nMaxValence") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nMinLogicalVertices") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nMaxLogicalVertices") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nVSplits") );
    AddMember( new XTemplateArray("splitRecords", "PMVSplitRecord", "nVSplits") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nAttributeMispredicts") );
    AddMember( new XTemplateArray("attributeMispredicts", ETemplatePrimitiveType::Dword, "nAttributeMispredicts") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// PMVSplitRecord
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( PMVSplitRecord, MAKE_GUID(0x574CCC14, 0xF0B3, 0x4333, 0x82, 0x2D, 0x93, 0xE8, 0xA8, 0xA0, 0x8E, 0x4C), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "iFaceCLW") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "iVlrOffset") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "iCode") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// SkinWeights
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( SkinWeights, MAKE_GUID(0x6F0D123B, 0xBAD2, 0x4167, 0xA0, 0xD0, 0x80, 0x22, 0x4F, 0x25, 0xFA, 0xBB), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Lpstr, "transformNodeName") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nWeights") );
    AddMember( new XTemplateArray("vertexIndices", ETemplatePrimitiveType::Dword, "nWeights") );
    AddMember( new XTemplateArray("weights", ETemplatePrimitiveType::Float, "nWeights") );
    AddMember( new XTemplateReference("Matrix4x4", "matrixOffset") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// TextureFilename
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( TextureFilename, MAKE_GUID(0xA42790E1, 0x7810, 0x11cf, 0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Lpstr, "filename") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// TimedFloatKeys
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( TimedFloatKeys, MAKE_GUID(0xF406B180, 0x7B3B, 0x11cf, 0x8F, 0x52, 0x00, 0x40, 0x33, 0x35, 0x94, 0xA3), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword , "time") );
    AddMember( new XTemplateReference("FloatKeys", "tfkeys") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// Vector
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( Vector, MAKE_GUID(0x3D82AB5E, 0x62DA, 0x11cf, 0xAB, 0x39, 0x00, 0x20, 0xAF, 0x71, 0xE4, 0x33), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Float, "x") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Float, "y") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Float, "z") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// VertexDuplicationIndices
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( VertexDuplicationIndices, MAKE_GUID(0xB8D65549, 0xD7C9, 0x4995, 0x89, 0xCF, 0x53, 0xA9, 0xA8, 0xB0, 0x31, 0xE3), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nIndices") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "nOriginalVertices") );
    AddMember( new XTemplateArray("indices", ETemplatePrimitiveType::Dword, "nIndices") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// VertexElement
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( VertexElement, MAKE_GUID(0xF752461C, 0x1E23, 0x48f6, 0xB9, 0xF8, 0x83, 0x50, 0x85, 0x0F, 0x33, 0x6F), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "Type") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "Method") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "Usage") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Dword, "UsageIndex") );
IMPLEMENTATION_STDTEMPLATE_END()

//////////////////////////////////////////////////////////////////////////
// XSkinMeshHeader
//////////////////////////////////////////////////////////////////////////

IMPLEMENTATION_STDTEMPLATE_BEGIN( XSkinMeshHeader, MAKE_GUID(0x3CF169CE, 0xFF7C, 0x44ab, 0x93, 0xC0, 0xF7, 0x8F, 0x62, 0xD1, 0x72, 0xE2), false )
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Word, "nMaxSkinWeightsPerVertex") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Word, "nMaxSkinWeightsPerFace") );
    AddMember( new XTemplatePrimitive(ETemplatePrimitiveType::Word, "nBones") );
IMPLEMENTATION_STDTEMPLATE_END()


}}

