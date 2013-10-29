#include "forg_pch.h"

#include "noise.h"
#include <math.h>
#include <stdlib.h>

#define PSIZE 0x100

namespace forg {

int perm_tab[PSIZE];
float grad2d_tab[PSIZE][2];

// cubic S-curve
float easy_curve(float p)
{
    // y = 3*p^2 - 2*p^3
    float p2 = p * p;

    return (p2*(3 - 2*p));
}

// quintic S-curve
float easy_curve2(float p)
{
    // y = 6*p^5 - 15*p^4 + 10*p^3
    float p2 = p * p;
    float p3 = p2 * p;

    return (p3*(6*p2 - 15*p + 10));
}

float lerp1(float a, float b, float t)
{
    return a + t*(b - a);
}

float lerp2(float a, float b, float t)
{
    return a*(1.0f-t) + t*b;
}

// cos interpolation
float lerp(float a, float b, float t)
{
	float ft = t * 3.1415927f;
	float f = (1 - cos(ft)) * 0.5f;

	return  a*(1-f) + b*f;
}

// cubic interpolation
float cubic_interpolate(float a, float b, float t, float c1, float c2)
{
	float P = (c2 - b) - (c1 - a);
	float Q = (c1 - a) - P;
	float R = b - c1;
	float S = a;
	float t2 = t*t;

	return P*t2*t + Q*t2 + R*t + S;
}

////////////////////////////////////////////////////////////////////////////////

float dot2d(float a0, float a1, float b0, float b1)
{
    return (a0*b0 + a1*b1);
}

void normalize2d(float v[2])
{
	float s = sqrt(dot2d(v[0],v[0],v[1],v[1]));

    if (s > 0.00001)
    {
	    v[0] /= s;
	    v[1] /= s;
    } else
    {
        v[0] = 1.0f;
        v[1] = 0.0f;
    }
}



int CloudExpCurve(int v, int CloudCover /*0-255*/, float CloudSharpness/*<=1.0*/)
{
      int c = v - CloudCover;
      if (c < 0) c=0;

      int CloudDensity = 255 - (int)(pow(CloudSharpness, c) * 255);

      return CloudDensity;
}

// =============================================================================

PerlinNoise::PerlinNoise()
: m_bInitialized(false)
{
    m_fPersistence = 0.5f;//1.0f;
    m_nOctavesCount = 6;

    init();
}

void PerlinNoise::init()
{
    m_bInitialized = true;

    int k, j;

    for (int i = 0; i<PSIZE; i++)
    {
        perm_tab[i] = i;

		for (j = 0 ; j < 2 ; j++)
        {
			grad2d_tab[i][j] = (float)((rand() % (PSIZE + PSIZE)) - PSIZE) / PSIZE; // rand in [-1:1]
        	normalize2d(grad2d_tab[i]);
        }
    }

    for (int i = 0; i<PSIZE; i++)
    {
        k = perm_tab[i];
        j = rand() % PSIZE;
        perm_tab[i] = perm_tab[j];
        perm_tab[j] = k;
    }
}

void PerlinNoise::grad2d(float x, float y, float& gx, float& gy)
{
    int index = ((int)x + perm_tab [((int)y % PSIZE)]) % PSIZE;
    gx = grad2d_tab[ index ] [0];
    gy = grad2d_tab[ index ] [1];
}

float PerlinNoise::int_noise2d(float x, float y)
{
    return PerlinNoise_2D(x, y);
}

// interpolate between four pseudo-random 2d gradient vectors
// precomputed for every integer coordinate
float PerlinNoise::GradientNoise_2D(float x, float y)
{
    float x0, y0, x1, y1;
    float s,t,u,v;

    x0 = floor(x);
    x1 = ceil(x);
    y0 = floor(y);
    y1 = ceil(y);

    // compute influence of gradients
    float gx, gy;
    
    grad2d(x0, y0, gx, gy);
    s = dot2d(gx, gy, x - x0, y - y0);
    
    grad2d(x1, y0, gx, gy);
    t = dot2d(gx, gy, x - x1, y - y0);
    
    grad2d(x0, y1, gx, gy);
    u = dot2d(gx, gy, x - x0, y - y1);
    
    grad2d(x1, y1, gx, gy);
    v = dot2d(gx, gy, x - x1, y - y1);

    float sx = easy_curve2(x - x0);
    float sy = easy_curve2(y - y0);
    float a = lerp2(s, t, sx);
    float b = lerp2(u, v, sx);

    return lerp2(a, b, sy);
}

float PerlinNoise::grad_noise2d(float x, float y)
{
    float total = 0.0f;
    float amplitude = 1.0f;
    int frequency = 1;

    for( int i=0; i<m_nOctavesCount; i++)
    {
        if (i > 0)
        {
            frequency <<= i;
            amplitude *= m_fPersistence;
        }

        total = total + GradientNoise_2D(x * frequency, y * frequency) * amplitude;
    }

    return total;
}

// =============================================================================

int grad3[12][3] = {
    {1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},
    {1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},
    {0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1}
};

double dot(int* g, double x, double y) 
{
    return g[0]*x + g[1]*y; 
}

float PerlinNoise::simplex_noise2d(float xin, float yin)
{
    double n0, n1, n2; // Noise contributions from the three corners
    // Skew the input space to determine which simplex cell we're in
    double F2 = 0.5*(sqrt(3.0)-1.0);
    double s = (xin+yin)*F2; // Hairy factor for 2D
    int i = floor(xin+s);
    int j = floor(yin+s);

    double G2 = (3.0-sqrt(3.0))/6.0;
    double t = (i+j)*G2;
    double X0 = i-t; // Unskew the cell origin back to (x,y) space
    double Y0 = j-t;
    double x0 = xin-X0; // The x,y distances from the cell origin
    double y0 = yin-Y0;
    // For the 2D case, the simplex shape is an equilateral triangle.
    // Determine which simplex we are in.
    int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
    if(x0>y0) {i1=1; j1=0;} // lower triangle, XY order: (0,0)->(1,0)->(1,1)
    else {i1=0; j1=1;}      // upper triangle, YX order: (0,0)->(0,1)->(1,1)
    // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
    // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
    // c = (3-sqrt(3))/6

    double x1 = x0 - i1 + G2; // Offsets for middle corner in (x,y) unskewed coords
    double y1 = y0 - j1 + G2;
    double x2 = x0 - 1.0 + 2.0 * G2; // Offsets for last corner in (x,y) unskewed coords
    double y2 = y0 - 1.0 + 2.0 * G2;
    
    // Work out the hashed gradient indices of the three simplex corners
    int ii = i % PSIZE;
    int jj = j % PSIZE;
    int gindex0 = (ii+perm_tab[jj]) % PSIZE;
    int gindex1 = (ii+i1+perm_tab[(jj+j1)%PSIZE]) % PSIZE;
    int gindex2 = (ii+1+perm_tab[(jj+1)%PSIZE]) % PSIZE;
    int gi0 = perm_tab[gindex0] % 12;
    int gi1 = perm_tab[gindex1] % 12;
    int gi2 = perm_tab[gindex2] % 12;

    // Calculate the contribution from the three corners
    double t0 = 0.5 - x0*x0-y0*y0;
    if(t0<0) n0 = 0.0;
    else {
      t0 *= t0;
      n0 = t0 * t0 * dot(grad3[gi0], x0, y0);  // (x,y) of grad3 used for 2D gradient
    }

    double t1 = 0.5 - x1*x1-y1*y1;
    if(t1<0) n1 = 0.0;
    else {
      t1 *= t1;
      n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
    }

    double t2 = 0.5 - x2*x2-y2*y2;
    if(t2<0) n2 = 0.0;
    else {
      t2 *= t2;
      n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
    }
    // Add contributions from each corner to get the final noise value.
    // The result is scaled to return values in the interval [-1,1].
    return 70.0f * (n0 + n1 + n2);
}

// =============================================================================

float PerlinNoise::int_noise(int x)
{
    x = (x<<13) ^ x;
    return ( 1.0f - float( (x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

float PerlinNoise::SmoothNoise_1D(int x)
{
    return int_noise(x)/2  +  int_noise(x-1)/4  +  int_noise(x+1)/4;
}

// one-dimensional coherent noise
float PerlinNoise::InterpolatedNoise_1D(float x)
{
    int ix = (int)floor(x); // integer part
    float fx = x - ix;  // fractional part

    // getting values from 1D grid
    float v1 = SmoothNoise_1D(ix);
    float v2 = SmoothNoise_1D(ix + 1);

    return lerp(v1 , v2 , fx);
}

float PerlinNoise::PerlinNoise_1D(float x)
{
    float total = 0.0f;
    float amplitude = 1.0f;
    int frequency = 1;

    // n = number of octaves = number of noise function to add
    // it's common to use twice the frequency and half the amplitude for each
    // successive noise function added
    for( int i=0; i<m_nOctavesCount; i++)
    {
        if (i > 0)
        {
            amplitude *= m_fPersistence;
            frequency = 1 << i;
        }

        total = total + InterpolatedNoise_1D(x * frequency) * amplitude;
    }

    return total;
}

float PerlinNoise::Noise2D(float x, float y)
{
    int a = (int)x;
    int b = (int)y;

    int n = a + b * 57;
    n = (n<<13) ^ n;
    return ( 1.0f - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

float PerlinNoise::SmoothNoise_2D(float x, float y)
{
    float corners = ( Noise2D(x-1, y-1)+Noise2D(x+1, y-1)+Noise2D(x-1, y+1)+Noise2D(x+1, y+1) ) / 16;
    float sides   = ( Noise2D(x-1, y)  +Noise2D(x+1, y)  +Noise2D(x, y-1)  +Noise2D(x, y+1) ) /  8;
    float center  =  Noise2D(x, y) / 4;
    return corners + sides + center;
}

float PerlinNoise::InterpolatedNoise_2D(float x, float y)
{
    int ix = (int)floor(x);
    float fx = x - ix;

    int iy = (int)floor(y);
    float fy = y - iy;

    float v1 = SmoothNoise_2D(x,     y);
    float v2 = SmoothNoise_2D(x + 1, y);
    float v3 = SmoothNoise_2D(x,     y + 1);
    float v4 = SmoothNoise_2D(x + 1, y + 1);

    float i1 = lerp(v1 , v2 , fx);
    float i2 = lerp(v3 , v4 , fx);

    return lerp(i1 , i2 , fy);
}

float PerlinNoise::PerlinNoise_2D(float x, float y)
{
    float total = 0.0f;
    float amplitude = 1.0f;
    int frequency = 1;

    for( int i=0; i<m_nOctavesCount; i++)
    {
        if (i > 0)
        {
            frequency <<= i;
            amplitude *= m_fPersistence;
        }

        total = total + InterpolatedNoise_2D(x * frequency, y * frequency) * amplitude;
    }

    return total;
}



}   // namespace forg

//////////////////////////////////////////////////////////////////////////
/* coherent noise function over 1, 2 or 3 dimensions */
/* (copyright Ken Perlin) */
/*
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define B 0x100
#define BM 0xff

#define N 0x1000
#define NP 12
#define NM 0xfff

static p[B + B + 2];
static float g3[B + B + 2][3];
static float g2[B + B + 2][2];
static float g1[B + B + 2];
static start = 1;

static void init(void);

#define s_curve(t) ( t * t * (3. - 2. * t) )

#define lerp(t, a, b) ( a + t * (b - a) )

#define setup(i,b0,b1,r0,r1)\
	t = vec[i] + N;\
	b0 = ((int)t) & BM;\
	b1 = (b0+1) & BM;\
	r0 = t - (int)t;\
	r1 = r0 - 1.;

double noise1(double arg)
{
	int bx0, bx1;
	float rx0, rx1, sx, t, u, v, vec[1];

	vec[0] = arg;
	if (start) {
		start = 0;
		init();
	}

	setup(0, bx0,bx1, rx0,rx1);

	sx = s_curve(rx0);

	u = rx0 * g1[ p[ bx0 ] ];
	v = rx1 * g1[ p[ bx1 ] ];

	return lerp(sx, u, v);
}

float noise2(float vec[2])
{
	int bx0, bx1, by0, by1, b00, b10, b01, b11;
	float rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v;
	register i, j;

	if (start) {
		start = 0;
		init();
	}

	setup(0, bx0,bx1, rx0,rx1);
	setup(1, by0,by1, ry0,ry1);

	i = p[ bx0 ];
	j = p[ bx1 ];

	b00 = p[ i + by0 ];
	b10 = p[ j + by0 ];
	b01 = p[ i + by1 ];
	b11 = p[ j + by1 ];

	sx = s_curve(rx0);
	sy = s_curve(ry0);

#define at2(rx,ry) ( rx * q[0] + ry * q[1] )

	q = g2[ b00 ] ; u = at2(rx0,ry0);
	q = g2[ b10 ] ; v = at2(rx1,ry0);
	a = lerp(sx, u, v);

	q = g2[ b01 ] ; u = at2(rx0,ry1);
	q = g2[ b11 ] ; v = at2(rx1,ry1);
	b = lerp(sx, u, v);

	return lerp(sy, a, b);
}

float noise3(float vec[3])
{
	int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
	float rx0, rx1, ry0, ry1, rz0, rz1, *q, sy, sz, a, b, c, d, t, u, v;
	register i, j;

	if (start) {
		start = 0;
		init();
	}

	setup(0, bx0,bx1, rx0,rx1);
	setup(1, by0,by1, ry0,ry1);
	setup(2, bz0,bz1, rz0,rz1);

	i = p[ bx0 ];
	j = p[ bx1 ];

	b00 = p[ i + by0 ];
	b10 = p[ j + by0 ];
	b01 = p[ i + by1 ];
	b11 = p[ j + by1 ];

	t  = s_curve(rx0);
	sy = s_curve(ry0);
	sz = s_curve(rz0);

#define at3(rx,ry,rz) ( rx * q[0] + ry * q[1] + rz * q[2] )

	q = g3[ b00 + bz0 ] ; u = at3(rx0,ry0,rz0);
	q = g3[ b10 + bz0 ] ; v = at3(rx1,ry0,rz0);
	a = lerp(t, u, v);

	q = g3[ b01 + bz0 ] ; u = at3(rx0,ry1,rz0);
	q = g3[ b11 + bz0 ] ; v = at3(rx1,ry1,rz0);
	b = lerp(t, u, v);

	c = lerp(sy, a, b);

	q = g3[ b00 + bz1 ] ; u = at3(rx0,ry0,rz1);
	q = g3[ b10 + bz1 ] ; v = at3(rx1,ry0,rz1);
	a = lerp(t, u, v);

	q = g3[ b01 + bz1 ] ; u = at3(rx0,ry1,rz1);
	q = g3[ b11 + bz1 ] ; v = at3(rx1,ry1,rz1);
	b = lerp(t, u, v);

	d = lerp(sy, a, b);

	return lerp(sz, c, d);
}

static void normalize2(float v[2])
{
	float s;

	s = sqrt(v[0] * v[0] + v[1] * v[1]);
	v[0] = v[0] / s;
	v[1] = v[1] / s;
}

static void normalize3(float v[3])
{
	float s;

	s = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] = v[0] / s;
	v[1] = v[1] / s;
	v[2] = v[2] / s;
}

static void init(void)
{
	int i, j, k;

	for (i = 0 ; i < B ; i++) {
		p[i] = i;

		g1[i] = (float)((random() % (B + B)) - B) / B;

		for (j = 0 ; j < 2 ; j++)
			g2[i][j] = (float)((random() % (B + B)) - B) / B;
		normalize2(g2[i]);

		for (j = 0 ; j < 3 ; j++)
			g3[i][j] = (float)((random() % (B + B)) - B) / B;
		normalize3(g3[i]);
	}

	while (--i) {
		k = p[i];
		p[i] = p[j = random() % B];
		p[j] = k;
	}

	for (i = 0 ; i < B + 2 ; i++) {
		p[B + i] = p[i];
		g1[B + i] = g1[i];
		for (j = 0 ; j < 2 ; j++)
			g2[B + i][j] = g2[i][j];
		for (j = 0 ; j < 3 ; j++)
			g3[B + i][j] = g3[i][j];
	}
}
*/
