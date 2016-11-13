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

#include <StormGraph/HeightMap.hpp>

namespace StormGraph
{
    class HeightMap : public IHeightMap
    {
        protected:
            Vector2<unsigned> resolution;
            float* data, ** heights;

            void allocate();

        public:
            HeightMap( const Vector2<unsigned>& resolution );
            virtual ~HeightMap();

            virtual void buildTerrain( const TerrainBuildInfo* buildInfo, List<Vertex>& vertices );
            virtual void buildTerrain( const TerrainBuildInfo* buildInfo, List<BspPolygon>& polygons );

            virtual float get( unsigned x, unsigned y );
            virtual float get( Vector2<float> uv );
            virtual const Vector2<unsigned>& getResolution() { return resolution; }
            virtual void set( unsigned x, unsigned y, float value );
    };

    static const unsigned offsets[2][3][2] = { { { 0, 0 }, { 0, 1 }, { 1, 0 } }, { { 1, 0 }, { 0, 1 }, { 1, 1 } } };

    static float filter( float a )
    {
        return a < 0.5f ? 2 * a * a : 1.0f - 2 * ( 1.0f - a ) * ( 1.0f - a );
    }

    HeightMap::HeightMap( const Vector2<unsigned>& resolution )
            : resolution( resolution )
    {
        SG_assert3( resolution.x >= 2 && resolution.y >= 2, "StormGraph.HeightMap.HeightMap" )

        allocate();
    }

    HeightMap::~HeightMap()
    {
        Allocator<float>::release( heights );
        Allocator<float*>::release( data );
    }

    void HeightMap::allocate()
    {
        data = Allocator<float>::allocate( resolution.x * resolution.y );
        heights = Allocator<float*>::allocate( resolution.x );

        for ( unsigned x = 0; x < resolution.x; x++ )
            heights[x] = data + x * resolution.y;
    }

    void HeightMap::buildTerrain( const TerrainBuildInfo* buildInfo, List<Vertex>& vertices )
    {
        Vector2<> spacing, spacingUv[3], spacingLightUv;
        const Vector<> pos( buildInfo->pos ), origin( buildInfo->origin ), size( buildInfo->size );

        spacing = size.getXy() / ( buildInfo->resolution - 1 );

        for ( int i = 0; i < 3; i++ )
            spacingUv[i] = ( buildInfo->uv[i][1] - buildInfo->uv[i][0] ) / ( buildInfo->resolution - 1 );

        spacingLightUv = ( buildInfo->lightUv[1] - buildInfo->lightUv[0] ) / ( buildInfo->resolution - 1 );

        // *** Generate vertices ***

        for ( unsigned y = 0; y < buildInfo->resolution.y; y++ )
            for ( unsigned x = 0; x < buildInfo->resolution.x; x++ )
            {
                Vertex vertex;

                vertex.pos = pos - origin + Vector<>( x * spacing.x, y * spacing.y,
                        get( Vector2<>( ( float ) x / ( buildInfo->resolution.x ), ( float ) y / ( buildInfo->resolution.y ) ) ) * size.z );

                for ( int i = 0; i < 3; i++ )
                    vertex.uv[i] = buildInfo->uv[i][0] + Vector2<>( x, y ) * spacingUv[i];

                vertex.lightUv = buildInfo->lightUv[0] + Vector2<>( x, y ) * spacingLightUv;

                vertices.add( vertex );
            }

        // *** Calculate normals ***

        Array<unsigned> connections( buildInfo->resolution.x * buildInfo->resolution.y );

        for ( unsigned y = 0; y < buildInfo->resolution.y - 1; y++ )
            for ( unsigned x = 0; x < buildInfo->resolution.x - 1; x++ )
                for ( int tri = 0; tri < 2; tri++ )
                {
                    Vector<> corners[3], sides[2], normal;

                    //printf( "%u, %u, %i: ", x, y, tri );

                    // Get the corners of this polygon
                    for ( int i = 0; i < 3; i++ )
                    {
                        unsigned vertexIndex = ( y + offsets[tri][i][1] ) * buildInfo->resolution.x + x + offsets[tri][i][0];
//printf( "<+%u,%u=%u>", offsets[tri][i][0], offsets[tri][i][1], vertexIndex );
                        corners[i] = vertices[vertexIndex].pos;
                    }

                    //printf( "vertices: [%s]\n\t[%s][%s]\n", corners[0].toString().c_str(),
                    //        corners[1].toString().c_str(), corners[2].toString().c_str() );

                    // Calculate vectors representing two sides of this poly
                    sides[0] = corners[1] - corners[0];
                    sides[1] = corners[2] - corners[0];

                    // Get the cross-product of those two vectors and normalize it to get a direction vector
                    normal = sides[0].crossProduct( sides[1] ).normalize();

                    for ( int i = 0; i < 3; i++ )
                    {
                        unsigned vertexIndex = ( y + offsets[tri][i][1] ) * buildInfo->resolution.x + x + offsets[tri][i][0];

                        // Remember that the normals for this polygon were calculated (accumulate this information per-vertex)
                        connections[vertexIndex]++;

                        // And add this normal to the normal-sum
                        vertices[vertexIndex].normal += normal;

                        //printf( "setting with n = %s\n", vertices[vertexIndex].normal.toString().c_str() );
                    }
                }

        for ( unsigned y = 0; y < buildInfo->resolution.y; y++ )
            for ( unsigned x = 0; x < buildInfo->resolution.x; x++ )
            {
                unsigned vertexIndex = y * buildInfo->resolution.x + x;

                // Divide every normal-sum by the connection count
                if ( connections[vertexIndex] > 0 )
                    vertices[vertexIndex].normal /= connections[vertexIndex];

                // In order for diffuse lighting to work correctly, we need all the normals to point up
                if ( vertices[vertexIndex].normal.z < 0 )
                    vertices[vertexIndex].normal = -vertices[vertexIndex].normal;
            }
    }

    void HeightMap::buildTerrain( const TerrainBuildInfo* buildInfo, List<BspPolygon>& polygons )
    {
        List<Vertex> vertices;

        buildTerrain( buildInfo, vertices );

        // *** Generate triangles ***

        //size_t listOffset = triangles.getLength();

        for ( unsigned y = 0; y < buildInfo->resolution.y - 1; y++ )
            for ( unsigned x = 0; x < buildInfo->resolution.x - 1; x++ )
                /*for ( int tri = 0; tri < 2; tri++ )
                {
                    BspPolygon polygon;

                    polygon.numVertices = 3;

                    for ( int i = 0; i < 3; i++ )
                    {
                        unsigned vertexIndex = ( y + offsets[tri][i][1] ) * buildInfo->resolution.x + x + offsets[tri][i][0];

                    //printf( "adding with n = %s\n", vertices[y0 * buildInfo->resolution.x + x0].normal.toString().c_str() );
                        polygon.v[i] = vertices[vertexIndex];
                    }

                    polygon.materialIndex = buildInfo->materialIndex;

                    polygons.add( polygon );
                }*/

                {
                    BspPolygon polygon;

                    polygon.numVertices = 4;

                    polygon.v[0] = vertices[y * buildInfo->resolution.x + x];
                    polygon.v[1] = vertices[( y + 1 ) * buildInfo->resolution.x + x];
                    polygon.v[2] = vertices[( y + 1 ) * buildInfo->resolution.x + x + 1];
                    polygon.v[3] = vertices[y * buildInfo->resolution.x + x + 1];

                    polygon.materialIndex = buildInfo->materialIndex;

                    polygons.add( polygon );
                }

        //size_t numTriangles = triangles.getLength() - listOffset;
    }

    float HeightMap::get( unsigned x, unsigned y )
    {
        if ( x >= 0 && x < resolution.x && y >= 0 && y < resolution.y )
            return heights[x][y];
        else
            return 0.0f;
    }

    float HeightMap::get( Vector2<float> uv )
    {
        // Bilinear-filtered sampling
        // Also vectors ftw

        uv *= resolution - 1;

        const Vector2<float> uv0 = uv.floor(), uv1 = uv.ceil();

        if ( uv0 == uv1 )
        {
            // Woohoo! Exact hit!

            return get( ( unsigned ) uv0.x, ( unsigned ) uv0.y );
        }
        else if ( uv0.x == uv1.x )
        {
            // X is integer, linear-interpolate Y

            float uv0sample = get( ( unsigned ) uv0.x, ( unsigned ) uv0.y );
            float uv1sample = get( ( unsigned ) uv1.x, ( unsigned ) uv1.y );

            return uv0sample + ( uv1sample - uv0sample ) * filter( uv.y - uv0.y );
        }
        else if ( uv0.y == uv1.y )
        {
            // Y is integer, linear-interpolate X

            float uv0sample = get( ( unsigned ) uv0.x, ( unsigned ) uv0.y );
            float uv1sample = get( ( unsigned ) uv1.x, ( unsigned ) uv1.y );

            return uv0sample + ( uv1sample - uv0sample ) * filter( uv.x - uv0.x );
        }
        else
        {
            // Ouch! Some nasty stuff has to happen here...

            return get( ( unsigned ) uv0.x, ( unsigned ) uv0.y ) * filter( uv1.x - uv.x ) * filter( uv1.y - uv.y )
                    + get( ( unsigned ) uv1.x, ( unsigned ) uv0.y ) * filter( uv.x - uv0.x ) * filter( uv1.y - uv.y )
                    + get( ( unsigned ) uv0.x, ( unsigned ) uv1.y ) * filter( uv1.x - uv.x ) * filter( uv.y - uv0.y )
                    + get( ( unsigned ) uv1.x, ( unsigned ) uv1.y ) * filter( uv.x - uv0.x ) * filter( uv.y - uv0.y );
        }
    }

    void HeightMap::set( unsigned x, unsigned y, float value )
    {
        if ( x >= 0 && x < resolution.x && y >= 0 && y < resolution.y )
            heights[x][y] = value;
    }

    IHeightMap* createHeightMap( IEngine* engine, const Vector2<unsigned>& resolution )
    {
        return new HeightMap( resolution );
    }
}
