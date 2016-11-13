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

#include "OpenGlDriver.hpp"

namespace OpenGlDriver
{
    static void generateTriangleCoords( float* output, const Vector<float>& a, const Vector<float>& b, const Vector<float>& c )
    {
        output[0] = a.x;
        output[1] = a.y;
        output[2] = a.z;

        output[3] = b.x;
        output[4] = b.y;
        output[5] = b.z;

        output[6] = c.x;
        output[7] = c.y;
        output[8] = c.z;
    }

    static void generateTriangleUvs( float* output, const Vector2<float>& a, const Vector2<float>& b, const Vector2<float>& c )
    {
        output[0] = a.x;
        output[1] = a.y;

        output[2] = b.x;
        output[3] = b.y;

        output[4] = c.x;
        output[5] = c.y;
    }

    static void generateFaceCoords( float* output, bool reversed, const Vector<float>& a, const Vector<float>& b, const Vector<float>& c, const Vector<float>& d )
    {
        if ( !reversed )
        {
            generateTriangleCoords( output, a, b, d );
            generateTriangleCoords( output + 9, d, b, c );
        }
        else
        {
            generateTriangleCoords( output, d, c, a );
            generateTriangleCoords( output + 9, a, c, b );
        }
    }

    static void generateFaceNormals( float* output, bool reversed, const Vector<float>& a, const Vector<float>& b, const Vector<float>& c, const Vector<float>& d )
    {
        if ( !reversed )
        {
            generateTriangleCoords( output, a, b, d );
            generateTriangleCoords( output + 9, d, b, c );
        }
        else
        {
            generateTriangleCoords( output, -d, -c, -a );
            generateTriangleCoords( output + 9, -a, -c, -b );
        }
    }

    static void generateFaceUvs( float* output, bool reversed, const Vector2<float>& a, const Vector2<float>& b, const Vector2<float>& c, const Vector2<float>& d )
    {
        if ( !reversed )
        {
            generateTriangleUvs( output, a, b, d );
            generateTriangleUvs( output + 6, d, b, c );
        }
        else
        {
            generateTriangleUvs( output, d, c, a );
            generateTriangleUvs( output + 6, a, c, b );
        }
    }

    Mesh* Mesh::createCuboid( OpenGlDriver* driver, CuboidCreationInfo* cuboid, unsigned flags )
    {
        const Vector<> origin( cuboid->origin );
        const Vector<> size( cuboid->dimensions );

        if ( cuboid->wireframe )
        {
            // No texturing (not supported), wireframe
            // If texturing is requested, silently fail

            const float vertices[] =
            {
                -origin.x, -origin.y, -origin.z,
                -origin.x + size.x, -origin.y, -origin.z,
                -origin.x + size.x, -origin.y + size.y, -origin.z,
                -origin.x, -origin.y + size.y, -origin.z,

                -origin.x, -origin.y, -origin.z + size.z,
                -origin.x + size.x, -origin.y, -origin.z + size.z,
                -origin.x + size.x, -origin.y + size.y, -origin.z + size.z,
                -origin.x, -origin.y + size.y, -origin.z + size.z
            };

            static const unsigned indices[] =
            {
                0, 1,
                1, 2,
                2, 3,
                3, 0,

                0, 4,
                1, 5,
                2, 6,
                3, 7,

                4, 5,
                5, 6,
                6, 7,
                7, 4
            };

            MeshCreationInfo3 mesh;
            memset( &mesh, 0, sizeof( mesh ) );

            mesh.format = MeshFormat::lineList;
            mesh.layout = MeshLayout::indexed;
            mesh.material = cuboid->material;

            // 8 * 12 = 96 Bytes
            mesh.numVertices = 8;

            // 24 * 4 = 96 bytes
            mesh.numIndices = 24;

            mesh.coords = vertices;
            mesh.indices = indices;

            return new Mesh( driver, &mesh, flags, true );
        }

        if ( !cuboid->withUvs && !cuboid->withNormals )
        {
            // No texturing, use simple indexed mesh

            const float vertices[] =
            {
                -origin.x, -origin.y, -origin.z,
                -origin.x + size.x, -origin.y, -origin.z,
                -origin.x + size.x, -origin.y + size.y, -origin.z,
                -origin.x, -origin.y + size.y, -origin.z,

                -origin.x, -origin.y, -origin.z + size.z,
                -origin.x + size.x, -origin.y, -origin.z + size.z,
                -origin.x + size.x, -origin.y + size.y, -origin.z + size.z,
                -origin.x, -origin.y + size.y, -origin.z + size.z
            };

            static const unsigned indices[] =
            {
                // bottom, front, back, left, right, top
                3, 0, 2, 0, 1, 2,
                7, 3, 6, 3, 2, 6,
                5, 1, 4, 1, 0, 4,
                4, 0, 7, 0, 3, 7,
                6, 2, 5, 2, 1, 5,
                4, 7, 5, 7, 6, 5
            };

            MeshCreationInfo3 mesh;
            memset( &mesh, 0, sizeof( mesh ) );

            mesh.format = MeshFormat::triangleList;
            mesh.layout = MeshLayout::indexed;
            mesh.material = cuboid->material;

            // 8 * 12 = 96 Bytes
            mesh.numVertices = 8;

            // 36 * 4 = 144 bytes
            mesh.numIndices = 36;

            mesh.coords = vertices;
            mesh.indices = indices;

            return new Mesh( driver, &mesh, flags, true );
        }
        else
        {
            float coords[6 * 6 * 3], normals[6 * 6 * 3], uvs[6 * 6 * 2];
            bool visibleFromInside = cuboid->visibleFromInside;

            // Front
            generateFaceCoords( coords + 0, visibleFromInside, Vector<float>( -origin.x, -origin.y + size.y, -origin.z + size.z ),
                    Vector<float>( -origin.x, -origin.y + size.y, -origin.z ),
                    Vector<float>( -origin.x + size.x, -origin.y + size.y, -origin.z ),
                    Vector<float>( -origin.x + size.x, -origin.y + size.y, -origin.z + size.z ) );

            generateFaceNormals( normals + 0, visibleFromInside,
                    Vector<float>( 0.0f, 1.0f, 0.0f ), Vector<float>( 0.0f, 1.0f, 0.0f ), Vector<float>( 0.0f, 1.0f, 0.0f ), Vector<float>( 0.0f, 1.0f, 0.0f ) );

            // Back
            generateFaceCoords( coords + 18, visibleFromInside, Vector<float>( -origin.x + size.x, -origin.y, -origin.z + size.z ),
                    Vector<float>( -origin.x + size.x, -origin.y, -origin.z ),
                    Vector<float>( -origin.x, -origin.y, -origin.z ),
                    Vector<float>( -origin.x, -origin.y, -origin.z + size.z ) );

            // Left
            generateFaceCoords( coords + 36, visibleFromInside, Vector<float>( -origin.x, -origin.y, -origin.z + size.z ),
                    Vector<float>( -origin.x, -origin.y, -origin.z ),
                    Vector<float>( -origin.x, -origin.y + size.y, -origin.z ),
                    Vector<float>( -origin.x, -origin.y + size.y, -origin.z + size.z ) );

            // Right
            generateFaceCoords( coords + 54, visibleFromInside, Vector<float>( -origin.x + size.x, -origin.y + size.y, -origin.z + size.z ),
                    Vector<float>( -origin.x + size.x, -origin.y + size.y, -origin.z ),
                    Vector<float>( -origin.x + size.x, -origin.y, -origin.z ),
                    Vector<float>( -origin.x + size.x, -origin.y, -origin.z + size.z ) );

            // Top
            generateFaceCoords( coords + 72, visibleFromInside, Vector<float>( -origin.x, -origin.y, -origin.z + size.z ),
                    Vector<float>( -origin.x, -origin.y + size.y, -origin.z + size.z ),
                    Vector<float>( -origin.x + size.x, -origin.y + size.y, -origin.z + size.z ),
                    Vector<float>( -origin.x + size.x, -origin.y, -origin.z + size.z ) );

            generateFaceNormals( normals + 72, visibleFromInside,
                    Vector<float>( 0.0f, 0.0f, -1.0f ), Vector<float>( 0.0f, 0.0f, -1.0f ), Vector<float>( 0.0f, 0.0f, -1.0f ), Vector<float>( 0.0f, 0.0f, -1.0f ) );

            // Bottom
            generateFaceCoords( coords + 90, visibleFromInside, Vector<float>( -origin.x, -origin.y + size.y, -origin.z ),
                    Vector<float>( -origin.x, -origin.y, -origin.z ),
                    Vector<float>( -origin.x + size.x, -origin.y, -origin.z ),
                    Vector<float>( -origin.x + size.x, -origin.y + size.y, -origin.z ) );

            for ( int face = 0; face < 6; face++ )
                generateFaceUvs( uvs + face * 12, visibleFromInside, Vector2<float>( 0.0f, 0.0f ), Vector2<float>( 0.0f, 1.0f ),
                        Vector2<float>( 1.0f, 1.0f ), Vector2<float>( 1.0f, 0.0f ) );

            MeshCreationInfo3 mesh;
            memset( &mesh, 0, sizeof( mesh ) );

            mesh.format = MeshFormat::triangleList;
            mesh.layout = MeshLayout::linear;
            mesh.material = cuboid->material;

            // 36 * 12 = 432 Bytes
            mesh.numVertices = 36;

            mesh.coords = coords;
            mesh.normals = cuboid->withNormals ? normals : nullptr;
            mesh.uvs[0] = cuboid->withUvs ? uvs : nullptr;

            return new Mesh( driver, &mesh, flags, true );
        }
    }

    Mesh* Mesh::createPlane( OpenGlDriver* driver, PlaneCreationInfo* plane, unsigned flags )
    {
        SG_assert3( plane != nullptr, "OpenGlDriver.Mesh.createPlane" )
        SG_assert3( plane->withNormals == false, "OpenGlDriver.Mesh.createPlane" )

        const Vector<float>& dimensions = plane->dimensions, & origin = plane->origin;

        const float coords[] =
        {
            -origin.x, -origin.y, -origin.z,
            -origin.x, -origin.y + dimensions.y, -origin.z,
            -origin.x + dimensions.x, -origin.y, -origin.z,

            -origin.x, -origin.y + dimensions.y, -origin.z,
            -origin.x + dimensions.x, -origin.y + dimensions.y, -origin.z,
            -origin.x + dimensions.x, -origin.y, -origin.z,
        };

        const float uvs[] =
        {
            plane->uv0.x, plane->uv0.y,
            plane->uv0.x, plane->uv1.y,
            plane->uv1.x, plane->uv0.y,

            plane->uv0.x, plane->uv1.y,
            plane->uv1.x, plane->uv1.y,
            plane->uv1.x, plane->uv0.y
        };

        MeshCreationInfo3 mesh;
        memset( &mesh, 0, sizeof( mesh ) );

        mesh.format = MeshFormat::triangleList;
        mesh.layout = MeshLayout::linear;
        mesh.material = ( Material* ) plane->material;

        mesh.numVertices = 6;
        mesh.coords = coords;
        mesh.uvs[0] = plane->withUvs ? uvs : nullptr;

        return new Mesh( driver, &mesh, flags, true );
    }
}
