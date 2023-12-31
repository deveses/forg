#include "forg_pch.h"

#include "rendering/Mesh.h"
#include "rendering/Vertex.h"
#include "math/Math.h"
#include "PerformanceCounter.h"
#include "debug/dbg.h"

#include "mesh/ply/plyfile.h"
#include "mesh/XLoader.h"

namespace forg
{
namespace geometry
{

    /////////////////////////////////////////////////////////////////////////////////////

Mesh::Mesh(
    uint NumFaces,
    uint NumVertices,
    uint Options,
    const VertexElement* pDeclaration,
    LPRENDERDEVICE pDevice
)
    : m_vertex_declaration(pDeclaration)
{
    m_num_faces = NumFaces;
    m_num_vertices = NumVertices;
    m_options = Options;
    m_device = pDevice;
    m_stride_size = m_vertex_declaration.GetVertexSize();

    bool use32 = _fget(m_options, MeshFlags::Use32Bit);

    m_vertex_buffer = VertexBufferPtr( pDevice ? pDevice->CreateVertexBuffer(
                                           m_num_vertices * m_stride_size,
                                           Usage::WriteOnly,
                                           Pool_Managed
                                       ) : 0);
    m_index_buffer =  IndexBufferPtr( pDevice ? pDevice->CreateIndexBuffer(
                                          m_num_faces * 3 * (use32 ? 4 : 2),
                                          Usage::WriteOnly,
                                          ! use32,
                                          Pool_Managed
                                      ) : 0);

}

/////////////////////////////////////////////////////////////////////////////////////
Mesh::~Mesh(void)
{
}

/////////////////////////////////////////////////////////////////////////////////////
Mesh::MeshPtr Mesh::Box(IRenderDevice* device, float width, float height, float depth)
{
    PositionNormalTextured *buffer = 0;
    uint vertices = 24, /*indices = 36,*/ primitives = 12;

    MeshPtr m(
        new Mesh(
            primitives,
            vertices,
            0,
            PositionNormalTextured::Declaration,
            device
        )
    );

    if (m->LockVertexBuffer(0, (void**)&buffer) == FORG_OK)
    {
        Vector3 vmax(width/2.0f, height/2.0f, depth/2.0f);
        Vector3 vmin = -vmax;
        Vector3 sizex = Vector3::XAxis *(width);
        Vector3 sizey = Vector3::YAxis * (height);
        Vector3 sizez = Vector3::ZAxis * (depth);

        // down
        buffer[0].set_Position(vmin);
        buffer[1].set_Position(vmin + sizey);
        buffer[2].set_Position(vmin + sizey + sizex);
        buffer[3].set_Position(vmin + sizex);
        buffer[0].set_Normal(-Vector3::ZAxis);
        buffer[1].set_Normal(-Vector3::ZAxis);
        buffer[2].set_Normal(-Vector3::ZAxis);
        buffer[3].set_Normal(-Vector3::ZAxis);
        buffer[0].set_Texel(0.0f, 0.0f);
        buffer[1].set_Texel(0.0f, 1.0f);
        buffer[2].set_Texel(1.0f, 1.0f);
        buffer[3].set_Texel(1.0f, 0.0f);

        // up
        buffer[4].set_Position(vmin + sizez);
        buffer[5].set_Position(vmin + sizey + sizez);
        buffer[6].set_Position(vmin + sizey + sizex + sizez);
        buffer[7].set_Position(vmin + sizex + sizez);
        buffer[4].set_Normal(Vector3::ZAxis);
        buffer[5].set_Normal(Vector3::ZAxis);
        buffer[6].set_Normal(Vector3::ZAxis);
        buffer[7].set_Normal(Vector3::ZAxis);
        buffer[4].set_Texel(0.0f, 0.0f);
        buffer[5].set_Texel(0.0f, 1.0f);
        buffer[6].set_Texel(1.0f, 1.0f);
        buffer[7].set_Texel(1.0f, 0.0f);

        // left
        buffer[8].set_Position(vmin);
        buffer[9].set_Position(vmin + sizez);
        buffer[10].set_Position(vmin + sizey + sizez);
        buffer[11].set_Position(vmin + sizey);
        buffer[8].set_Normal(-Vector3::XAxis);
        buffer[9].set_Normal(-Vector3::XAxis);
        buffer[10].set_Normal(-Vector3::XAxis);
        buffer[11].set_Normal(-Vector3::XAxis);
        buffer[8].set_Texel(0.0f, 0.0f);
        buffer[9].set_Texel(0.0f, 1.0f);
        buffer[10].set_Texel(1.0f, 1.0f);
        buffer[11].set_Texel(1.0f, 0.0f);

        // back
        buffer[12].set_Position(vmin + sizey);
        buffer[13].set_Position(vmin + sizey + sizez);
        buffer[14].set_Position(vmin + sizex + sizey + sizez);
        buffer[15].set_Position(vmin + sizex + sizey);
        buffer[12].set_Normal(Vector3::YAxis);
        buffer[13].set_Normal(Vector3::YAxis);
        buffer[14].set_Normal(Vector3::YAxis);
        buffer[15].set_Normal(Vector3::YAxis);
        buffer[12].set_Texel(0.0f, 0.0f);
        buffer[13].set_Texel(0.0f, 1.0f);
        buffer[14].set_Texel(1.0f, 1.0f);
        buffer[15].set_Texel(1.0f, 0.0f);

        // right
        buffer[16].set_Position(vmin + sizex + sizey);
        buffer[17].set_Position(vmin + sizex);
        buffer[18].set_Position(vmin + sizex + sizez);
        buffer[19].set_Position(vmin + sizex + sizey + sizez);
        buffer[16].set_Normal(Vector3::XAxis);
        buffer[17].set_Normal(Vector3::XAxis);
        buffer[18].set_Normal(Vector3::XAxis);
        buffer[19].set_Normal(Vector3::XAxis);
        buffer[16].set_Texel(0.0f, 0.0f);
        buffer[17].set_Texel(0.0f, 1.0f);
        buffer[18].set_Texel(1.0f, 1.0f);
        buffer[19].set_Texel(1.0f, 0.0f);

        // front
        buffer[20].set_Position(vmin);
        buffer[21].set_Position(vmin + sizex);
        buffer[22].set_Position(vmin + sizex + sizez);
        buffer[23].set_Position(vmin + sizez);
        buffer[20].set_Normal(-Vector3::YAxis);
        buffer[21].set_Normal(-Vector3::YAxis);
        buffer[22].set_Normal(-Vector3::YAxis);
        buffer[23].set_Normal(-Vector3::YAxis);
        buffer[20].set_Texel(0.0f, 0.0f);
        buffer[21].set_Texel(0.0f, 1.0f);
        buffer[22].set_Texel(1.0f, 1.0f);
        buffer[23].set_Texel(1.0f, 0.0f);

        m->UnlockVertexBuffer();
    }

    ushort *ibuffer = 0;

    if (m->LockIndexBuffer(0, (void**)&ibuffer) == FORG_OK)
    {
        // down
        ibuffer[0] = 3;
        ibuffer[1] = 1;
        ibuffer[2] = 0;
        ibuffer[3] = 3;
        ibuffer[4] = 2;
        ibuffer[5] = 1;

        // up
        ibuffer[6] = 4;
        ibuffer[7] = 5;
        ibuffer[8] = 6;
        ibuffer[9] = 4;
        ibuffer[10] = 6;
        ibuffer[11] = 7;

        // left
        ibuffer[12] = 10;
        ibuffer[13] = 9;
        ibuffer[14] = 8;
        ibuffer[15] = 11;
        ibuffer[16] = 10;
        ibuffer[17] = 8;

        // back
        ibuffer[18] = 14;
        ibuffer[19] = 13;
        ibuffer[20] = 12;
        ibuffer[21] = 15;
        ibuffer[22] = 14;
        ibuffer[23] = 12;

        // right
        ibuffer[24] = 16;
        ibuffer[25] = 17;
        ibuffer[26] = 18;
        ibuffer[27] = 16;
        ibuffer[28] = 18;
        ibuffer[29] = 19;

        // front
        ibuffer[30] = 22;
        ibuffer[31] = 21;
        ibuffer[32] = 20;
        ibuffer[33] = 23;
        ibuffer[34] = 22;
        ibuffer[35] = 20;

        m->UnlockIndexBuffer();
    }

    AttributeRange att;

    att.FaceCount = primitives;
    att.FaceStart = 0;
    att.VertexCount = vertices;
    att.VertexStart = 0;
    att.AttribId = 0;

    m->SetAttributeTable(&att, 1);

    return m;
}

/////////////////////////////////////////////////////////////////////////////////////
Mesh::MeshPtr Mesh::Sphere(
    IRenderDevice* device,
    float radius,
    int slices,
    int stacks)
{
    PositionNormalTextured *buffer = 0;
    ushort *ibuffer = 0;

    // 2 poles + slices x stacks
    uint vertices = 2 + (slices-1) * stacks;
    // 2 pole slices * stacks + (slices - 2) * stacks * 2
    uint primitives = stacks * (slices-2) * 2 + stacks * 2;

    MeshPtr m(
        new Mesh(
            primitives,
            vertices,
            0,
            PositionNormalTextured::Declaration,
            device
        )
    );

    uint vcount = 0;
    uint fcount = 0;

    if (m->LockVertexBuffer(0, (void**)&buffer) == FORG_OK && m->LockIndexBuffer(0, (void**)&ibuffer) == FORG_OK)
    {
        // prepare vertices

        Vector3 rad_vec;
        buffer[vcount].set_Position(0.0f, radius, 0.0f);
        vcount++;

        for (int i=1; i<slices; i++)
        {
            float vangle = Math::PI * i / slices;

            rad_vec.X = 0.0f;
            rad_vec.Y = radius * Math::Cos(vangle);
            rad_vec.Z = radius * Math::Sin(vangle);

            for (int j=0; j<stacks; j++)
            {
                float hangle = 2.0f * Math::PI * j / stacks;

                buffer[vcount].set_Position(
                    rad_vec.Z * Math::Sin(hangle),
                    rad_vec.Y,
                    rad_vec.Z * Math::Cos(hangle)
                );
                vcount++;
            }
        }

        buffer[vcount].set_Position(0.0f, -radius, 0.0f);
        vcount++;

        // prepare faces

        uint vstart = 1;

        // first (top) slice
        for (int i=0; i<stacks; i++)
        {
            ibuffer[fcount*3 + 0] = vstart + i;
            ibuffer[fcount*3 + 1] = vstart + (i + 1)%stacks;
            ibuffer[fcount*3 + 2] = 0;
            fcount++;
        }


        for (int i=0; i<slices-2; i++)
        {
            vstart += stacks;

            for (int j=0; j<stacks; j++)
            {
                uint j_next = (j + 1)%stacks;

                ibuffer[fcount*3 + 0] = vstart + j;
                ibuffer[fcount*3 + 1] = vstart + j_next;
                ibuffer[fcount*3 + 2] = vstart + j - stacks;
                fcount++;

                ibuffer[fcount*3 + 0] = vstart + j_next;
                ibuffer[fcount*3 + 1] = vstart + j_next - stacks;
                ibuffer[fcount*3 + 2] = vstart + j - stacks;
                fcount++;
            }
        }

        for (int i=0; i<stacks; i++)
        {
            ibuffer[fcount*3 + 0] = vstart + (i + 1)%stacks;
            ibuffer[fcount*3 + 1] = vstart + i;
            ibuffer[fcount*3 + 2] = vstart + stacks;
            fcount++;
        }
    }

    if (buffer)
        m->UnlockVertexBuffer();

    if (ibuffer)
        m->UnlockIndexBuffer();

    AttributeRange att;

    att.FaceCount = fcount;
    att.FaceStart = 0;
    att.VertexCount = vcount;
    att.VertexStart = 0;
    att.AttribId = 0;

    m->SetAttributeTable(&att, 1);

    m->ComputeTangentFrame(TangentOptions::CalculateNormals);

    return m;
}

/////////////////////////////////////////////////////////////////////////////////////
Mesh::MeshPtr Mesh::Cylinder(
		IRenderDevice* device,
		float radius1,
		float radius2,
		float length,
		int slices,
		int stacks
		)
{
    PositionNormalTextured *buffer = 0;
    ushort *ibuffer = 0;

    uint vertices = (slices+1)*stacks + 2;
    uint primitives = 2*stacks + 2*slices*stacks;

    MeshPtr m(
        new Mesh(
            primitives,
            vertices,
            0,
            PositionNormalTextured::Declaration,
            device
        )
    );

    uint vcount = 0;
    uint fcount = 0;

    if (m->LockVertexBuffer(0, (void**)&buffer) == FORG_OK && m->LockIndexBuffer(0, (void**)&ibuffer) == FORG_OK)
    {
        // prepare vertices

        Vector3 rad_vec;
        buffer[vcount].set_Position(0.0f, 0.0f, length/2.0f);
        //buffer[vcount].set_Normal(Vector3(0.0f, 0.0f, 1.0f));
        vcount++;
        buffer[vcount].set_Position(0.0f, 0.0f, -length/2.0f);
        vcount++;

        for (int i=0; i<slices+1; i++)
        {
            float t = (float)i / slices;

            rad_vec.Z = length/2.0f - i*length/slices;
            rad_vec.X = (1.0f - t)*radius1 + t*radius2;
            rad_vec.Y = rad_vec.X;

            for (int j=0; j<stacks; j++)
            {
                float hangle = (2.0f * Math::PI * (float)j) / (float)stacks;

                buffer[vcount].set_Position(
                    rad_vec.X * Math::Sin(hangle),
                    rad_vec.Y * Math::Cos(hangle),
                    rad_vec.Z
                );

                //buffer[vcount].set_Normal(Vector3(0.0f, 0.0f, 1.0f));

                vcount++;
            }
        }

        // prepare faces

        uint vstart = 2;

        // first (top) slice
        for (int i=0; i<stacks; i++)
        {
            ibuffer[fcount*3 + 0] = 0;
            ibuffer[fcount*3 + 1] = vstart + (i + 1)%stacks;
            ibuffer[fcount*3 + 2] = vstart + i;
            fcount++;
        }


        for (int i=0; i<slices; i++)
        {
            for (int j=0; j<stacks; j++)
            {
                uint j_next = (j + 1)%stacks;

                ibuffer[fcount*3 + 0] = vstart + j;
                ibuffer[fcount*3 + 1] = vstart + j_next;
                ibuffer[fcount*3 + 2] = vstart + j + stacks;
                fcount++;

                ibuffer[fcount*3 + 0] = vstart + j + stacks;
                ibuffer[fcount*3 + 1] = vstart + j_next;
                ibuffer[fcount*3 + 2] = vstart + j_next + stacks;
                fcount++;
            }

            vstart += stacks;
        }

        for (int i=0; i<stacks; i++)
        {
            ibuffer[fcount*3 + 0] = 1;
            ibuffer[fcount*3 + 2] = vstart + (i + 1)%stacks;
            ibuffer[fcount*3 + 1] = vstart + i;
            fcount++;
        }

    }

    if (buffer)
        m->UnlockVertexBuffer();

    if (ibuffer)
        m->UnlockIndexBuffer();

    AttributeRange att;

    //ASSERT(fcount == primitives);
    //ASSERT(vcount == vertices);

    att.FaceCount = fcount;
    att.FaceStart = 0;
    att.VertexCount = vcount;
    att.VertexStart = 0;
    att.AttribId = 0;

    m->SetAttributeTable(&att, 1);

    m->ComputeTangentFrame(TangentOptions::CalculateNormals);

    return m;
}

/////////////////////////////////////////////////////////////////////////////////////
Mesh::MeshPtr Mesh::Landscape(
        IRenderDevice* _device,
        const Vector3& _span,
        const float* _hmap,
        unsigned int _sizex, unsigned int _sizey
        )
{
    PositionNormalTextured *buffer = 0;
    ushort *ibuffer = 0;

    uint vertices = _sizex * _sizey;
    uint primitives = 2*(_sizex-1) * (_sizey-1);

    MeshPtr m(
        new Mesh(
            primitives,
            vertices,
            0,
            PositionNormalTextured::Declaration,
            _device
        )
    );

    uint vcount = 0;
    uint fcount = 0;

    if (m->LockVertexBuffer(0, (void**)&buffer) == FORG_OK && m->LockIndexBuffer(0, (void**)&ibuffer) == FORG_OK)
    {
        // prepare vertices

        float xstep = 2*_span.X/(_sizex-1);
        float ystep = 2*_span.Z/(_sizey-1);

        for (uint iy=0; iy<_sizey; iy++)
        {
            for (uint ix=0; ix<_sizex; ix++)
            {
                float posx = -_span.X + xstep*ix;
                float posz = -_span.Z + ystep*iy;

                buffer[vcount].set_Position(posx, _span.Y * _hmap[iy*_sizex + ix], posz);
                vcount++;
            }
        }

        // prepare faces

        for (uint iy=1; iy<_sizey; iy++)
        {
            for (uint ix=1; ix<_sizex; ix++)
            {
                // v0-v1
                // v2-v3
                ushort v0=(iy-1)*_sizex + ix-1;
                ushort v1=(iy-1)*_sizex + ix;
                ushort v2=(iy)*_sizex + ix-1;
                ushort v3=(iy)*_sizex + ix;

                ibuffer[3*fcount+0] = v0;
                ibuffer[3*fcount+1] = v2;
                ibuffer[3*fcount+2] = v3;

                fcount++;

                ibuffer[3*fcount+0] = v0;
                ibuffer[3*fcount+1] = v3;
                ibuffer[3*fcount+2] = v1;

                fcount++;
            }
        }
    }

    if (buffer)
        m->UnlockVertexBuffer();

    if (ibuffer)
        m->UnlockIndexBuffer();

    AttributeRange att;

    att.FaceCount = fcount;
    att.FaceStart = 0;
    att.VertexCount = vcount;
    att.VertexStart = 0;
    att.AttribId = 0;

    m->SetAttributeTable(&att, 1);

    m->ComputeTangentFrame(TangentOptions::CalculateNormals);

    return m;
}

/*
Mesh* DefaultMesh::CreateHemisphereMesh(Mesh* mesh, float radius, int density)
{
	density *= 2;
	if (4 > density)
		density = 4;
	float angle = (float)(2 * Math::PI / (double) density); // kat pomiêdzy punktami
	Vector3 creator = Vector3(0, 0, radius); // tworz±ca
	Vector3 axisZ = Vector3(0, 0, radius); // os z
	Vector3 axisX = Vector3(radius, 0, 0); // os x
	Quaternion q = Quaternion::Identity; // kwaternion rotacji
	Matrix m = Matrix::Identity;
	int vertices = density*(density/2-1)+2;
	int indices = density*density + 2*density*(density/2-1);
	int primitives = density*(density-1);

	array <Vertex> vb(vertices);
	array <uint> ib(indices);


	vb[0].set_Position(creator);
	vb[vertices-1].set_Position(Vector3::Empty - creator);

	for (int d = 1; d<density/2; d++)
	{
		q.RotationAxis(axisX, d*angle);
		Quaternion slope = Quaternion::Identity;
		for(int r = 0; r<density; r++)
		{
			int i = 1 + (d-1)*density + r;
			slope.RotationAxis(axisZ, r*angle);
			m.RotationQuaternion(q*slope);
			Vector3 v;
			v.TransformCoord(creator, m);
			vb[i].set_Position(v);

		}
	}

	int next = 0;

	for (int d = 0; d<density/2-1; d++)
	{
		ib[next++] = (short)(1+d*(density));
		for(int r=1; r<density; r++)
		{
			ib[next++] = (short)(1+d*(density)+r);
			ib[next++] = (short)(1+d*(density)+r);
		}
		ib[next++] = (short)(1+d*(density));
	}
	for (int d = 0; d<density; d++)
	{
		ib[next++] = 0;
		for(int r=0; r<density/2-1; r++)
		{
			ib[next++] = (short)(1+r*density+d);
			ib[next++] = (short)(1+r*density+d);
		}
		ib[next++] = (short)(density*(density/2-1)+1);
	}

	mesh->FromArrays(vb, ib, primitives, D3DPT_LINELIST);

	return mesh;
}
*/

/////////////////////////////////////////////////////////////////////////////////////
//ostroslup
Mesh::MeshPtr Mesh::Pyramid(IRenderDevice* device, uint numAngles, float radius, float height)
{
    if (numAngles < 3)
        numAngles = 3;

    PositionNormalTextured *buffer = 0;
    uint primitives = numAngles*2;
    uint indices = primitives*3;
    uint vertices = indices + 2;
    double angle_step = (Math::PI*2.0)/numAngles;

    MeshPtr m(
        new Mesh(
            primitives,
            vertices,
            0,
            PositionNormalTextured::Declaration,
            device
        )
    );

    if (m->LockVertexBuffer(0, (void**)&buffer) == FORG_OK)
    {
        Vector3 rvec(0.0f, 0.0f, 1.0f);
        Vector3 nvec(0.0f, 0.0f, 1.0f);
        Vector3 upvec(0.0f, height, 0.0f);
        Vector3 base_normal(0.0f, -1.0f, 0.0f);
        Vector3 fnormal;	//normalna scianki bocznej
        double angle_cos = 1.0f, angle_sin = 0.0f;

        // cosx = a; sinx = b;
        // cos2x = 2.0*cosx*cosx - 1.0f; sin2x = 2*sinx*cosx;
        // cos3x = cos2x*cosx - sin2x*sinx; sin3x = sin2x*cosx+cos2x*sinx

        for (uint i=0; i<numAngles; i++)
        {
            // wektor wiodacy
            rvec.X = (float) (radius * angle_cos);
            rvec.Z = (float) (radius * angle_sin);

            angle_cos = Math::Cos((i+1)*angle_step);
            angle_sin = Math::Sin((i+1)*angle_step);

            // nastepna pozycja
            nvec.X = (float) (radius * angle_cos);
            nvec.Z = (float) (radius * angle_sin);

            //base triangle
            buffer[3*i+0].set_Position(nvec);
            buffer[3*i+0].set_Normal(base_normal);
            buffer[3*i+0].set_Texel(0.0f, 0.0f);

            buffer[3*i+1].set_Position(rvec);
            buffer[3*i+1].set_Normal(base_normal);
            buffer[3*i+1].set_Texel(1.0f, 0.0f);

            buffer[3*i+2].set_Position(Vector3::Empty);
            buffer[3*i+2].set_Normal(base_normal);
            buffer[3*i+2].set_Texel(0.5f, 1.0f);

            //slope triangle
            fnormal.Cross(nvec - upvec, rvec - upvec);
            fnormal.Normalize();
            buffer[3*(i+numAngles)+0].set_Position(rvec);
            buffer[3*(i+numAngles)+0].set_Normal(fnormal);
            buffer[3*(i+numAngles)+0].set_Texel(0.0f, 0.0f);

            buffer[3*(i+numAngles)+1].set_Position(nvec);
            buffer[3*(i+numAngles)+1].set_Normal(fnormal);
            buffer[3*(i+numAngles)+1].set_Texel(1.0f, 0.0f);

            buffer[3*(i+numAngles)+2].set_Position(upvec);
            buffer[3*(i+numAngles)+2].set_Normal(fnormal);
            buffer[3*(i+numAngles)+2].set_Texel(0.5f, 1.0f);
        }

        m->UnlockVertexBuffer();
    }
    else
    {
        m.reset();
    }

    ushort *ibuffer = 0;

    if (m != 0 && m->LockIndexBuffer(0, (void**)&ibuffer) == FORG_OK)
    {
        //uint indhalf = indices >> 1;
        for (uint i=0; i<indices; i++)
        {
            ibuffer[i] = i;
        }

        m->UnlockIndexBuffer();
    }

    return m;
}

/////////////////////////////////////////////////////////////////////////////////////
Mesh::MeshPtr Mesh::Grid(
    IRenderDevice* device,
    float sizeX,
    float sizeY,
    int color,
    uint subgrid
)
{
    PositionColored *buffer = 0;
    uint primitives = (subgrid + 2) << 1;   // 4 + 2*s = (s+2)*2
    //uint indices = primitives*2;
    uint vertices = (subgrid + 1) << 2;    // 4 + 4*s = (s+1)*4

    if (sizeX < 0.0f) sizeX = -sizeX;
    if (sizeY < 0.0f) sizeY = -sizeY;

    MeshPtr m(
        new Mesh(
            primitives,
            vertices,
            0,
            PositionColored::Declaration,
            device
        )
    );

    if (m->LockVertexBuffer(0, (void**)&buffer) == FORG_OK)
    {
        float halfr = sizeY / 2.0f;
        float halfc = sizeX / 2.0f;

        float vstart_X = - halfc;
        float vstart_Z = - halfr;

        float stepc = sizeX / (1 + subgrid);
        float stepr = sizeY / (1 + subgrid);

        // frame
        buffer[0].set_Position(-halfc, 0.0f, -halfr);
        buffer[1].set_Position( halfc, 0.0f, -halfr);
        buffer[2].set_Position( halfc, 0.0f,  halfr);
        buffer[3].set_Position(-halfc, 0.0f,  halfr);
        buffer[0].set_Color(color);
        buffer[1].set_Color(color);
        buffer[2].set_Color(color);
        buffer[3].set_Color(color);

        uint voff = 4;
        for (uint row = 0; row < subgrid; row++)
        {
            buffer[voff + 2*row+0].set_Position( halfc, 0.0f, vstart_Z + (row+1)*stepr);
            buffer[voff + 2*row+1].set_Position(-halfc, 0.0f, vstart_Z + (row+1)*stepr);

            buffer[voff + 2*row+0].set_Color(color);
            buffer[voff + 2*row+1].set_Color(color);
        }

        voff += (subgrid << 1);
        for (uint col = 0; col < subgrid; col++)
        {
            buffer[voff + 2*col+0].set_Position( vstart_X + (col+1)*stepc, 0.0f, halfr);
            buffer[voff + 2*col+1].set_Position( vstart_X + (col+1)*stepc, 0.0f, -halfr);

            buffer[voff + 2*col+0].set_Color(color);
            buffer[voff + 2*col+1].set_Color(color);
        }

        m->UnlockVertexBuffer();
    }
    else
    {
        m.reset();
    }

    ushort *ibuffer = 0;

    if (m != 0 && m->LockIndexBuffer(0, (void**)&ibuffer) == FORG_OK)
    {
        // frame
        ibuffer[0] = 0;
        ibuffer[1] = 1;

        ibuffer[2] = 1;
        ibuffer[3] = 2;

        ibuffer[4] = 2;
        ibuffer[5] = 3;

        ibuffer[6] = 3;
        ibuffer[7] = 0;

        uint ioff = 8;
        uint voff = 4;

        for (uint row = 0; row < subgrid; row++)
        {
            ibuffer[ioff + 2*row] = voff + 2*row;
            ibuffer[ioff + 2*row+1] = voff + 2*row+1;
        }

        ioff += 2*subgrid;
        voff += 2*subgrid;

        for (uint col = 0; col < subgrid; col++)
        {
            ibuffer[ioff + 2*col] = voff + 2*col;
            ibuffer[ioff + 2*col+1] = voff + 2*col+1;
        }

        m->UnlockIndexBuffer();
    }

    return m;
}

/////////////////////////////////////////////////////////////////////////////////////
Mesh::MeshPtr Mesh::FromFile(
    const char* filename,
    uint options,
    IRenderDevice* device
)
{
    ExtendedMaterialVec tmp_mat;

    return FromFile(filename, options, device, tmp_mat);
}

/////////////////////////////////////////////////////////////////////////////////////
Mesh::MeshPtr Mesh::FromFile(
    const char* filename,
    uint options,
    IRenderDevice* device,
    ExtendedMaterialVec& materials
)
{
    std::string fname = filename;
    MeshPtr m;

    PerformanceCounter perf;

    perf.Start();

    if (fname.rfind(".ply") != std::string::npos)
        m = FromPly(filename, options, device);
    else
        m = FromX(filename, options, device, materials);

    perf.Stop();

    uint64 dur;
    perf.GetDurationInMs(dur);
    DBG_MSG("[Mesh::FromFile] took %d\n", dur);

    return m;
}

/////////////////////////////////////////////////////////////////////////////////////
LPVERTEXBUFFER Mesh::GetVertexBuffer() const
{
    return m_vertex_buffer.get();
}

/////////////////////////////////////////////////////////////////////////////////////
LPINDEXBUFFER Mesh::GetIndexBuffer() const
{
    return m_index_buffer.get();
}

/////////////////////////////////////////////////////////////////////////////////////
const VertexDeclaration* Mesh::GetVertexDeclaration() const
{
    return &m_vertex_declaration;
}

/////////////////////////////////////////////////////////////////////////////////////
uint Mesh::GetNumVertices() const
{
    return m_num_vertices;
}

/////////////////////////////////////////////////////////////////////////////////////
uint Mesh::GetNumFaces() const
{
    return m_num_faces;
}

/////////////////////////////////////////////////////////////////////////////////////
uint Mesh::GetNumBytesPerVertex() const
{
    return m_stride_size;
}

/////////////////////////////////////////////////////////////////////////////////////
uint Mesh::GetOptions() const
{
    return m_options;
}

/////////////////////////////////////////////////////////////////////////////////////
int Mesh::LockVertexBuffer(
    uint /*Flags*/,
    void** ppData
)
{
    if (m_vertex_buffer != 0)
        return m_vertex_buffer->Lock(0, 0, ppData, 0);

    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////
int Mesh::LockIndexBuffer(
    uint /*Flags*/,
    void** ppData
)
{
    if (m_index_buffer != 0)
        return m_index_buffer->Lock(0, 0, ppData, 0);

    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////
int Mesh::UnlockVertexBuffer()
{
    if (m_vertex_buffer != 0)
        return m_vertex_buffer->Unlock();

    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////
int Mesh::UnlockIndexBuffer()
{
    if (m_index_buffer != 0)
        return m_index_buffer->Unlock();

    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////
int Mesh::SetAttributeTable(const AttributeRange* pAttribTable, uint cAttribTableSize)
{
    m_attribtab.resize(cAttribTableSize);

    for (uint i=0; i<cAttribTableSize; i++)
    {
        m_attribtab[i] = pAttribTable[i];
    }

    return FORG_OK;
}

/////////////////////////////////////////////////////////////////////////////////////
int Mesh::DrawSubset(uint attributeID)
{
    if (attributeID < m_attribtab.size())
    {
        m_device->SetStreamSource(0, m_vertex_buffer.get(), 0, m_stride_size);
        m_device->SetIndices(m_index_buffer.get());
        m_device->SetVertexDeclaration(&m_vertex_declaration);
        //m_device->SetTexture(0, m_texture.get());
        m_device->DrawIndexedPrimitive(
            PrimitiveType_TriangleList,
            0,
            m_attribtab[attributeID].VertexStart,
            m_attribtab[attributeID].VertexCount,
            m_attribtab[attributeID].FaceStart*3,
            m_attribtab[attributeID].FaceCount);

    }

    return FORG_OK;
}

/////////////////////////////////////////////////////////////////////////////////////
void Mesh::ComputeTangentFrame(uint options)
{
    char* vbuffer = 0;
    ushort *ibuffer = 0;

    uint stride = m_vertex_declaration.GetVertexSize();
    int p_off = -1;
    int n_off = -1;

    for (uint e=0; e<m_vertex_declaration.GetElementsCount(); e++)
    {
        const VertexElement* el = m_vertex_declaration.GetDeclaration()+e;

        if (el->Usage == DeclarationUsage_Position && el->Type == DeclarationType_Float3)
        {
            p_off = el->Offset;
        }
        else if (el->Usage == DeclarationUsage_Normal && el->Type == DeclarationType_Float3)
        {
            n_off = el->Offset;
        }
    }

    if (p_off < 0 || n_off < 0)
        return;

    if (LockVertexBuffer(0, (void**)&vbuffer) == FORG_OK && LockIndexBuffer(0, (void**)&ibuffer) == FORG_OK)
    {
        bool use32 = _fget(m_options, MeshFlags::Use32Bit);
        uint primitives = m_num_faces;
        uint vertices = m_num_vertices;

        for (int j = 0; j < vertices; j++)
        {
            Vector3* n = (Vector3*)(vbuffer + j*stride + n_off);
            n->Zero();
        }

        uint *ib32 = (uint*)ibuffer;
        ushort *ib16 = (ushort*)ibuffer;
        for (int j = 0; j < primitives; j++)
        {
            uint idx0, idx1, idx2;


            if (use32)
            {
                idx0 = ib32[j*3];
                idx1 = ib32[j*3+1];
                idx2 = ib32[j*3+2];
            }
            else
            {
                idx0 = ib16[j*3];
                idx1 = ib16[j*3+1];
                idx2 = ib16[j*3+2];
            }

            Vector3* p0 = (Vector3*)(vbuffer + idx0*stride + p_off);
            Vector3* p1 = (Vector3*)(vbuffer + idx1*stride + p_off);
            Vector3* p2 = (Vector3*)(vbuffer + idx2*stride + p_off);

            Vector3* n0 = (Vector3*)(vbuffer + idx0*stride + n_off);
            Vector3* n1 = (Vector3*)(vbuffer + idx1*stride + n_off);
            Vector3* n2 = (Vector3*)(vbuffer + idx2*stride + n_off);

            Vector3 e0 = *p1 - *p0;
            Vector3 e1 = *p2 - *p0;

            Vector3 normal;
            Vector3::Cross(normal, e0, e1);
            //normal.Normalize();

            *n0 += normal;
            *n1 += normal;
            *n2 += normal;

        }

        for (int j = 0; j < vertices; j++)
        {
            Vector3* n = (Vector3*)(vbuffer + j*stride + n_off);
            n->Normalize();
        }
    }

    if (vbuffer)
        UnlockVertexBuffer();

    if (ibuffer)
        UnlockIndexBuffer();
}

/************************************************************************/
/* Mesh loading                                                         */
/************************************************************************/
Mesh::MeshPtr Mesh::FromPly(
    const char* filename,
    uint /*options*/,
    IRenderDevice* device
)
{
    mesh::ply::plyfile loader;

    loader.Open(filename);

    int vertices = loader.GetElementCount("vertex");
    int primitives = loader.GetElementCount("face");

    bool use32 = (vertices > 0xffff);

    MeshPtr m(
        new Mesh(
            primitives,
            vertices,
            use32 ? MeshFlags::Use32Bit : 0,
            PositionNormal::Declaration,
            device
        )
    );

    PositionNormal *vbuffer = 0;
    if (m != 0 && m->LockVertexBuffer(0, (void**)&vbuffer) == FORG_OK)
    {
        loader.GetProperty("vertex", "x", 0, PLY_FLOAT);
        loader.GetProperty("vertex", "y", 4, PLY_FLOAT);
        loader.GetProperty("vertex", "z", 8, PLY_FLOAT);

        for (int j = 0; j < vertices; j++)
        {
            loader.GetElement(&vbuffer[j]);
        }
    }


    char *ibuffer = 0;
    if (m != 0 && m->LockIndexBuffer(0, (void**)&ibuffer) == FORG_OK)
    {
        loader.GetProperty("face", "vertex_indices", 0, use32 ? PLY_UINT : PLY_USHORT);

        int stride = (use32 ? 4 : 2) * 3;
        for (int j = 0; j < primitives; j++)
        {
            loader.GetElement(ibuffer+j*stride);
        }
    }

    m->UnlockVertexBuffer();
    m->UnlockIndexBuffer();
    //////////////////////////////////////////////////////////////////////////
    // Normal computation
    //////////////////////////////////////////////////////////////////////////
    m->ComputeTangentFrame(TangentOptions::CalculateNormals);

    AttributeRange att;

    att.FaceCount = primitives;
    att.FaceStart = 0;
    att.VertexCount = vertices;
    att.VertexStart = 0;
    att.AttribId = 0;

    m->SetAttributeTable(&att, 1);

    return m;
}

Mesh::MeshPtr Mesh::FromX(
    const char* filename,
    uint options,
    IRenderDevice* device,
    ExtendedMaterialVec& materials
)
{
    MeshPtr m(0);

    m = xfile::XLoader::Load(filename, options, device, materials);

    return m;
}


}
}
