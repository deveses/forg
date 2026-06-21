/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2024  Slawomir Strumecki

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

// System headers first: forg's base.h defines null/IN/OUT macros that break the
// Metal / Cocoa headers if they are seen earlier. (MRC build: manual
// retain/release.)
#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include <cstdint>
#include <cstring>
#include <unordered_map>

#include "MetalBuffers.h"
#include "MetalRenderDevice.h"

namespace forg
{

/////////////////////////////////////////////////////////////////////////////////////
// Shared CPU/GPU layout. float4x4 is read column-major by MSL, so the row-major
// row-vector matrices FORG produces arrive transposed - exactly what `mvp *
// pos` (column-vector convention) needs. See docs/metal-renderer-plan.md,
// point 3.
/////////////////////////////////////////////////////////////////////////////////////
struct MetalUniforms
{
    float mvp[16];
    float world[16];
    float lightPos[4];
    float lightDiffuse[4];
    float lightAmbient[4];
    uint32_t lightingEnabled;
    uint32_t _pad[3];
};

// MSL embedded as source and compiled at init - avoids a .metal build step.
static NSString* const kMetalShaderSource = @R"METAL(
#include <metal_stdlib>
using namespace metal;

struct Uniforms {
    float4x4 mvp;
    float4x4 world;
    float4 lightPos;
    float4 lightDiffuse;
    float4 lightAmbient;
    uint lightingEnabled;
};

struct VSIn {
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
    float2 uv       [[attribute(2)]];
};

struct VSOut {
    float4 position [[position]];
    float3 worldPos;
    float3 worldNormal;
};

vertex VSOut vs_main(VSIn in [[stage_in]],
                     constant Uniforms& u [[buffer(1)]])
{
    VSOut out;
    out.position    = u.mvp * float4(in.position, 1.0);
    out.worldPos    = (u.world * float4(in.position, 1.0)).xyz;
    out.worldNormal = (u.world * float4(in.normal, 0.0)).xyz;
    return out;
}

fragment float4 fs_main(VSOut in [[stage_in]],
                        constant Uniforms& u [[buffer(0)]])
{
    if (u.lightingEnabled == 0) {
        return float4(1.0, 1.0, 1.0, 1.0);
    }

    float3 N = normalize(in.worldNormal);
    float3 L = normalize(u.lightPos.xyz - in.worldPos);

    // Match the reference software renderer's shading so the two backends look
    // identical: half-Lambert (dot+1)*0.5 modulated by Diffuse*Ambient. The demo
    // sets a white Ambient and a yellow Diffuse, which this reproduces. For a
    // physically-correct Lambert instead, use: max(0, dot(N, L)).
    float d = (dot(N, L) + 1.0) * 0.5;
    float3 color = d * u.lightDiffuse.rgb * u.lightAmbient.rgb;
    return float4(color, 1.0);
}
)METAL";

/////////////////////////////////////////////////////////////////////////////////////
// Opaque Metal state (hidden from the header).
/////////////////////////////////////////////////////////////////////////////////////
struct MetalImpl
{
    id<MTLDevice> device = nil;
    id<MTLCommandQueue> queue = nil;
    CAMetalLayer* layer = nil;
    id<MTLLibrary> library = nil;
    id<MTLFunction> vfn = nil;
    id<MTLFunction> ffn = nil;
    id<MTLDepthStencilState> depthState = nil;
    id<MTLTexture> depthTexture = nil;
    NSUInteger depthW = 0;
    NSUInteger depthH = 0;

    // Render pipelines keyed by a (vertex declaration) fingerprint.
    std::unordered_map<uint64_t, id<MTLRenderPipelineState>> pipelines;

    // Live for the duration of one frame (BeginScene -> Present).
    id<CAMetalDrawable> drawable = nil;
    id<MTLCommandBuffer> cmd = nil;
    id<MTLRenderCommandEncoder> encoder = nil;
};

/////////////////////////////////////////////////////////////////////////////////////
// Small mapping helpers
/////////////////////////////////////////////////////////////////////////////////////
static int AttributeForUsage(byte usage)
{
    switch (usage)
    {
    case DeclarationUsage_Position:
        return 0;
    case DeclarationUsage_Normal:
        return 1;
    case DeclarationUsage_TextureCoordinate:
        return 2;
    default:
        return -1;
    }
}

static MTLVertexFormat FormatForType(byte type)
{
    switch (type)
    {
    case DeclarationType_Float1:
        return MTLVertexFormatFloat;
    case DeclarationType_Float2:
        return MTLVertexFormatFloat2;
    case DeclarationType_Float3:
        return MTLVertexFormatFloat3;
    case DeclarationType_Float4:
        return MTLVertexFormatFloat4;
    default:
        return MTLVertexFormatFloat3;
    }
}

static uint64_t DeclarationFingerprint(const VertexDeclaration& decl)
{
    // FNV-1a over the element layout - one pipeline per distinct declaration.
    uint64_t h = 1469598103934665603ULL;
    const VertexElement* els = decl.GetDeclaration();
    uint n = decl.GetElementsCount();
    h = (h ^ decl.GetVertexSize()) * 1099511628211ULL;
    for (uint i = 0; i < n; i++)
    {
        h = (h ^ els[i].Usage) * 1099511628211ULL;
        h = (h ^ els[i].Type) * 1099511628211ULL;
        h = (h ^ els[i].Offset) * 1099511628211ULL;
    }
    return h;
}

/////////////////////////////////////////////////////////////////////////////////////
// MetalRenderDevice
/////////////////////////////////////////////////////////////////////////////////////
MetalRenderDevice::MetalRenderDevice(HWIN handle)
    : m_window(handle), m_impl(new MetalImpl()), m_width(0), m_height(0),
      m_clear_color(0.0f, 0.0f, 0.0f, 1.0f), m_clear_z(1.0f),
      m_clear_flags(ClearFlags_Target | ClearFlags_ZBuffer),
      m_vdecl(&VertexElement::VertexDeclarationEnd), m_stream0(0),
      m_stream0_offset(0), m_stream0_stride(0), m_indices(0), m_cull(Cull_None),
      m_fill(FillMode_Solid), m_lighting(true)
{
    memset(m_lights, 0, sizeof(m_lights));
    for (int i = 0; i < NUM_LIGHTS; i++)
        m_light_enabled[i] = false;
}

MetalRenderDevice::~MetalRenderDevice()
{
    SetStreamSource(0, 0, 0, 0);
    SetIndices(0);

    if (m_impl)
    {
        for (std::unordered_map<uint64_t, id<MTLRenderPipelineState>>::iterator
                 it = m_impl->pipelines.begin();
             it != m_impl->pipelines.end(); ++it)
        {
            [it->second release];
        }
        m_impl->pipelines.clear();

        [m_impl->depthTexture release];
        [m_impl->depthState release];
        [m_impl->ffn release];
        [m_impl->vfn release];
        [m_impl->library release];
        [m_impl->layer release];
        [m_impl->queue release];
        [m_impl->device release];

        delete m_impl;
        m_impl = 0;
    }
}

int MetalRenderDevice::Initialize(uint /*width*/, uint /*height*/)
{
    m_impl->device = MTLCreateSystemDefaultDevice();
    if (m_impl->device == nil)
        return FORG_INVALID_CALL;

    m_impl->queue = [m_impl->device newCommandQueue];

    NSError* error = nil;
    MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
    m_impl->library = [m_impl->device newLibraryWithSource:kMetalShaderSource
                                                   options:options
                                                     error:&error];
    [options release];
    if (m_impl->library == nil)
    {
        NSLog(@"[metalrenderer] shader compilation failed: %@", error);
        return FORG_INVALID_CALL;
    }

    m_impl->vfn = [m_impl->library newFunctionWithName:@"vs_main"];
    m_impl->ffn = [m_impl->library newFunctionWithName:@"fs_main"];

    MTLDepthStencilDescriptor* dsd = [[MTLDepthStencilDescriptor alloc] init];
    dsd.depthCompareFunction =
        MTLCompareFunctionLessEqual; // matches reference `d <= depth`, clear
                                     // = 1.0
    dsd.depthWriteEnabled = YES;
    m_impl->depthState =
        [m_impl->device newDepthStencilStateWithDescriptor:dsd];
    [dsd release];

    EnsureSurfaces();

    return FORG_OK;
}

void MetalRenderDevice::EnsureSurfaces()
{
    NSView* view = (NSView*)m_window;
    if (view == nil)
        return;

    CGFloat scale = (view.window != nil) ? view.window.backingScaleFactor : 1.0;
    if (scale <= 0.0)
        scale = 1.0;

    NSSize pts = view.bounds.size;
    NSUInteger pw = (NSUInteger)(pts.width * scale);
    NSUInteger ph = (NSUInteger)(pts.height * scale);
    if (pw == 0)
        pw = 1;
    if (ph == 0)
        ph = 1;

    if (m_impl->layer == nil)
    {
        CAMetalLayer* layer = [CAMetalLayer layer];
        layer.device = m_impl->device;
        layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        layer.framebufferOnly = YES;
        m_impl->layer = [layer retain];

        // Host the Metal layer in the renderer-agnostic NSView - same trick the
        // software renderer uses with layer.contents.
        view.wantsLayer = YES;
        view.layer = m_impl->layer;
    }

    m_impl->layer.contentsScale = scale;
    m_impl->layer.drawableSize = CGSizeMake((CGFloat)pw, (CGFloat)ph);
    m_width = (uint)pw;
    m_height = (uint)ph;

    if (m_impl->depthTexture == nil || m_impl->depthW != pw ||
        m_impl->depthH != ph)
    {
        [m_impl->depthTexture release];

        MTLTextureDescriptor* td = [MTLTextureDescriptor
            texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                         width:pw
                                        height:ph
                                     mipmapped:NO];
        td.usage = MTLTextureUsageRenderTarget;
        td.storageMode = MTLStorageModePrivate;

        m_impl->depthTexture =
            [m_impl->device newTextureWithDescriptor:td]; // +1 owned
        m_impl->depthW = pw;
        m_impl->depthH = ph;
    }
}

int MetalRenderDevice::Reset()
{
    EnsureSurfaces();
    return FORG_OK;
}

int MetalRenderDevice::Clear(uint flags, Color color, float zdepth,
                             int /*stencil*/)
{
    // No GPU work here - the render pass applies these as load actions in
    // BeginScene.
    m_clear_flags = flags;
    m_clear_color = color;
    m_clear_z = zdepth;
    return FORG_OK;
}

int MetalRenderDevice::BeginScene(void)
{
    if (m_impl->layer == nil)
        EnsureSurfaces();

    id<CAMetalDrawable> drawable = [m_impl->layer nextDrawable];
    if (drawable == nil)
        return FORG_OK; // skip the frame; the timer will try again

    m_impl->drawable = [drawable retain];

    MTLRenderPassDescriptor* rp =
        [MTLRenderPassDescriptor renderPassDescriptor];
    rp.colorAttachments[0].texture = drawable.texture;
    rp.colorAttachments[0].loadAction = (m_clear_flags & ClearFlags_Target)
                                            ? MTLLoadActionClear
                                            : MTLLoadActionLoad;
    rp.colorAttachments[0].storeAction = MTLStoreActionStore;
    rp.colorAttachments[0].clearColor = MTLClearColorMake(
        m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a);

    rp.depthAttachment.texture = m_impl->depthTexture;
    rp.depthAttachment.loadAction = (m_clear_flags & ClearFlags_ZBuffer)
                                        ? MTLLoadActionClear
                                        : MTLLoadActionLoad;
    rp.depthAttachment.storeAction = MTLStoreActionDontCare;
    rp.depthAttachment.clearDepth = m_clear_z;

    m_impl->cmd = [[m_impl->queue commandBuffer] retain];
    m_impl->encoder =
        [[m_impl->cmd renderCommandEncoderWithDescriptor:rp] retain];

    MTLViewport vp;
    vp.originX = 0.0;
    vp.originY = 0.0;
    vp.width = (double)m_width;
    vp.height = (double)m_height;
    vp.znear = 0.0;
    vp.zfar = 1.0;
    [m_impl->encoder setViewport:vp];

    [m_impl->encoder setDepthStencilState:m_impl->depthState];

    // The reference renderer fills both windings (it never culls); map the cull
    // render state but verify against it visually - see docs plan, point 4.
    MTLCullMode cull = MTLCullModeNone;
    switch (m_cull)
    {
    case Cull_None:
        cull = MTLCullModeNone;
        break;
    case Cull_Clockwise:
        cull = MTLCullModeBack;
        break;
    case Cull_CounterClockwise:
        cull = MTLCullModeFront;
        break;
    default:
        cull = MTLCullModeNone;
        break;
    }
    [m_impl->encoder setCullMode:cull];
    [m_impl->encoder setFrontFacingWinding:MTLWindingCounterClockwise];

    [m_impl->encoder setTriangleFillMode:(m_fill == FillMode_WireFrame)
                                             ? MTLTriangleFillModeLines
                                             : MTLTriangleFillModeFill];

    return FORG_OK;
}

int MetalRenderDevice::EndScene(void)
{
    if (m_impl->encoder != nil)
        [m_impl->encoder endEncoding];
    return FORG_OK;
}

int MetalRenderDevice::Present()
{
    if (m_impl->cmd != nil && m_impl->drawable != nil)
    {
        [m_impl->cmd presentDrawable:m_impl->drawable];
        [m_impl->cmd commit];
    }

    [m_impl->encoder release];
    m_impl->encoder = nil;
    [m_impl->cmd release];
    m_impl->cmd = nil;
    [m_impl->drawable release];
    m_impl->drawable = nil;

    return FORG_OK;
}

LPVERTEXDECLARATION
MetalRenderDevice::CreateVertexDeclaration(const VertexElement* pVertexElements)
{
    return new VertexDeclaration(pVertexElements);
}

LPVERTEXBUFFER MetalRenderDevice::CreateVertexBuffer(uint length,
                                                     uint /*usage*/,
                                                     uint /*pool*/)
{
    MetalVertexBuffer* vb = new MetalVertexBuffer();
    vb->Create((void*)m_impl->device, length);
    return vb;
}

LPINDEXBUFFER MetalRenderDevice::CreateIndexBuffer(uint length, uint /*usage*/,
                                                   bool sixteenBitIndices,
                                                   uint /*pool*/)
{
    MetalIndexBuffer* ib = new MetalIndexBuffer();
    ib->Create((void*)m_impl->device, length, sixteenBitIndices);
    return ib;
}

LPTEXTURE MetalRenderDevice::CreateTexture(uint /*Width*/, uint /*Height*/,
                                           uint /*Levels*/, uint /*Usage*/,
                                           uint /*Format*/, uint /*Pool*/)
{
    // Out of scope for the first pass; the demo calls SetTexture(0, null) only.
    return 0;
}

LPTEXTURE MetalRenderDevice::CreateTextureFromFile(
    const char* /*filename*/, uint /*Width*/, uint /*Height*/, uint /*Levels*/,
    uint /*Usage*/, uint /*Format*/, uint /*Pool*/)
{
    return 0;
}

int MetalRenderDevice::DrawIndexedPrimitive(PrimitiveType primitiveType,
                                            int /*baseVertex*/,
                                            int /*minVertexIndex*/,
                                            int /*numVertices*/, int startIndex,
                                            int primCount)
{
    if (m_impl->encoder == nil || m_stream0 == 0 || m_indices == 0)
        return FORG_OK;

    if (primitiveType != PrimitiveType_TriangleList)
        return FORG_OK; // only triangle lists in the first pass

    // --- pipeline (cached per vertex declaration)
    // -----------------------------
    uint64_t key = DeclarationFingerprint(m_vdecl);
    id<MTLRenderPipelineState> pipeline = nil;
    std::unordered_map<uint64_t, id<MTLRenderPipelineState>>::iterator found =
        m_impl->pipelines.find(key);
    if (found != m_impl->pipelines.end())
    {
        pipeline = found->second;
    }
    else
    {
        MTLVertexDescriptor* vd = [MTLVertexDescriptor vertexDescriptor];
        const VertexElement* els = m_vdecl.GetDeclaration();
        uint n = m_vdecl.GetElementsCount();
        for (uint i = 0; i < n; i++)
        {
            int attr = AttributeForUsage(els[i].Usage);
            if (attr < 0)
                continue;
            vd.attributes[attr].format = FormatForType(els[i].Type);
            vd.attributes[attr].offset = els[i].Offset;
            vd.attributes[attr].bufferIndex = 0;
        }
        vd.layouts[0].stride = (NSUInteger)m_stream0_stride;
        vd.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

        MTLRenderPipelineDescriptor* pd =
            [[MTLRenderPipelineDescriptor alloc] init];
        pd.vertexFunction = m_impl->vfn;
        pd.fragmentFunction = m_impl->ffn;
        pd.vertexDescriptor = vd;
        pd.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
        pd.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

        NSError* error = nil;
        pipeline = [m_impl->device newRenderPipelineStateWithDescriptor:pd
                                                                  error:&error];
        [pd release];

        if (pipeline == nil)
        {
            NSLog(@"[metalrenderer] pipeline creation failed: %@", error);
            return FORG_INVALID_CALL;
        }
        m_impl->pipelines[key] = pipeline; // map keeps the +1 ownership
    }

    [m_impl->encoder setRenderPipelineState:pipeline];

    // --- uniforms
    // -------------------------------------------------------------
    Matrix4 mv;
    Matrix4 mvp;
    Matrix4::Multiply(mv, m_world, m_view);
    Matrix4::Multiply(mvp, mv, m_proj);

    MetalUniforms u;
    memset(&u, 0, sizeof(u));
    memcpy(u.mvp, (const float*)mvp, sizeof(u.mvp));
    memcpy(u.world, (const float*)m_world, sizeof(u.world));

    const Light& light = m_lights[0];
    u.lightPos[0] = light.Position.X;
    u.lightPos[1] = light.Position.Y;
    u.lightPos[2] = light.Position.Z;
    u.lightPos[3] = 1.0f;
    u.lightDiffuse[0] = light.Diffuse.r;
    u.lightDiffuse[1] = light.Diffuse.g;
    u.lightDiffuse[2] = light.Diffuse.b;
    u.lightDiffuse[3] = light.Diffuse.a;
    u.lightAmbient[0] = light.Ambient.r;
    u.lightAmbient[1] = light.Ambient.g;
    u.lightAmbient[2] = light.Ambient.b;
    u.lightAmbient[3] = light.Ambient.a;
    u.lightingEnabled = (m_lighting && m_light_enabled[0]) ? 1u : 0u;

    // --- buffers + draw
    // -------------------------------------------------------
    MetalVertexBuffer* vb = (MetalVertexBuffer*)m_stream0;
    MetalIndexBuffer* ib = (MetalIndexBuffer*)m_indices;

    [m_impl->encoder setVertexBuffer:(id<MTLBuffer>)vb->GetMTLBuffer()
                              offset:(NSUInteger)m_stream0_offset
                             atIndex:0];
    [m_impl->encoder setVertexBytes:&u length:sizeof(u) atIndex:1];
    [m_impl->encoder setFragmentBytes:&u length:sizeof(u) atIndex:0];

    NSUInteger indexCount = (NSUInteger)primCount * 3;
    NSUInteger indexSize = ib->IsIndexShort() ? 2 : 4;
    MTLIndexType indexType =
        ib->IsIndexShort() ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32;
    NSUInteger indexOffset = (NSUInteger)startIndex * indexSize;

    [m_impl->encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                indexCount:indexCount
                                 indexType:indexType
                               indexBuffer:(id<MTLBuffer>)ib->GetMTLBuffer()
                         indexBufferOffset:indexOffset];

    return FORG_OK;
}

int MetalRenderDevice::DrawIndexedUserPrimitives(
    PrimitiveType /*primitiveType*/, uint /*minVertexIndex*/,
    uint /*numVertexIndices*/, uint /*primitiveCount*/,
    const void* /*indexData*/, bool /*sixteenBitIndices*/,
    const void* /*vertexStreamZeroData*/, uint /*vertexStreamZeroStride*/)
{
    // Out of scope for the first pass (the demo draws via vertex/index
    // buffers).
    return FORG_OK;
}

void MetalRenderDevice::SetTransform(TransformType state, const Matrix4& matrix)
{
    switch (state)
    {
    case TransformType_World:
        m_world = matrix;
        break;
    case TransformType_View:
        m_view = matrix;
        break;
    case TransformType_Projection:
        m_proj = matrix;
        break;
    default:
        break;
    }
}

void MetalRenderDevice::GetTransform(TransformType state, Matrix4& matrix)
{
    switch (state)
    {
    case TransformType_World:
        matrix = m_world;
        break;
    case TransformType_View:
        matrix = m_view;
        break;
    case TransformType_Projection:
        matrix = m_proj;
        break;
    default:
        break;
    }
}

int MetalRenderDevice::SetVertexDeclaration(const VertexDeclaration* pDecl)
{
    if (pDecl)
        m_vdecl = *pDecl;
    return FORG_OK;
}

int MetalRenderDevice::SetStreamSource(int streamNumber,
                                       IVertexBuffer* streamData,
                                       int offsetInBytes, int stride)
{
    if (streamNumber != 0)
        return FORG_OK; // single stream in the first pass

    if (m_stream0)
        m_stream0->Release();

    m_stream0 = streamData;
    m_stream0_offset = offsetInBytes;
    m_stream0_stride = stride;

    if (m_stream0)
        m_stream0->AddRef();

    return FORG_OK;
}

int MetalRenderDevice::SetIndices(IIndexBuffer* pIndexData)
{
    if (m_indices)
        m_indices->Release();

    m_indices = pIndexData;

    if (m_indices)
        m_indices->AddRef();

    return FORG_OK;
}

int MetalRenderDevice::SetViewport(uint /*X*/, uint /*Y*/, uint /*Width*/,
                                   uint /*Height*/, float /*MinZ*/,
                                   float /*MaxZ*/)
{
    // The actual viewport always covers the full drawable (set in BeginScene
    // from the backing-pixel size); the demo never renders to a sub-rectangle.
    return FORG_OK;
}

int MetalRenderDevice::GetViewport(Viewport* viewport)
{
    if (viewport == 0)
        return FORG_INVALID_CALL;

    viewport->X = 0;
    viewport->Y = 0;
    viewport->Width = m_width;
    viewport->Height = m_height;
    viewport->MinZ = 0.0f;
    viewport->MaxZ = 1.0f;
    return FORG_OK;
}

int MetalRenderDevice::SetRenderState(uint state, uint value)
{
    switch (state)
    {
    case RenderStates_CullMode:
        m_cull = value;
        break;
    case RenderStates_FillMode:
        m_fill = value;
        break;
    case RenderStates_Lighting:
        m_lighting = (value != 0);
        break;
    default:
        break; // ShadeMode / blend factors etc. are stubbed for the first pass
    }
    return FORG_OK;
}

int MetalRenderDevice::SetTexture(uint /*Sampler*/, ITexture* /*pTexture*/)
{
    // Texturing is out of scope; the demo only ever clears the sampler (null).
    return FORG_OK;
}

int MetalRenderDevice::SetLight(uint Index, const Light* pLight)
{
    if (Index < NUM_LIGHTS && pLight != 0)
    {
        m_lights[Index] = *pLight;
        return FORG_OK;
    }
    return FORG_INVALID_CALL;
}

int MetalRenderDevice::LightEnable(uint LightIndex, bool bEnable)
{
    if (LightIndex < NUM_LIGHTS)
        m_light_enabled[LightIndex] = bEnable;
    return FORG_OK;
}

int MetalRenderDevice::SetMaterial(const Material* /*pMaterial*/)
{
    // Material is unused by the demo shading path (lighting reads the Light
    // only).
    return FORG_OK;
}

} // namespace forg
