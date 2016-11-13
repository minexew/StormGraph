/*
    Copyright (c) 2011 Xeatheran Minexew

    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

#pragma once

#include <StormGraph/Abstract.hpp>

namespace StormGraph
{
    li_enum_class( ShapeVisibility ) { inside, outside };

    /*struct MeshStaticCreationInfo
    {
        MeshFormat format;
        MeshLayout layout;
        IMaterial* material;

        unsigned numVertices, numIndices;
        List<float> coords, normals, uvs[TEXTURES_PER_MATERIAL], lightUvs;
        List<unsigned> indices;
    };*/

    /**
     *  Structure containing general geometry properties of a cuboid model.
     */
    SgStruct CuboidCreationInfo2
    {
        public:
            const Vector<> pos, size, origin;
            VertexProperties vertexProperties;

            bool front, back, left, right, top, bottom, wireframe;
            ShapeVisibility visibility;

            Vector2<> uv[6][2][TEXTURES_PER_VERTEX], lightUv[6][2];

        public:
            /**
             *  Default constructor, initializing all properties.
             *
             *  Note: Light mapping is supported too, but the UV coordinates must be set manually and enabled in <i>vertexProperties</i>
             */
            CuboidCreationInfo2( const Vector<>& pos, const Vector<>& size, const Vector<>& origin, bool withNormals = false, unsigned numTextures = 0,
                    bool wireframe = false, ShapeVisibility visibility = ShapeVisibility::outside );
    };

    /**
     *  Structure containing BSP-specific properties of a cuboid model.
     */
    SgStruct CuboidBspProperties
    {
        public:
            unsigned materialIndex[6];

        public:
            CuboidBspProperties( unsigned materialIndex );
    };

    /**
     *  Helper class to generate geometry for primitive shapes.
     *
     *  The common side index order for cuboids is: front, back, left, right, top, bottom
     */
    SgClass GeometryFactory
    {
        public:
            static void createCuboidTriangles( const CuboidCreationInfo2& cuboid, List<Vertex>& vertices, List<uint32_t>& indices );

            /**
             *  Generate cuboid BSP geometry.
             *
             *  @param cuboid common properties of the shape
             *  @param bspProperties properties specific to BSP geometry
             *  @param polygons (output) array receiving the generated polygons
             *
             *  @return the actual number of polygon emitted
             */
            static unsigned createCuboid( const CuboidCreationInfo2& cuboid, const CuboidBspProperties& bspProperties, BspPolygon polygons[6] );
    };
}
