#include "forg_pch.h"

#include "mesh/XLoader.h"

#include "rendering/IRenderDevice.h"
#include "rendering/Vertex.h"
#include "math/Math.h"
#include "PerformanceCounter.h"
#include "debug/dbg.h"

#include "mesh/xfile/xfile.h"

namespace forg { namespace xfile {


struct mesh_data
{
    Vector3* points;
    Vector3* normals;
    Vector2* tverts;
    uint*    indices;

    uint     num_vertices;
    uint     num_faces;

    mesh_data()
    {
        reset();
    }

    ~mesh_data()
    {
        clear();
    }

    void clear()
    {
        if (points)
            delete [] points;

        if (normals)
            delete [] normals;

        if (tverts)
            delete [] tverts;

        if (indices)
            delete [] indices;

        reset();
    }

    void reset()
    {
        points = 0;
        normals = 0;
        tverts = 0;
        indices = 0;

        num_vertices = 0;
        num_faces = 0;
    }

};

typedef core::vector<mesh_data> MeshDataVec;


bool ExtractMesh(const xfile::IData* xdata, mesh_data* out, Mesh::ExtendedMaterialVec& materials)
{
    //////////////////////////////////////////////////////////////////////////
    // Extract vertices and indices
    //////////////////////////////////////////////////////////////////////////

    uint nVertices = 0;
    uint nFaces = 0;

    xdata->GetSubdata(0)->ToByteArray(&nVertices, sizeof(nVertices));
    xdata->GetSubdata(2)->ToByteArray(&nFaces, sizeof(nFaces));

    out->num_vertices = nVertices;
    out->num_faces = nFaces;

    out->points = new Vector3[out->num_vertices];
    out->indices = new uint[out->num_faces*3];

    // extract points
    const xfile::IData* vert = xdata->GetSubdata(1);
    vert->ToByteArray(out->points, nVertices * sizeof(Vector3));

    // extract indices
    {
        const xfile::IData* faces = xdata->GetSubdata(3);
        uint fsize = faces->GetSize(); // size of byte buffer
        uint icount = fsize >> 2;   // number of uints

        char *fb = new char[fsize];
        uint *iarr = (uint*)fb;

        faces->ToByteArray(fb, fsize);
        // TODO: check if there are 3 indices per face

        // copy indices to our index buffer
        // we assume 4 uints per MeshFace
        uint findex = 0;
        for (uint i=0; i<icount; i+=4, findex++)
        {
            //ASSERT(iarr[i] == 3);
            ASSERT(findex < nFaces);

            out->indices[3*findex+0] = iarr[i+1];

            // swap indices for ccw
            out->indices[3*findex+1] = iarr[i+3];   //iarr[i+2];
            out->indices[3*findex+2] = iarr[i+2];   //iarr[i+3];

            // skip rest of indices if not triangle
            if (iarr[i] != 3)
            {
                DBG_MSG("[XLoader] Warning! Skipping non triangular face %d (%d vertices)\n", findex, iarr[i]);

                i += iarr[i] - 3;
            }
        }

        delete [] fb;
    }

    //////////////////////////////////////////////////////////////////////////
    // Extract optional data
    //////////////////////////////////////////////////////////////////////////

    // Normals
    const IData* xmesh_normals = xdata->FindObject(XTemplateMeshTextureCoords::GUID);
    if (xmesh_normals)
    {
//         DWORD nNormals;
//         array Vector normals[nNormals];
//         DWORD nFaceNormals;
//         array MeshFace faceNormals[nFaceNormals];
        uint num_normals = 0;
        uint num_face_normals = 0;
        xdata->GetSubdata(0)->ToByteArray(&num_normals, sizeof(num_normals));
        xdata->GetSubdata(2)->ToByteArray(&num_face_normals, sizeof(num_face_normals));

        Vector3* normals = new Vector3[num_normals];
        xdata->GetSubdata(1)->ToByteArray(normals, num_normals*sizeof(Vector3));

        out->normals = new Vector3[out->num_vertices];
        const xfile::IData* faces = xdata->GetSubdata(3);
        uint icount = faces->GetSize() >> 2;
        uint *face_normals = new uint[icount];
        faces->ToByteArray(face_normals, icount*4);

        uint findex = 0;
        for (uint i=0; i<icount; i+=4, findex++)
        {
            ASSERT(findex < num_face_normals);

            out->normals[out->indices[3*findex+0]] = normals[ face_normals[i+1] ];
            out->normals[out->indices[3*findex+1]] = normals[ face_normals[i+2] ];
            out->normals[out->indices[3*findex+2]] = normals[ face_normals[i+3] ];

            // skip rest of indices if not triangle
            if (face_normals[i] != 3)
            {
                i += face_normals[i] - 3;
            }
        }

        delete [] face_normals;
        delete [] normals;
    }

    // Texture Coords

    const IData* xtcoords = xdata->FindObject(XTemplateMeshTextureCoords::GUID);
    if (xtcoords)
    {
        uint tc_count = 0;

        xtcoords->GetSubdata(0)->ToByteArray(&tc_count, sizeof(tc_count));

        out->tverts = new Vector2[out->num_vertices];

        xtcoords->GetSubdata(1)->ToByteArray(out->tverts, sizeof(Vector2)*out->num_vertices);
    }

    // Materials

    const IData* xmat_list = xdata->FindObject(XTemplateMeshMaterialList::GUID);
    if (xmat_list)
    {
//         DWORD nMaterials;
//         DWORD nFaceIndexes;
//         array DWORD faceIndexes[nFaceIndexes];
//         [Material <3D82AB4D-62DA-11CF-AB39-0020AF71E433>]

        uint nMaterials = 0;
        uint nFaceIndexes = 0;

        xmat_list->GetSubdata(0)->ToByteArray(&nMaterials, sizeof(nMaterials));
        xmat_list->GetSubdata(1)->ToByteArray(&nFaceIndexes, sizeof(nFaceIndexes));

        if (nMaterials > 0)
        {
            ExtendedMaterial mesh_emat;

            const IData* xmat = xmat_list->FindObject(xfile::XTemplateMaterial::GUID);
            if (xmat != 0)
            {
                Color3f spec_col;
                Color3f emis_col;

                xmat->GetSubdata(0)->ToByteArray(&mesh_emat.Material3D.Diffuse, sizeof(Color));
                xmat->GetSubdata(1)->ToByteArray(&mesh_emat.Material3D.Power, sizeof(float));
                xmat->GetSubdata(2)->ToByteArray(&spec_col, sizeof(Color3f));
                xmat->GetSubdata(3)->ToByteArray(&emis_col, sizeof(Color3f));

                mesh_emat.Material3D.Specular = Color(spec_col.r, spec_col.g, spec_col.b);
                mesh_emat.Material3D.Emissive = Color(emis_col.r, emis_col.g, emis_col.b);

                const IData* xtexfilename = xmat->FindObject(xfile::XTemplateTextureFilename::GUID);
                if (xtexfilename)
                {
                    uint ssize = xtexfilename->GetSize();
                    mesh_emat.TextureFilename.resize(ssize);
                    xtexfilename->ToByteArray(&mesh_emat.TextureFilename[0], ssize);
                }

                materials.push_back(mesh_emat);
            }
        }
    }

    return true;
}

Mesh::MeshPtr BuildMesh(IRenderDevice* device, MeshDataVec& data)
{
    Mesh::MeshPtr m(0);
    core::vector<AttributeRange> attribs;
    uint vert_total = 0;
    uint faces_total = 0;

    for (uint i=0; i<data.size(); i++)
    {
        vert_total += data[i].num_vertices;
        faces_total += data[i].num_faces;
    }

    if (vert_total == 0 || faces_total == 0)
    {
        return m;
    }

    attribs.resize(data.size());

    m = Mesh::MeshPtr(
            new Mesh( faces_total, vert_total,
                MeshFlags::Use32Bit,
                PositionNormalTextured::Declaration,
                device )
        );

    PositionNormalTextured *vbuffer = 0;
    uint *ibuffer = 0;
    bool no_error = (m != 0 && m->LockVertexBuffer(0, (void**)&vbuffer) == FORG_OK
                            && m->LockIndexBuffer(0, (void**)&ibuffer) == FORG_OK);

    uint voff = 0;
    uint foff = 0;

    for (uint i=0; i<data.size(); i++)
    {
        attribs[i].AttribId = i;
        attribs[i].FaceStart = foff;
        attribs[i].FaceCount = data[i].num_faces;
        attribs[i].VertexStart = voff;
        attribs[i].VertexCount = data[i].num_vertices;

        uint *ib32 = ibuffer + foff*3;
        // faces first to add correct vertex offset
        for (uint j=0; j<data[i].num_faces*3; j++)
        {
            ib32[j] = data[i].indices[j] + voff;
        }

        for (uint j = 0; j < data[i].num_vertices; j++)
        {
            vbuffer[voff].Position = data[i].points[j];

            if (data[i].tverts)
            {
                vbuffer[voff].Tu = data[i].tverts[j].X;
                vbuffer[voff].Tv = data[i].tverts[j].Y;
            }

            voff++;
        }

        foff += data[i].num_faces;
    }

    //////////////////////////////////////////////////////////////////////////
    // Normal computation
    //////////////////////////////////////////////////////////////////////////
    if (data.size() && data[0].normals == 0)
    {

        for (uint j = 0; j < vert_total; j++)
        {
            vbuffer[j].Normal.Zero();
        }

        uint *ib32 = (uint*)ibuffer;
        for (uint j = 0; j < faces_total; j++)
        {
            uint idx0, idx1, idx2;

            idx0 = ib32[j*3];
            idx1 = ib32[j*3+1];
            idx2 = ib32[j*3+2];

            Vector3 e0 = vbuffer[idx1].Position - vbuffer[idx0].Position;
            Vector3 e1 = vbuffer[idx2].Position - vbuffer[idx0].Position;

            Vector3 normal;
            Vector3::Cross(normal, e0, e1);
            //normal.Normalize();

            vbuffer[idx0].Normal += normal;
            vbuffer[idx1].Normal += normal;
            vbuffer[idx2].Normal += normal;

        }

        for (uint j = 0; j < vert_total; j++)
        {
            vbuffer[j].Normal.Normalize();
        }
    }

    m->UnlockVertexBuffer();
    m->UnlockIndexBuffer();

    m->SetAttributeTable(attribs.get(), attribs.size());

    return m;
}

Mesh::MeshPtr XLoader::Load(
    const char* filename,
    uint /*options*/,
    IRenderDevice* device,
    Mesh::ExtendedMaterialVec& materials
)
{
    xfile::XFile loader;
    Mesh::MeshPtr m(0);

    mesh_data mdata;
    MeshDataVec mesh_list;

    if (loader.Open(filename))
    {
        xfile::XFile::XDataPtrVec objs;     // our stack for DFS search

        //////////////////////////////////////////////////////////////////////////

        loader.GetDataObjects(objs);

        while ( ! objs.empty() )
        {
            const xfile::IData* xobj = objs.back();
            objs.pop_back();

//             if ( xobj->GetGUID() == xfile::XTemplateFrameTransformMatrix::GUID )
//             {
//                 Matrix4 mat;
//
//                 xobj->ToByteArray(&mat, sizeof(mat));
//             }
//             else
            if ( xobj->GetGUID() == xfile::XTemplateMesh::GUID)
            {
                mesh_list.push_back(mesh_data());
                ExtractMesh(xobj, &(mesh_list.back()), materials);

                // TODO: temporary
                //break;
            }

            // push child nodes on our fifo queue
            for (uint sub = 0; sub < xobj->GetSubdataSize(); sub++)
            {
                const xfile::IData* xchild = xobj->GetSubdata(sub);

                if (xchild->IsObject())
                    objs.push_back(xchild);
            }
        }
    }

    return BuildMesh(device, mesh_list);

/*
        if (pVertices && pFaces)
        {
            Mesh::MeshPtr mtmp(
                new Mesh(
                nFacesTotal,
                nVerticesTotal,
                MeshFlags::Use32Bit,
                PositionNormal::Declaration,
                device
                )
                );

            PositionNormal *vbuffer = 0;
            if (mtmp != 0 && mtmp->LockVertexBuffer(0, (void**)&vbuffer) == FORG_OK)
            {
                for (uint j = 0; j < nVerticesTotal; j++)
                {
                    vbuffer[j].Position = pVerticesAll[j];
                }
            }


            char *ibuffer = 0;
            if (mtmp != 0 && mtmp->LockIndexBuffer(0, (void**)&ibuffer) == FORG_OK)
            {
                memcpy(ibuffer, pFacesAll, nFacesTotal*3*4);
            }

            //////////////////////////////////////////////////////////////////////////
            // Normal computation
            //////////////////////////////////////////////////////////////////////////

            for (uint j = 0; j < nVerticesTotal; j++)
            {
                vbuffer[j].Normal.Zero();
            }

            uint *ib32 = (uint*)ibuffer;
            for (uint j = 0; j < nFacesTotal; j++)
            {
                uint idx0, idx1, idx2;

                idx0 = ib32[j*3];
                idx1 = ib32[j*3+1];
                idx2 = ib32[j*3+2];

                Vector3 e0 = vbuffer[idx1].Position - vbuffer[idx0].Position;
                Vector3 e1 = vbuffer[idx2].Position - vbuffer[idx0].Position;

                Vector3 normal;
                Vector3::Cross(normal, e0, e1);
                //normal.Normalize();

                vbuffer[idx0].Normal += normal;
                vbuffer[idx1].Normal += normal;
                vbuffer[idx2].Normal += normal;

            }

            for (uint j = 0; j < nVerticesTotal; j++)
            {
                vbuffer[j].Normal.Normalize();
            }

            mtmp->UnlockVertexBuffer();
            mtmp->UnlockIndexBuffer();

            m = mtmp;
        }

        delete [] pVerticesAll;
        delete [] pFacesAll;*/


    return m;
}


}}
