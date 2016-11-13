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
#include <StormGraph/GraphicsDriver.hpp>

namespace StormGraph
{
    BspPolygon::BspPolygon( const Polygon& polygon, unsigned materialIndex )
            : numVertices( polygon.numVertices ), materialIndex( materialIndex )
    {
        for ( unsigned i = 0; i < numVertices; i++ )
            v[i] = polygon.v[i];
    }

    CuboidBspProperties::CuboidBspProperties( unsigned materialIndex )
    {
        for ( size_t i = 0; i < lengthof( this->materialIndex ); i++ )
            this->materialIndex[i] = materialIndex;
    }

    CuboidCreationInfo::CuboidCreationInfo() : withNormals( false ), withUvs( false ), wireframe( false ), material( nullptr ),
            visibleFromInside( false )
    {
    }

    CuboidCreationInfo::CuboidCreationInfo( const Vector<>& dimensions, const Vector<>& origin, bool withNormals, bool withUvs,
            bool wireframe, IMaterial* material, bool visibleFromInside )
            : dimensions( dimensions ), origin( origin ), withNormals( withNormals ), withUvs( withUvs ), wireframe( wireframe ),
            material( material ), visibleFromInside( visibleFromInside )
    {
    }

    CuboidCreationInfo2::CuboidCreationInfo2( const Vector<>& pos, const Vector<>& size, const Vector<>& origin, bool withNormals, unsigned numTextures,
            bool wireframe, ShapeVisibility visibility )
            : pos( pos ), size( size ), origin( origin ),
#ifdef li_GCC4
            vertexProperties { withNormals, false, numTextures, false },
#endif
            front( true ), back( true ), left( true ), right( true ), top( true ), bottom( true ), wireframe( wireframe ), visibility( visibility )
    {
#ifndef li_GCC4
        vertexProperties.hasNormals = withNormals;
        vertexProperties.numTextures = numTextures;
        vertexProperties.hasLightUvs = false;
#endif

        // Set default UVs

        if ( numTextures > 0 )
        {
            for ( int side = 0; side < 6; side++ )
            {
                for ( unsigned texture = 0; texture < TEXTURES_PER_VERTEX; texture++ )
                {
                    uv[side][0][texture] = Vector2<>( 0.0f, 0.0f );
                    uv[side][1][texture] = Vector2<>( 1.0f, 1.0f );
                }
            }
        }
    }

    TerrainCreationInfo::TerrainCreationInfo() : heightMap( nullptr ), withNormals( false ), withUvs( false ), wireframe( false ), material( nullptr )
    {
    }

    TerrainCreationInfo::TerrainCreationInfo( IHeightMap* heightMap, const Vector<>& dimensions, const Vector<>& origin, const Vector2<unsigned>& resolution,
            const Vector2<>& uv0, const Vector2<>& uv1, bool withNormals, bool withUvs, bool wireframe, IMaterial* material )
            : heightMap( heightMap ), dimensions( dimensions ), origin( origin ), resolution( resolution ), uv0( uv0 ), uv1( uv1 ),
            withNormals( withNormals ), withUvs( withUvs ), wireframe( wireframe ), material( material )
    {
    }
}
