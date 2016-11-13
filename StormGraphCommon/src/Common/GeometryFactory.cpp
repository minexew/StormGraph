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

#include <StormGraph/GeometryFactory.hpp>

namespace StormGraph
{
    static void setUpCuboidSide( const CuboidCreationInfo2& cuboid, const CuboidBspProperties& bspProperties, BspPolygon polygons[6], unsigned polyIndex, const Vector<> corners[8],
            unsigned side, const Vector<>& normal, unsigned cornerA, unsigned cornerB, unsigned cornerC, unsigned cornerD )
    {
        Vertex vertices[4];

        vertices[0].pos = corners[cornerA];
        vertices[1].pos = corners[cornerB];
        vertices[2].pos = corners[cornerC];
        vertices[3].pos = corners[cornerD];

        if ( cuboid.vertexProperties.hasNormals )
            for ( int i = 0; i < 4; i++ )
                vertices[i].normal = ( cuboid.visibility == ShapeVisibility::outside ) ? normal : -normal;

        for ( unsigned i = 0; i < cuboid.vertexProperties.numTextures; i++ )
            vertices[0].uv[i] = cuboid.uv[side][0][i];

        for ( unsigned i = 0; i < cuboid.vertexProperties.numTextures; i++ )
            vertices[1].uv[i] = Vector2<>( cuboid.uv[side][0][i].x, cuboid.uv[side][1][i].y );

        for ( unsigned i = 0; i < cuboid.vertexProperties.numTextures; i++ )
            vertices[2].uv[i] = cuboid.uv[side][1][i];

        for ( unsigned i = 0; i < cuboid.vertexProperties.numTextures; i++ )
            vertices[3].uv[i] = Vector2<>( cuboid.uv[side][1][i].x, cuboid.uv[side][0][i].y );

        if ( cuboid.vertexProperties.hasLightUvs )
        {
            vertices[0].lightUv = cuboid.lightUv[side][0];
            vertices[1].lightUv = Vector2<>( cuboid.lightUv[side][0].x, cuboid.lightUv[side][1].y );
            vertices[2].lightUv = cuboid.lightUv[side][1];
            vertices[3].lightUv = Vector2<>( cuboid.lightUv[side][1].x, cuboid.lightUv[side][0].y );
        }

        polygons[polyIndex].numVertices = 4;

        if ( cuboid.visibility == ShapeVisibility::outside )
        {
            for ( int i = 0; i < 4; i++ )
                polygons[polyIndex].v[i] = vertices[i];
        }
        else
        {
            for ( int i = 0; i < 4; i++ )
                polygons[polyIndex].v[i] = vertices[3 - i];
        }

        polygons[polyIndex].materialIndex = bspProperties.materialIndex[side];
    }

    static void setUpCuboidSide( const CuboidCreationInfo2& cuboid, List<Vertex>& verticesOut, List<uint32_t>& indices, const Vector<> corners[8],
            unsigned side, const Vector<>& normal, unsigned cornerA, unsigned cornerB, unsigned cornerC, unsigned cornerD )
    {
        Vertex vertices[4];

        vertices[0].pos = corners[cornerA];
        vertices[1].pos = corners[cornerB];
        vertices[2].pos = corners[cornerC];
        vertices[3].pos = corners[cornerD];

        if ( cuboid.vertexProperties.hasNormals )
            for ( int i = 0; i < 4; i++ )
                vertices[i].normal = ( cuboid.visibility == ShapeVisibility::outside ) ? normal : -normal;

        for ( unsigned i = 0; i < cuboid.vertexProperties.numTextures; i++ )
            vertices[0].uv[i] = cuboid.uv[side][0][i];

        for ( unsigned i = 0; i < cuboid.vertexProperties.numTextures; i++ )
            vertices[1].uv[i] = Vector2<>( cuboid.uv[side][0][i].x, cuboid.uv[side][1][i].y );

        for ( unsigned i = 0; i < cuboid.vertexProperties.numTextures; i++ )
            vertices[2].uv[i] = cuboid.uv[side][1][i];

        for ( unsigned i = 0; i < cuboid.vertexProperties.numTextures; i++ )
            vertices[3].uv[i] = Vector2<>( cuboid.uv[side][1][i].x, cuboid.uv[side][0][i].y );

        if ( cuboid.vertexProperties.hasLightUvs )
        {
            vertices[0].lightUv = cuboid.lightUv[side][0];
            vertices[1].lightUv = Vector2<>( cuboid.lightUv[side][0].x, cuboid.lightUv[side][1].y );
            vertices[2].lightUv = cuboid.lightUv[side][1];
            vertices[3].lightUv = Vector2<>( cuboid.lightUv[side][1].x, cuboid.lightUv[side][0].y );
        }

        unsigned index[4];

        if ( cuboid.visibility == ShapeVisibility::outside )
        {
            for ( int i = 0; i < 4; i++ )
                index[i] = verticesOut.add( vertices[i] );
        }
        else
        {
            for ( int i = 0; i < 4; i++ )
                index[i] = verticesOut.add( vertices[3 - i] );
        }

        indices.add( index[0] );
        indices.add( index[1] );
        indices.add( index[3] );
        indices.add( index[3] );
        indices.add( index[1] );
        indices.add( index[2] );
    }

    void GeometryFactory::createCuboidTriangles( const CuboidCreationInfo2& cuboid, List<Vertex>& vertices, List<uint32_t>& indices )
    {
#ifdef __GNUC__
        SG_assert( cuboid.vertexProperties.numTextures <= lengthof( Vertex::uv ) )
#endif

        Vector<> corners[8];

        corners[0] = cuboid.pos - cuboid.origin + Vector<>( 0.0f, 0.0f, 0.0f );
        corners[1] = cuboid.pos - cuboid.origin + Vector<>( cuboid.size.x, 0.0f, 0.0f );
        corners[2] = cuboid.pos - cuboid.origin + Vector<>( cuboid.size.x, cuboid.size.y, 0.0f );
        corners[3] = cuboid.pos - cuboid.origin + Vector<>( 0.0f, cuboid.size.y, 0.0f );
        corners[4] = cuboid.pos - cuboid.origin + Vector<>( 0.0f, 0.0f, cuboid.size.z );
        corners[5] = cuboid.pos - cuboid.origin + Vector<>( cuboid.size.x, 0.0f, cuboid.size.z );
        corners[6] = cuboid.pos - cuboid.origin + Vector<>( cuboid.size.x, cuboid.size.y, cuboid.size.z );
        corners[7] = cuboid.pos - cuboid.origin + Vector<>( 0.0f, cuboid.size.y, cuboid.size.z );

        if ( cuboid.front )
            setUpCuboidSide( cuboid, vertices, indices, corners, 0, Vector<>( 0.0f, 1.0f, 0.0f ), 7, 3, 2, 6 );

        if ( cuboid.back )
            setUpCuboidSide( cuboid, vertices, indices, corners, 1, Vector<>( 0.0f, -1.0f, 0.0f ), 5, 1, 0, 4 );

        if ( cuboid.left )
            setUpCuboidSide( cuboid, vertices, indices, corners, 2, Vector<>( -1.0f, 0.0f, 0.0f ), 4, 0, 3, 7 );

        if ( cuboid.right )
            setUpCuboidSide( cuboid, vertices, indices, corners, 3, Vector<>( 1.0f, 0.0f, 0.0f ), 6, 2, 1, 5 );

        if ( cuboid.top )
            setUpCuboidSide( cuboid, vertices, indices, corners, 4, Vector<>( 0.0f, 0.0f, 1.0f ), 4, 7, 6, 5 );

        if ( cuboid.bottom )
            setUpCuboidSide( cuboid, vertices, indices, corners, 5, Vector<>( 0.0f, 0.0f, -1.0f ), 3, 0, 1, 2 );
    }

    unsigned GeometryFactory::createCuboid( const CuboidCreationInfo2& cuboid, const CuboidBspProperties& bspProperties, BspPolygon polygons[6] )
    {
        SG_assert( cuboid.vertexProperties.numTextures <= TEXTURES_PER_VERTEX )

        Vector<> corners[8];

        corners[0] = cuboid.pos - cuboid.origin + Vector<>( 0.0f, 0.0f, 0.0f );
        corners[1] = cuboid.pos - cuboid.origin + Vector<>( cuboid.size.x, 0.0f, 0.0f );
        corners[2] = cuboid.pos - cuboid.origin + Vector<>( cuboid.size.x, cuboid.size.y, 0.0f );
        corners[3] = cuboid.pos - cuboid.origin + Vector<>( 0.0f, cuboid.size.y, 0.0f );
        corners[4] = cuboid.pos - cuboid.origin + Vector<>( 0.0f, 0.0f, cuboid.size.z );
        corners[5] = cuboid.pos - cuboid.origin + Vector<>( cuboid.size.x, 0.0f, cuboid.size.z );
        corners[6] = cuboid.pos - cuboid.origin + Vector<>( cuboid.size.x, cuboid.size.y, cuboid.size.z );
        corners[7] = cuboid.pos - cuboid.origin + Vector<>( 0.0f, cuboid.size.y, cuboid.size.z );

        unsigned numSides = 0;

        if ( cuboid.front )
            setUpCuboidSide( cuboid, bspProperties, polygons, numSides++, corners, 0, Vector<>( 0.0f, 1.0f, 0.0f ), 7, 3, 2, 6 );

        if ( cuboid.back )
            setUpCuboidSide( cuboid, bspProperties, polygons, numSides++, corners, 1, Vector<>( 0.0f, -1.0f, 0.0f ), 5, 1, 0, 4 );

        if ( cuboid.left )
            setUpCuboidSide( cuboid, bspProperties, polygons, numSides++, corners, 2, Vector<>( -1.0f, 0.0f, 0.0f ), 4, 0, 3, 7 );

        if ( cuboid.right )
            setUpCuboidSide( cuboid, bspProperties, polygons, numSides++, corners, 3, Vector<>( 1.0f, 0.0f, 0.0f ), 6, 2, 1, 5 );

        if ( cuboid.top )
            setUpCuboidSide( cuboid, bspProperties, polygons, numSides++, corners, 4, Vector<>( 0.0f, 0.0f, 1.0f ), 4, 7, 6, 5 );

        if ( cuboid.bottom )
            setUpCuboidSide( cuboid, bspProperties, polygons, numSides++, corners, 5, Vector<>( 0.0f, 0.0f, -1.0f ), 3, 0, 1, 2 );

        return numSides;
    }
}
