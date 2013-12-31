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
    float3 texcoord0;
} VSOutput;

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

float4 TriangleInterpolator_Interpolate(TriangleInterpolator* ti, float4 attribs0, float4 attribs1, float4 attribs2)
{
    return attribs0*ti->bary_a + attribs1*ti->bary_b + attribs2*ti->bary_c;
}

void SetPixel(__write_only image2d_t buffer, int2 coords, uint4 col)
{
  write_imageui(buffer, coords, col);
}

__kernel void DrawBlock(__write_only image2d_t buffer, __constant float* vertices, uint vertex_size, uint num_triangles)
{
    int width = get_image_width(buffer);
    int height = get_image_height(buffer);
    int stride = vertex_size/4;
  
    int sx = get_global_id(0);
    int sy = get_global_id(1);
    int2 spos = (int2)(sx, sy);
    
    int lx = get_local_id(0);
    int ly = get_local_id(1);
    
    TriangleInterpolator interpolator;
      
    // GRAB?
    // RGBA -> GRAB
    //uint4 colMask = (uint4)(1, 0, 3, 2);
    uint4 colMask = (uint4)(2, 1, 0, 3);     // RGBA -> BGRA
    
    for (uint t=0; t<num_triangles; t++)
    {
        int tri0 = t*3*stride;
        int tri1 = t*3*stride + stride;
        int tri2 = t*3*stride + 2*stride;

        const float2 pos0 = vload2(0, vertices+tri0);  // (float2)(vertices[tri0], vertices[tri0+1]);
        const float2 pos1 = vload2(0, vertices+tri1);  //(float2)(vertices[tri1], vertices[tri1+1]);
        const float2 pos2 = vload2(0, vertices+tri2);  //(float2)(vertices[tri2], vertices[tri2+1]);

        const float4 col0 = vload4(0, vertices+tri0+4); //(float4)(vertices[tri0+4], vertices[tri0+5], vertices[tri0+6], vertices[tri0+7]);
        const float4 col1 = vload4(0, vertices+tri1+4); //(float4)(vertices[tri1+4], vertices[tri1+5], vertices[tri1+6], vertices[tri1+7]);
        const float4 col2 = vload4(0, vertices+tri2+4); //(float4)(vertices[tri2+4], vertices[tri2+5], vertices[tri2+6], vertices[tri2+7]);

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
                 
        TriangleInterpolator_Initialize(&interpolator, pos0, pos1, pos2);
        
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
                        TriangleInterpolator_SetPoint(&interpolator, (int2)(sx, sy));
                      
                        float4 icol = TriangleInterpolator_Interpolate(&interpolator, col0, col1, col2);
                      
                        uint4 colIn = convert_uint4_sat_rte(icol*255.0f);
                        colIn = shuffle(colIn, colMask);
                      
                        SetPixel(buffer, (int2)(sx, height - 1 - sy), colIn);
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
                            TriangleInterpolator_SetPoint(&interpolator, (int2)(sx, sy));
    
                            float4 icol = TriangleInterpolator_Interpolate(&interpolator, col0, col1, col2);
                            uint4 colEdge = convert_uint4_sat_rte(icol*255.0f);
                            colEdge = shuffle(colEdge, colMask);
    
                            SetPixel(buffer, (int2)(sx, height - 1 - sy), colEdge);  
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

