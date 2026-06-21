/*******************************************************************************
    This source file is part of FORG library (http://forg.googlecode.com)
    Copyright (C) 2005  Slawomir Strumecki

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

#ifndef _FORG_GLTFLOADER_H_
#define _FORG_GLTFLOADER_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "rendering/Mesh.h"
#include "rendering/Vertex.h"

#include <vector>

namespace forg { namespace gltf {

    using namespace forg;
    using namespace forg::geometry;

    /// Static glTF 2.0 mesh loader for v1 static models.
    /**
    * Loads positions, normals, texture coordinates, indices, base-color
    * materials and base-color texture filenames from .gltf/.glb files. The
    * default scene is flattened into a single mesh; static node transforms are
    * baked into vertex positions and normals. Animation, skinning, morph
    * targets, cameras and lights are out of scope and ignored gracefully.
    */
    class GltfLoader
    {
    public:
        /// CPU-side flattened geometry for one merged mesh.
        /**
        * Device-independent so the parsing/flattening logic can be unit tested
        * without a render device. @c subsets and @c materials are parallel:
        * subset i has @c AttribId == i and is rendered with @c materials[i].
        */
        struct CpuMesh
        {
	            std::vector<PositionNormalTextured> vertices;
	            std::vector<uint>                   indices;   // triangle list, global, 32-bit
            Mesh::AttributeRangeVec              subsets;   // one per triangle primitive
            Mesh::ExtendedMaterialVec            materials; // parallel to subsets
            bool                                 use32bit;

            CpuMesh() : use32bit(false) {}
        };

        /// Parse a .gltf/.glb file and flatten the default scene into CPU arrays.
        /**
        * @return false if the file cannot be parsed or no triangle geometry
        * could be produced.
        */
        static bool Flatten(const char* filename, CpuMesh& out);

        /// Load a .gltf/.glb file into a device Mesh, filling @p materials.
        /**
        * @return an empty (null) MeshPtr on failure.
        */
        static Mesh::MeshPtr Load(
            const char* filename,
            uint options,
            IRenderDevice* device,
            Mesh::ExtendedMaterialVec& materials
            );
    };
}}

#endif //_FORG_GLTFLOADER_H_
