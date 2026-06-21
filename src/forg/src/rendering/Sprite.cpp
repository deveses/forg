#include "forg_pch.h"
#include "core/auto_ptr.hpp"
#include "rendering/Sprite.h"
#include "rendering/Vertex.h"
#include "math/Math.h"

namespace forg {

Sprite* Sprite::CreateSprite(IRenderDevice* device)
{
    Sprite* spr = new Sprite();

    device->AddRef();
    spr->m_device = device;

    return spr;
}

Sprite::Sprite()
{
    m_Flags = 0;
    m_device = 0;
}

Sprite::~Sprite()
{
    if (m_device)
    {
        m_device->Release();
        m_device = 0;
    }
}

int Sprite::SetTransform(const Matrix4* transform)
{
    if (transform)
    {
        // TODO: could be some memcopy
        m_transform = *transform;
    } else
    {
        m_transform = Matrix4::Identity;
    }

    return FORG_OK;
}

int Sprite::Begin(uint Flags)
{
    m_Flags = Flags;

    m_device->SetRenderState(RenderStates_Lighting, false);
    m_device->SetRenderState(RenderStates_CullMode, Cull_None);

    if (m_Flags & SpriteFlags::AlphaBlend)
    {
        m_device->SetRenderState(RenderStates_AlphaBlendEnable, true);
    }

    return FORG_OK;
}

// TODO: restore render states
int Sprite::End()
{
    Flush();

    m_device->SetRenderState(RenderStates_Lighting, true);
    m_device->SetRenderState(RenderStates_AlphaBlendEnable, false);
    m_device->SetRenderState(RenderStates_CullMode, Cull_CounterClockwise);

    return FORG_OK;
}

int Sprite::Flush()
{
    return FORG_OK;
}


int Sprite::Draw(ITexture* srcTexture,
                  const Rectangle* srcRectangle,
                  const Vector3* center,
                  const Vector3* position,
                  Color4b color)
{
	if (!srcTexture)
		return FORG_INVALID_CALL;

    uint tex_width = 0;
    uint tex_height = 0;

    SurfaceDescription desc;

    srcTexture->GetLevelDesc(0, &desc);
    tex_width = desc.Width;
    tex_height = desc.Height;

    Viewport vp;
    m_device->GetViewport(&vp);

    float sprite_width = tex_width;
    float sprite_height = tex_height;
    float u0 = 0.0f;
    float u1 = 1.0f;
    float v0 = 0.0f;
    float v1 = 1.0f;

    if (srcRectangle != 0)
    {
        u0 = float(srcRectangle->left) / tex_width;
        u1 = float(srcRectangle->right) / tex_width;
        v0 = float(srcRectangle->top) / tex_height;
        v1 = float(srcRectangle->bottom) / tex_height;

        sprite_width = srcRectangle->right - srcRectangle->left;
        sprite_height = srcRectangle->bottom - srcRectangle->top;
    }

    float absw = sprite_width;
    float absh = sprite_height;

    /*
    forg::geometry::PositionColoredTextured points[4] =
    {
        {0.0f, 0.0f, 0.0f, 0xffffffff, u0, v1}, // bottom-left
        {absw, 0.0f, 0.0f, 0xffffffff, u1, v1}, // bottom-right
        {absw, absh, 0.0f, 0xffffffff, u1, v0}, // top-right
        {0.0f, absh, 0.0f, 0xffffffff, u0, v0}  // top-left
    };
    */
    forg::geometry::PositionColoredTextured points[4] =
    {
        {0.0f, 0.0f, 0.0f, 0xffffffff, u0, v0}, // bottom-left
        {absw, 0.0f, 0.0f, 0xffffffff, u1, v0}, // bottom-right
        {absw, absh, 0.0f, 0xffffffff, u1, v1}, // top-right
        {0.0f, absh, 0.0f, 0xffffffff, u0, v1}  // top-left
    };

    const short indices[4] = {3, 2, 0, 1};
    const VertexDeclaration decl(forg::geometry::PositionColoredTextured::Declaration);

    Matrix4 tm = m_transform;

    float screen_x = 0.0f;  // left by default
    float screen_y = 0.0f; //(float)sprite_height;
    float screen_z = 0.0f;



    if (center)
    {
        float norm_cx = center->X;
        float norm_cy = center->Y;

        Matrix4 trans_tm;

        Matrix4::Translation(trans_tm, -center->X, -center->Y, -center->Z);
        Matrix4::Multiply(tm, trans_tm, tm);

        Vector3 tm_scale;
        m_transform.Decompose(&tm_scale, 0, 0);
        Matrix4::Translation(trans_tm, center->X*tm_scale.X, center->Y*tm_scale.Y, center->Z*tm_scale.Z);
        tm.Multiply(trans_tm);
    }

    if (position)
    {
        Matrix4 trans_tm;

        Matrix4::Translation(trans_tm, position->X, position->Y, position->Z);
        tm.Multiply(trans_tm);
    }

    if (m_Flags & SpriteFlags::Billboard)
    {
        Matrix4 view;
        m_device->GetTransform(TransformType_View, view);

        Vector3 look;
        Vector3 up;
        Vector3 right;

        view.GetColumn(look, 2);
        view.GetColumn(up, 1);
        view.GetColumn(right, 0);

        // face sprite to eye
        if (0)
        {
            Vector3 sprite_pos(0.0f, 0.0f, 0.0f);
            Vector3 cam_pos(-view.M41, -view.M42, -view.M43);
            view.Transpose();
            cam_pos.TransformNormal(view);

            look = cam_pos - sprite_pos;
            look.Normalize();

            Vector3::Cross(right, up, look);
            right.Normalize();

            Vector3::Cross(up, look, right);
            up.Normalize();

            tm.SetRow(right, 0);
            tm.SetRow(up, 1);
            tm.SetRow(look, 2);
        } else
        {
            tm.SetRow(right, 0);
            tm.SetRow(up, 1);
            tm.SetRow(look, 2);
        }

        tm.SetPosition(0.0f, 0.0f, 0.0f);
    }

    if (m_Flags & SpriteFlags::ObjectSpace)
    {
        m_device->SetTransform(TransformType_World, tm);
    } else
    {
        Matrix4 proj_tm;
        Matrix4::OrthoOffCenterRH(proj_tm, 0, vp.Width, vp.Height, 0, 0.0f, 100.0f);

        m_device->SetTransform(TransformType_Projection, proj_tm /*Matrix4::Identity*/);
        m_device->SetTransform(TransformType_View, Matrix4::Identity);
        m_device->SetTransform(TransformType_World, tm);
    }

    m_device->SetStreamSource(0, 0, 0, 0);
    m_device->SetIndices(0);
    m_device->SetTexture(0, srcTexture);
    m_device->SetVertexDeclaration(&decl);
    m_device->DrawIndexedUserPrimitives(PrimitiveType_TriangleStrip, 0, 4, 2, indices, true, points, forg::geometry::PositionColoredTextured::StrideSize);

    return FORG_OK;
}

}
