#include "forg_pch.h"

#include "rendering/ITexture.h"
#include "rendering/IRenderDevice.h"

#include "debug/dbg.h"
#include "image/Image.h"

namespace forg {


ITexture* ITexture::FromFile( IRenderDevice* device, const char* srcFile )
{
    Image image;

    ITexture* tex = NULL;

    if (image.Load(srcFile))
    {
        tex = device->CreateTexture(image.GetWidth(), image.GetHeight(), 0, 0, Format::A8R8G8B8, Pool_Managed);

        if (tex)
        {
            image.GenerateMipmaps();

            for (uint i=0; i<tex->GetLevelCount(); i++)
            {
                void* bits = tex->LockRect(i, 0);

                if (bits != 0)
                {
                    memcpy(bits, image.GetData(i), image.GetSize(i));

                    tex->UnlockRect(i);
                } else
                {
                    DBG_MSG("[ITexture::FromFile] Failed to upload level %d\n", i);
                    //delete tex;
                }
            }
        }
    }


    /*
    PerlinNoise noise_gen;
    ByteArray ba(m_width*m_height*3);
    float xstart = 3.0f;
    float ystart = 7.0f;
    for(uint i=0; i<ba.size(); i+=3)
    {
    float x = (i/3)%m_width;
    float y = (i/3)/m_width;
    x /= m_width - 1.0f;
    y /= m_height - 1.0f;
    ba[i] = ((int)(256.0f*noise_gen.noise2d(xstart + 7.0f * x, ystart + 3.0f * y)))%256;
    ba[i+1] = ba[i];//255-ba[i];
    ba[i+2] = ba[i];//(i/m_width)%256;
    }*/

    return tex;
}












}
