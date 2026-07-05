// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2005 Slawomir Strumecki

#pragma once
#include "base.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "rendering/VertexElement.h"

namespace forg::geometry {

using namespace forg::math;

struct FORG_API PositionOnly
{
    float X;
    float Y;
    float Z;

    static const int StrideSize;
    static VertexElement Declaration[];

    PositionOnly(const Vector3& value);
    PositionOnly(float xvalue = 0.0f, float yvalue = 0.0f, float zvalue = 0.0f);
};

struct FORG_API PositionTextured
{
    float X;
    float Y;
    float Z;
    float Tu;
    float Tv;

    static const int StrideSize;
    static VertexElement Declaration[];

    PositionTextured() {};
    PositionTextured(const Vector3& pos, float u, float v);
    PositionTextured(float xvalue, float yvalue, float zvalue, float u,
                     float v);

    void set_Position(float fX, float fY, float fZ)
    {
        X = fX;
        Y = fY;
        Z = fZ;
    };
    void set_TexCoords(float _u, float _v)
    {
        Tu = _u;
        Tv = _v;
    };
};

struct FORG_API PositionColored
{
    float X;
    float Y;
    float Z;
    int Color; // ARGB

    static const int StrideSize;
    static VertexElement Declaration[];

    PositionColored(const Vector3& value, int c);
    PositionColored(float xvalue = 0.0f, float yvalue = 0.0f,
                    float zvalue = 0.0f, int c = 0);

    void set_Position(const Vector3& value);
    void set_Position(float fX, float fY, float fZ);

    void set_Color(int iColor);
};

struct FORG_API PositionColoredTextured
{
    float X;
    float Y;
    float Z;
    int Color;
    float Tu;
    float Tv;

    static const int StrideSize;
    static VertexElement Declaration[];

    /*
        PositionColoredTextured(const Vector3& value, int c, float u, float v);
        PositionColoredTextured(
                float xvalue = 0.0f, float yvalue = 0.0f, float zvalue = 0.0f,
                int c = 0,
                float u = 0.0f, float v = 0.0f
                );
    */

    void set_Position(float fX, float fY, float fZ)
    {
        X = fX;
        Y = fY;
        Z = fZ;
    };
    void set_Color(int iColor) { Color = iColor; };
    void set_TexCoords(float _u, float _v)
    {
        Tu = _u;
        Tv = _v;
    };
};

struct FORG_API PositionNormal
{
    Vector3 Position;
    Vector3 Normal;

    static const int StrideSize;
    static VertexElement Declaration[];

    PositionNormal(const Vector3& pos, const Vector3& nor);
    PositionNormal(float xvalue = 0.0f, float yvalue = 0.0f,
                   float zvalue = 0.0f, float nxvalue = 0.0f,
                   float nyvalue = 0.0f, float nzvalue = 0.0f);

    void set_Position(const Vector3& value);
    void set_Normal(const Vector3& value);
};

struct FORG_API PositionNormalTextured
{
    Vector3 Position;
    Vector3 Normal;
    float Tu;
    float Tv;

    static const int StrideSize;
    static VertexElement Declaration[];

    PositionNormalTextured(const Vector3& pos, const Vector3& nor, float u,
                           float v);
    PositionNormalTextured(float xvalue = 0.0f, float yvalue = 0.0f,
                           float zvalue = 0.0f, float nxvalue = 0.0f,
                           float nyvalue = 0.0f, float nzvalue = 0.0f,
                           float u = 0.0f, float v = 0.0f);

    void set_Position(const Vector3& value);
    void set_Position(float fX, float fY, float fZ)
    {
        Position.X = fX;
        Position.Y = fY;
        Position.Z = fZ;
    }
    void set_Normal(const Vector3& value);
    void set_Texel(const Vector2& value);
    void set_Texel(float u, float v);
};

} // namespace forg::geometry
