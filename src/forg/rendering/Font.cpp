#include "forg_pch.h"
#include "rendering/Font.h"
#include "rendering/Vertex.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

#include "debug/dbg.h"

namespace forg {

static unsigned int next_pow2(unsigned int v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;

    return v;
}

struct PixelA8L8
{
    byte l;
    byte a;
};

struct PixelA8R8G8B8
{
    byte b;
    byte g;
    byte r;
    byte a;
};

Font::Font()
{
    m_device = 0;
    m_bitmap = (NULL);
    m_texture = (NULL);
    m_sprite = NULL;
    m_size = (0);
    m_tex_width = (0);
    m_tex_height = (0);
}

Font::~Font()
{
    if (m_bitmap)
    {
        delete [] m_bitmap;
        m_bitmap = 0;
    }

    if (m_texture)
    {
        m_texture->Release();
        m_texture = 0;
    }

    if (m_sprite)
    {
        delete m_sprite;
        m_sprite = 0;
    }

    if (m_device)
    {
        m_device->Release();
        m_device = 0;
    }

    m_size = 0;
}

Font* Font::CreateIndirect(IRenderDevice* device, FontDescription* fontDesc)
{
    if (device == NULL || fontDesc == NULL)
        return NULL;

    FT_Library library;
    if (FT_Init_FreeType( &library ))
        return NULL;

    DBG_MSG("Freetype version: %d.%d.%d\n", FREETYPE_MAJOR, FREETYPE_MINOR, FREETYPE_PATCH);

    FT_Face face;
    if (FT_New_Face( library, fontDesc->FontPath, 0, &face ))
        return NULL;

    uint Width = fontDesc->Width;
    uint Height = fontDesc->Height;

    if (Width == 0)
        Width = Height;

    FT_Set_Char_Size( face, Width << 6, Height << 6, 96, 96);

    //////////////////////////////////////////////////////////////////////////
    uint bwidth = Width * 2 * 256;
    uint bheight = Height * 2;

    device->AddRef();
    Font* font = new Font();
    font->m_size = bwidth * bheight;
    font->m_bitmap = new char[font->m_size];
    font->m_CharWidth = Width;
    font->m_CharHeight = Height;
    font->m_device = device;
    font->m_sprite = Sprite::CreateSprite(device);

    int cur_off = 0;
    for (int i=0; i<256; i++)
    {
        if(FT_Load_Glyph( face, FT_Get_Char_Index( face, i ), FT_LOAD_DEFAULT ))
        {
            continue;
        }

        FT_Glyph glyph;
        if(FT_Get_Glyph( face->glyph, &glyph ) == 0)
        {
            FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1 );
            FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

            font->m_metrics[i].width = bitmap_glyph->bitmap.width;
            font->m_metrics[i].rows = bitmap_glyph->bitmap.rows;
            font->m_metrics[i].left = bitmap_glyph->left;
            font->m_metrics[i].top = bitmap_glyph->top;
            font->m_metrics[i].advance = face->glyph->advance.x >> 6;
            font->m_metrics[i].offset = cur_off;

            memcpy(font->m_bitmap + cur_off, bitmap_glyph->bitmap.buffer, font->m_metrics[i].width * font->m_metrics[i].rows);

            cur_off += font->m_metrics[i].width * font->m_metrics[i].rows;
        }

        FT_Done_Glyph(glyph);
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);

    return font;
}

int Font::DrawText2(LPCTSTR pString, int count, Rectangle* pRect, uint format, Color4b color)
{
    if (! pRect)
        return 0;

    uint total_width = 0;
    uint tex_width = 0;
    uint tex_height = 0;
    uint max_height = 0;
    uint max_bearingy = 0;
    uint tex_pad_x = 0;
    uint tex_pad_y = 0;

    if (count < 0)
        count = (int)strlen(pString);


    //----------------------------------------------------------------------
    // compute texture size
    //----------------------------------------------------------------------
    for (int i=0; i<count; i++)
    {
        int c = pString[i];
        uint th = m_metrics[c].rows + (m_metrics[c].rows - m_metrics[c].top);

        if (max_height < th)
            max_height = th;

        if ((int)max_bearingy < m_metrics[c].top)
            max_bearingy = m_metrics[c].top;

        total_width += m_metrics[c].advance;
    }

    tex_width = next_pow2(total_width);
    tex_height = next_pow2(max_height);

    tex_pad_x = total_width - tex_width;
    tex_pad_y = max_height - tex_height;

    if (tex_width == 0 || tex_height == 0)
        return FORG_INVALID_CALL;

    // TODO: check texture size!

    //----------------------------------------------------------------------
    // create new texture if size changed
    //----------------------------------------------------------------------
    if (tex_width != m_tex_width || tex_height != m_tex_height)
    {
        if (m_texture)
            m_texture->Release();

        m_texture = m_device->CreateTexture(tex_width, tex_height, 1, 0, forg::Format::A8R8G8B8, Pool_Managed);
        m_tex_width = tex_width;
        m_tex_height = tex_height;
    }

    if (m_texture == 0)
        return FORG_INVALID_CALL;

    //----------------------------------------------------------------------
    // fill texture with text
    //----------------------------------------------------------------------

    void* bits = m_texture->LockRect(0, 0);
    if (bits != 0)
    {
        PixelA8R8G8B8* data = (PixelA8R8G8B8*)bits;

        // comment to see padding
        memset(data, 0, sizeof(PixelA8R8G8B8)*tex_width*tex_height);

        uint offx = 0;

        for (int i=0; i<count; i++)
        {
            int c = pString[i];

            uint fw = m_metrics[c].width;
            uint fh = m_metrics[c].rows;

            for (uint h=0; h<fh; h++)
            {
                uint y = max_bearingy - m_metrics[c].top + h;
                uint x = offx + m_metrics[c].left;

                for (uint w=0; w<fw; w++)
                {
                    char grey = m_bitmap[m_metrics[c].offset + h*fw + w];

                    PixelA8R8G8B8& col = data[y*tex_width + x + w];
                    col.a = grey;
                    col.r = color.r;
                    col.g = color.g;
                    col.b = color.b;
                }
            }

            offx += m_metrics[c].advance;
        }

        m_texture->UnlockRect(0);
    }

    //----------------------------------------------------------------------
    // draw sprite
    //----------------------------------------------------------------------
    /*
    // uncomment for ortho view with screen dependend units
    sprite_width = twidth;
    sprite_height = theight;

    Matrix4::OrthoRH(tm, vp.Width, vp.Height, 0.01f, 100.0f);
    */

	// position of left bottom
    uint screen_x = 0;  // left by default
    uint screen_y = pRect->top;

    if ((format & DTFMT_CENTER) == DTFMT_CENTER)
        screen_x = pRect->left + (pRect->right - pRect->left - total_width) / 2;

    if ((format & DTFMT_VCENTER) == DTFMT_VCENTER)
        screen_y = pRect->top + (pRect->bottom - pRect->top + max_height) / 2 - tex_height;

    if ((format & DTFMT_RIGHT) == DTFMT_RIGHT)
        screen_x = (pRect->right - total_width);

    if ((format & DTFMT_BOTTOM) == DTFMT_BOTTOM)
        screen_y = pRect->bottom - max_height;

    Vector3 translation(screen_x, screen_y, 0.0f);

/*
    // uncomment for ortho view with screen dependend units
    tm = Matrix4::Identity;
    tm.SetPosition(0.0f, (float)vp.Height/2.0f-theight, 0.0f);
    m_device->SetTransform(TransformType_Projection, tm / *Matrix4::Identity* /);
*/

    m_sprite->Begin(SpriteFlags::AlphaBlend);
    m_sprite->Draw(m_texture, NULL, NULL, &translation, Color4b(0xff, 0xff, 0xff, 0xff));
    m_sprite->End();

    return FORG_OK;
}


}
