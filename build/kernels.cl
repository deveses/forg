int min3i(int a, int b, int c)
{
    return min(min(a, b), c);
}

int max3i(int a, int b, int c)
{
    return max(max(a, b), c);
}

int2 min3_int2(int2 a, int2 b, int2 c)
{
    return min(min(a, b), c);  
}

int2 max3_int2(int2 a, int2 b, int2 c)
{
    return max(max(a, b), c);
}

typedef struct _VSOutput
{
    float4 position;
    float4 color;
    float4 texcoord0;
} VSOutput;

typedef struct _Triangle
{
  VSOutput vertices[3];
} Triangle;


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


typedef struct _TriangleInterpolator
{
        float2 lineA01;
        float2 lineA12;
        float2 lineA20;

        float lineC01;
        float lineC12;
        float lineC20;

        float bary01;
        float bary12;
        float bary20;

        float bary_a;
        float bary_b;
        float bary_c;

        bool ext01;
        bool ext12;
        bool ext20;

        bool draw;
} TriangleInterpolator;

float linear_eq(float2 A, float C, float2 x)
{
    // could be fma or mad (faster)
    return dot(A, x) + C;    
}

float det(float2 col1, float2 col2)
{
  // area of the parallelogram with the two vectors for sides
    return col1.x*col2.y - col2.x*col1.y;
}

int det_int2(int2 col1, int2 col2)
{
    return col1.x*col2.y - col2.x*col1.y;
}

void TriangleInterpolator_Initialize(TriangleInterpolator* ti, float2 pos0, float2 pos1, float2 pos2)
{
    ti->lineA01 = (float2)(pos0.y - pos1.y, pos1.x - pos0.x);
    ti->lineA12 = (float2)(pos1.y - pos2.y, pos2.x - pos1.x);
    ti->lineA20 = (float2)(pos2.y - pos0.y, pos0.x - pos2.x);
        
    // cross product AxB = A.x * B.y - B.x * A.y 
    
    ti->lineC01 = det(pos0, pos1);
    ti->lineC12 = det(pos1, pos2);
    ti->lineC20 = det(pos2, pos0);
    
    ti->bary01 = linear_eq(ti->lineA01, ti->lineC01, pos2);
    ti->bary12 = linear_eq(ti->lineA12, ti->lineC12, pos0);
    ti->bary20 = linear_eq(ti->lineA20, ti->lineC20, pos1);
    
    float2 xp = (float2)(-1, -1);
    ti->ext01 = linear_eq(ti->lineA01, ti->lineC01, xp)*ti->bary01 > 0.0f;
    ti->ext12 = linear_eq(ti->lineA12, ti->lineC12, xp)*ti->bary12 > 0.0f;
    ti->ext20 = linear_eq(ti->lineA20, ti->lineC20, xp)*ti->bary20 > 0.0f;

}

void TriangleInterpolator_SetPoint(TriangleInterpolator* ti, int2 pos)
{
    //TODO: could increment only, because line(x+1,y) = line(x,y)+A
  
    float2 fp = convert_float2(pos);
  
    ti->bary_a = linear_eq(ti->lineA12, ti->lineC12, fp) / ti->bary12;
    ti->bary_b = linear_eq(ti->lineA20, ti->lineC20, fp) / ti->bary20;
    ti->bary_c = linear_eq(ti->lineA01, ti->lineC01, fp) / ti->bary01;
  
    ti->bary_a = fabs(ti->bary_a);
    ti->bary_b = fabs(ti->bary_b);
    ti->bary_c = fabs(ti->bary_c);
  
    ti->draw = false;
  
    if (ti->bary_a >= 0.0f && ti->bary_b >= 0.0f && ti->bary_c >= 0.0f)
    {
        if ((ti->bary_a > 0.0f || ti->ext12)
            && (ti->bary_b > 0.0f || ti->ext20)
            && (ti->bary_c > 0.0f || ti->ext01))
        {
            ti->draw = true;
        }
    }
}

float4 interpolate_float4(TriangleInterpolator* ti, float4 attribs0, float4 attribs1, float4 attribs2)
{
    return attribs0*ti->bary_a + attribs1*ti->bary_b + attribs2*ti->bary_c;
}

float interpolate_float(TriangleInterpolator* ti, float attribs0, float attribs1, float attribs2)
{
    return attribs0*ti->bary_a + attribs1*ti->bary_b + attribs2*ti->bary_c;
}

uint convert_color_uint(uint4 col)
{
  return ((col.x) | (col.y<<8) | (col.z<<16) | (col.w<<24));
}

// BGRA
uint4 convert_color_uint4(uint col)
{
  uint4 o;
  o.x = (col) & 0xff;
  o.y = (col >> 8) & 0xff;
  o.z = (col >> 16) & 0xff;
  o.w = (col >> 24);
  return o;
}

uint4 blend_color(uint4 src, uint4 dst, uint src_factor, uint dst_factor)
{
    uint4 out = src;

    if (src_factor == 0)
    {
      out = dst;
    }
    else if (src_factor < 255)
    {
      uint4 blend = src * src_factor + dst * dst_factor;
    
      blend = (blend + 1 + (blend>>8) )>> 8;
    
      blend = clamp(blend, (uint4)0, (uint4)255);
      
      blend.w = 0xff;
      
      out = blend;
    }

    return out;
}

void SetPixel(__global uint* cbuffer, uint2 dim, int2 coords, uint4 col)
{
    uint lindex = dim.x*coords.y + coords.x;        
    uint4 dst = convert_color_uint4(cbuffer[lindex]);   
       
    // alpha blending
    uint source_factor = col.w;       // SRC_ALPHA
    uint dest_factor = 255 - col.w;   // ONE_MINUS_DST_ALPHA

    uint4 blend = blend_color(col, dst, source_factor, dest_factor);
              
    cbuffer[lindex] = convert_color_uint(blend);
}

void ProcessPixel(__global uint* cbuffer, 
                  __global float* zbuffer, 
                  int2 coords, 
                  uint2 dim, 
                  TriangleInterpolator* interpolator, 
                  const Triangle* triangle, 
                  int usage,
                  __read_only image2d_t tex0)
{
    const uint4 colMask = (uint4)(2, 1, 0, 3);     // RGBA -> BGRA

    TriangleInterpolator_SetPoint(interpolator, coords);
    
    float d = interpolate_float(interpolator, triangle->vertices[0].position.z, triangle->vertices[1].position.z, triangle->vertices[2].position.z);

    size_t index = coords.y*dim.x + coords.x;
    float cur_z = zbuffer[index];

    if (d < cur_z)
    {   uint4 int_col = 0;
    
        if (usage & DeclarationUsage_TextureCoordinate)
        {
          const sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
        
          float4 coords = interpolate_float4(interpolator, triangle->vertices[0].texcoord0, triangle->vertices[1].texcoord0, triangle->vertices[2].texcoord0);                                
          uint4 tex_col = read_imageui(tex0, sampler, coords.xy);

          if (tex_col.w > 0)
          {
            int_col = tex_col;            
          }
        }
    
        if (usage & DeclarationUsage_Color)
        {
          float4 col0 = triangle->vertices[0].color;
          float4 col1 = triangle->vertices[1].color;
          float4 col2 = triangle->vertices[2].color;
              
          //col0 = d;    
          //col1 = d;    
          //col2 = d;    
              
          float4 pix_col = interpolate_float4(interpolator, col0, col1, col2);
          
          int_col = convert_uint4_sat_rte(pix_col*255.0f);
          int_col = shuffle(int_col, colMask);
        }
        
        coords.y = dim.y - 1 - coords.y;
        SetPixel(cbuffer, dim, coords, int_col);
        
        // z-buffer read-only for translucent polys
        if (int_col.w == 255)
        {
          zbuffer[index] = d;
        }
    }    
}

__kernel void ClearScreenBuffer(__global  float* buffer, float value)
{
    int sx = get_global_id(0);
    int sy = get_global_id(1);

    size_t width = get_global_size(0);
    size_t height = get_global_size(1);
    
    size_t index = sy*width + sx;
    
    buffer[index] = value;
}

__kernel void PrepareBlock(__constant Triangle* triangles, uint num_triangles)
{
}

__kernel void DrawBlock(__global uint* cbuffer, 
                        __global float* zbuffer, 
                        uint2 dim,
                        __constant Triangle* triangles, 
                        uint num_triangles,
                        int usage,
                        __read_only image2d_t tex0)
{
    uint width = dim.x;
    uint height = dim.y;
  
    int sx = get_global_id(0);
    int sy = get_global_id(1);
    
    //int2 dim = (int2)(width, height);                
    int2 spos = (int2)(sx, sy);
    
    int lx = get_local_id(0);
    int ly = get_local_id(1);
        
    TriangleInterpolator interpolator;
    // GRAB?
    // RGBA -> GRAB
    //uint4 colMask = (uint4)(1, 0, 3, 2);
    const uint4 colMask = (uint4)(2, 1, 0, 3);     // RGBA -> BGRA
    
    for (uint t=0; t<num_triangles; t++)
    {
        const Triangle triangle = triangles[t];

        const float2 pos0 = triangle.vertices[0].position.xy;
        const float2 pos1 = triangle.vertices[1].position.xy; 
        const float2 pos2 = triangle.vertices[2].position.xy;

        const int2 ipos1 = convert_int2(16.0f * pos0);
        const int2 ipos2 = convert_int2(16.0f * pos1);
        const int2 ipos3 = convert_int2(16.0f * pos2);

        // Bounding rectangle

        int2 bounds_min = (min3_int2(ipos1, ipos2, ipos3) + 0xF) >> 4;
        int2 bounds_max = (max3_int2(ipos1, ipos2, ipos3) + 0xF) >> 4;
                    
        if (spos.x < bounds_min.x || spos.x > bounds_max.x)
          continue;
          
        if (spos.y < bounds_min.y || spos.y > bounds_max.y)
          continue;
    
        bounds_min = max(bounds_min, (int2)(0, 0));
        bounds_max = min(bounds_max, (int2)(width, height));
                         
        // Deltas

        const int2 DXY12 = ipos1 - ipos2;
        const int2 DXY23 = ipos2 - ipos3;
        const int2 DXY31 = ipos3 - ipos1;
        
        // Fixed-point deltas

        const int2 FDXY12 = DXY12 << 4;
        const int2 FDXY23 = DXY23 << 4;
        const int2 FDXY31 = DXY31 << 4;

        // Half-edge constants
        int C1 = det_int2(ipos1, DXY12);  
        int C2 = det_int2(ipos2, DXY23);  
        int C3 = det_int2(ipos3, DXY31); 

        // Correct for fill convention
        if(DXY12.y < 0 || (DXY12.y == 0 && DXY12.x > 0)) C1++;
        if(DXY23.y < 0 || (DXY23.y == 0 && DXY23.x > 0)) C2++;
        if(DXY31.y < 0 || (DXY31.y == 0 && DXY31.x > 0)) C3++;     

        // Block size, standard 8x8 (must be power of two)
        const int q = 8;

        // Start in corner of 8x8 block
        bounds_min &= ~(q - 1);
                
        // Loop through blocks
        int2 cur_pos = spos & ~(q - 1);
        
        //for(int y = miny; y < maxy; y += q)
        if (cur_pos.y >= bounds_min.y && cur_pos.y < bounds_max.y)
        {        
            //for(int x = minx; x < maxx; x += q)
            if (cur_pos.x >= bounds_min.x && cur_pos.x < bounds_max.x)
            {
                // Corners of block
                int2 bc0 = cur_pos << 4;
                int2 bc1 = (cur_pos + q - 1) << 4;
                
                // Evaluate half-space functions
                
                bool a00 = C1 + det_int2(DXY12, bc0) > 0;
                bool a10 = C1 + det_int2(DXY12, (int2)(bc1.x, bc0.y)) > 0;
                bool a01 = C1 + det_int2(DXY12, (int2)(bc0.x, bc1.y)) > 0;
                bool a11 = C1 + det_int2(DXY12, bc1) > 0; 

                bool b00 = C2 + det_int2(DXY23, bc0) > 0;
                bool b10 = C2 + det_int2(DXY23, (int2)(bc1.x, bc0.y)) > 0;
                bool b01 = C2 + det_int2(DXY23, (int2)(bc0.x, bc1.y)) > 0;
                bool b11 = C2 + det_int2(DXY23, bc1) > 0; 

                bool c00 = C3 + det_int2(DXY31, bc0) > 0;
                bool c10 = C3 + det_int2(DXY31, (int2)(bc1.x, bc0.y)) > 0;
                bool c01 = C3 + det_int2(DXY31, (int2)(bc0.x, bc1.y)) > 0;
                bool c11 = C3 + det_int2(DXY31, bc1) > 0; 

                int a = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);
                int b = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);
                int c = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);

                // Skip block when outside an edge
                if(a == 0x0 || b == 0x0 || c == 0x0) //continue;
                {
                }
                // Accept whole block when totally covered
                else if(a == 0xF && b == 0xF && c == 0xF)
                {
                    if (sx < width && sy < height) 
                    {
                        TriangleInterpolator_Initialize(&interpolator, pos0, pos1, pos2);
                        ProcessPixel(cbuffer, zbuffer, spos, dim, &interpolator, &triangle, usage, tex0);  
                    }
                }
                else // Partially covered block
                {                    
                    // current block pixel
                    int2 bp = spos - cur_pos;
                
                    int CX1 = C1 + det_int2(DXY12, bc0) + det_int2(FDXY12, bp);
                    int CX2 = C2 + det_int2(DXY23, bc0) + det_int2(FDXY23, bp);
                    int CX3 = C3 + det_int2(DXY31, bc0) + det_int2(FDXY31, bp);
                         
                    if(CX1 > 0 && CX2 > 0 && CX3 > 0)
                    {                               
                        if (sx < width && sy < height) 
                        {
                            TriangleInterpolator_Initialize(&interpolator, pos0, pos1, pos2);
                            ProcessPixel(cbuffer, zbuffer, spos, dim, &interpolator, &triangle, usage, tex0);
                        }
                    }
                    
                }                
            }
        }
        // end of blocks checking loop    
  }
  
  if (sx < width && sy < height)
  {
    //write_imageui(buffer, (int2)(sx, height - 1 - sy), (uint4)(255, 255, 255, 255));
  }
}

