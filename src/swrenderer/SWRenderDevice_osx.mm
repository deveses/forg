// Cocoa must come first: forg's base.h defines macros (null, IN, OUT)
// that break the system headers if they are seen earlier.
#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

#include "SWRenderDevice.h"
#include "forg.h"

namespace forg
{
    /////////////////////////////////////////////////////////////////////////////////////
    // SWRenderDevice (macOS presentation: HWIN carries a layer-backed NSView*)
    /////////////////////////////////////////////////////////////////////////////////////
    SWRenderDevice::SWRenderDevice(HWIN handle)
        : super(handle)
    {
    }

    SWRenderDevice::~SWRenderDevice()
    {
    }

    int SWRenderDevice::Reset()
    {
        NSView* view = (NSView*)GetHWIN();
        NSSize size = view.bounds.size;

        uint width = (uint)size.width;
        uint height = (uint)size.height;
        SetBufferSize(width, height);

        super::Reset();

        return FORG_OK;
    }

    int SWRenderDevice::Present()
    {
        // frame buffer color format: ARGB (0xAARRGGBB), same layout GDI consumed
        NSView* view = (NSView*)GetHWIN();

        uint width = GetWidth();
        uint height = GetHeight();
        uint* buffer = GetBuffer();

        if (view == nil || buffer == 0 || width == 0 || height == 0)
            return FORG_OK;

        size_t length = (size_t)width * height * 4;
        CFDataRef data = CFDataCreate(NULL, (const UInt8*)buffer, length);
        CGDataProviderRef provider = CGDataProviderCreateWithCFData(data);
        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        CGImageRef image = CGImageCreate(width, height, 8, 32, width * 4, colorSpace,
            kCGBitmapByteOrder32Little | kCGImageAlphaNoneSkipFirst,
            provider, NULL, false, kCGRenderingIntentDefault);

        [CATransaction begin];
        [CATransaction setDisableActions:YES];
        view.layer.contents = (id)image;
        [CATransaction commit];

        CGImageRelease(image);
        CGColorSpaceRelease(colorSpace);
        CGDataProviderRelease(provider);
        CFRelease(data);

        return FORG_OK;
    }

}
