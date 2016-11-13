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

#include <StormGraph/HeightMap.hpp>

#define offsetToPtr( i_ ) ( ( unsigned char* )0 + ( i_ ) )

namespace OpenGlDriver
{
    static GLenum getIboUsageByFlags( unsigned flags )
    {
        GLenum usage;

        if ( flags & IModel::streamedIndices )
            usage = GL_STREAM_DRAW;
        else if ( flags & IModel::dynamicIndices )
            usage = GL_DYNAMIC_DRAW;
        else
            usage = GL_STATIC_DRAW;

        return usage;
    }

    static GLenum getVboUsageByFlags( unsigned flags )
    {
        GLenum usage;

        if ( flags & IModel::streamedVertices )
            usage = GL_STREAM_DRAW;
        else if ( flags & IModel::dynamicVertices )
            usage = GL_DYNAMIC_DRAW;
        else
            usage = GL_STATIC_DRAW;

        return usage;
    }

    Mesh::Mesh( OpenGlDriver* driver, MeshPreload* preload, bool finalized )
            : driver( driver ), nextQueued( nullptr )
    {
        SG_assert( preload != nullptr )

        localData = preload;
        fixLocalUvs();

        if ( finalized )
            finalize();
    }

    /*Mesh::Mesh( MeshCreationInfo* mesh, unsigned flags )
    {
        SG_assert( mesh != nullptr )

        initStructs( getRenderMode( mesh->format ), mesh->layout, static_cast<Material*>( mesh->material ) );

        ConstVertices vertices;
        memset( &vertices, 0, sizeof( vertices ) );

        vertices.count = mesh->numVertices;
        vertices.coords = mesh->coords;
        vertices.normals = mesh->normals;
        vertices.uvs[0] = mesh->uvs;

        loadVertices( IModel::fullStatic, &vertices );
        loadIndices( flags, mesh->numIndices, mesh->indices );
    }*/
/*
    Mesh::Mesh( OpenGlDriver* driver, const char* name, GLenum renderMode, MeshLayout layout, bool isSimple, unsigned numVertices, unsigned numIndices )
    {
        data = new MeshPreload;

        data->renderMode = renderMode;
        data->layout = layout;

        data->material = nullptr;

        if ( driverShared.useVertexBuffers )
            loadToBuffers( IModel::fullStatic, numVertices, numIndices, nullptr, nullptr, nullptr, nullptr );
    }*/

    Mesh::Mesh( OpenGlDriver* driver, MeshCreationInfo3* creationInfo, unsigned flags, bool finalize )
            : driver( driver ), nextQueued( nullptr )
    {
        SG_assert( creationInfo != nullptr )

        if ( finalize )
        {
            initStructs( getRenderMode( creationInfo->format ), creationInfo->layout, static_cast<Material*>( creationInfo->material ) );

            ConstVertices vertices;

            vertices.count = creationInfo->numVertices;
            vertices.coords = creationInfo->coords;
            vertices.normals = creationInfo->normals;

            for ( unsigned i = 0; i < TEXTURES_PER_VERTEX; i++ )
                    vertices.uvs[i] = creationInfo->uvs[i];

            vertices.lightUvs = creationInfo->lightUvs;

            loadVertices( flags, &vertices, true );
            loadIndices( flags, creationInfo->numIndices, creationInfo->indices );
        }
        else
        {
            localData = new MeshPreload;
            localData->renderMode = Mesh::getRenderMode( creationInfo->format );
            localData->layout = creationInfo->layout;

            if ( creationInfo->coords != nullptr )
                localData->coords.load( creationInfo->coords, creationInfo->numVertices * 3 );

            if ( creationInfo->normals != nullptr )
                localData->normals.load( creationInfo->normals, creationInfo->numVertices * 3 );

            if ( creationInfo->uvs[0] != nullptr )
                localData->uvs[0].load( creationInfo->uvs[0], creationInfo->numVertices * 2 );

            localData->indices.load( creationInfo->indices, creationInfo->numIndices );

            localData->material = static_cast<Material*>( creationInfo->material );
        }
    }

    Mesh::~Mesh()
    {
        if ( localData != nullptr )
        {
            release( localData->material );
        }

        if ( remoteData != nullptr )
        {
            release( remoteData->material );

            driver->gpuStats.bytesInMeshes -= remoteData->iboCapacity;
            driver->gpuStats.bytesInMeshes -= remoteData->vboCapacity;

            if ( remoteData->vbo )
                glApi.functions.glDeleteBuffers( 1, &remoteData->vbo );

            if ( remoteData->ibo )
                glApi.functions.glDeleteBuffers( 1, &remoteData->ibo );
        }
    }

    void Mesh::beginRender( bool flat )
    {
        // General Mesh Render Setup
        // Binds buffers, arrays and clientstates

        // App-level state caching is performed to avoid redundant state changes
        // (which are expeeeeeeensive)

        if ( driver->renderState.currentMesh == this )
            return;

        if ( driverShared.useVertexBuffers )
        {
            // Renderpath 1: Using Vertex Buffers

            // Bind this mesh's vertex buffer
            glApi.functions.glBindBuffer( GL_ARRAY_BUFFER, remoteData->vbo );

            // Set up coord array (incorrectly called Vertex Array by OpenGL)
            glEnableClientState( GL_VERTEX_ARRAY );

            if ( driver->globalState.currentCoordSource != &remoteData->vbo )
            {
                glVertexPointer( 3, GL_FLOAT, remoteData->vertexSize, offsetToPtr( 0 ) );
                driver->globalState.currentCoordSource = &remoteData->vbo;
            }

            // No need to set up normals & textures if we are rendering depth or picking
            if ( !flat )
            {
                if ( remoteData->vertexProperties.hasNormals )
                {
                    glEnableClientState( GL_NORMAL_ARRAY );
                    glNormalPointer( GL_FLOAT, remoteData->vertexSize, offsetToPtr( remoteData->normalOffset ) );
                }

                for ( unsigned i = 0; i < remoteData->vertexProperties.numTextures; i++ )
                {
                    glApi.functions.glClientActiveTexture( GL_TEXTURE0 + i );
                    glEnableClientState( GL_TEXTURE_COORD_ARRAY );

                    if ( driver->globalState.currentUvSource[i] != &remoteData->vbo )
                    {
                        glTexCoordPointer( 2, GL_FLOAT, remoteData->vertexSize, offsetToPtr( remoteData->uvOffset[i] ) );
                        driver->globalState.currentUvSource[i] = &remoteData->vbo;
                    }
                }

                if ( remoteData->vertexProperties.hasLightUvs )
                {
                    glApi.functions.glClientActiveTexture( GL_TEXTURE0 + remoteData->vertexProperties.numTextures );
                    glEnableClientState( GL_TEXTURE_COORD_ARRAY );

                    if ( driver->globalState.currentUvSource[remoteData->vertexProperties.numTextures] != &remoteData->vbo )
                    {
                        glTexCoordPointer( 2, GL_FLOAT, remoteData->vertexSize, offsetToPtr( remoteData->lightUvOffset ) );
                        driver->globalState.currentUvSource[remoteData->vertexProperties.numTextures] = &remoteData->vbo;
                    }
                }
            }

            if ( remoteData->layout == MeshLayout::indexed )
                glApi.functions.glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, remoteData->ibo );
        }
        else
        {
            // Renderpath 1: Using memory arrays

            glEnableClientState( GL_VERTEX_ARRAY );

            // We're cheating a bit here by using a pointer to localData instead of pointers to individual arrays
            // It allows us to avoid a few unnecessary dereferences
            if ( driver->globalState.currentCoordSource != localData )
            {
                glVertexPointer( 3, GL_FLOAT, 3 * sizeof( float ), *localData->coords );
                driver->globalState.currentCoordSource = localData;
            }

            // No need to set up normals & textures if we are rendering depth or picking
            if ( !flat )
            {
                if ( !localData->normals.isEmpty() )
                {
                    glEnableClientState( GL_NORMAL_ARRAY );
                    glNormalPointer( GL_FLOAT, 3 * sizeof( float ), localData->normals.getPtr() );
                }

                for ( unsigned i = 0; i < TEXTURES_PER_VERTEX && !localData->uvs[i].isEmpty(); i++ )
                {
                    glApi.functions.glClientActiveTexture( GL_TEXTURE0 + i );
                    glEnableClientState( GL_TEXTURE_COORD_ARRAY );

                    if ( driver->globalState.currentUvSource[i] != localData )
                    {
                        glTexCoordPointer( 2, GL_FLOAT, 2 * sizeof( float ), localData->uvs[i].getPtr() );
                        driver->globalState.currentUvSource[i] = localData;
                    }
                }

                if ( !localData->lightUvs.isEmpty() )
                {
                    glApi.functions.glClientActiveTexture( GL_TEXTURE1 );
                    glEnableClientState( GL_TEXTURE_COORD_ARRAY );

                    if ( driver->globalState.currentUvSource[1] != localData )
                    {
                        glTexCoordPointer( 2, GL_FLOAT, 2 * sizeof( float ), localData->lightUvs.getPtr() );
                        driver->globalState.currentUvSource[1] = localData;
                    }
                }
            }
        }

        driver->renderState.currentMesh = this;
    }

#if 0
    // moved to ModelBuiltins.cpp
    Mesh* Mesh::createHexahedron( const Vector<float>& size, const Vector<float>& origin, bool withNormals, bool withUvs, bool wireframe, Material* material );
#endif

#if 0
    // moved to ModelBuiltins.cpp
    Mesh* Mesh::createPlane( PlaneCreationInfo* plane );
#endif

    Mesh* Mesh::createFromHeightMap( OpenGlDriver* driver, TerrainCreationInfo* terrain, unsigned flags, bool finalized )
    {
        Vector2<unsigned> resolution( terrain->resolution );
        Vector<> origin( terrain->origin );

        Vector2<float> maxSample( resolution - 1 );

        Vector2<float> spacing( terrain->dimensions.getXy() / maxSample );
        Vector2<float> uvSpacing( ( terrain->uv1 - terrain->uv0 ) / maxSample );

        List<float> coords, normals, uvs;
        List<unsigned> indices;

        for ( unsigned y = 0; y < resolution.y; y++ )
        {
            for ( unsigned x = 0; x < resolution.x; x++ )
            {
                coords.add( x * spacing.x - origin.x );
                coords.add( y * spacing.y - origin.y );
                coords.add( terrain->heightMap->get( Vector2<float>( x / maxSample.x, y / maxSample.y ) ) * terrain->dimensions.z - origin.z );

                /*vertex.nX = 0.0f;
                vertex.nY = 0.0f;
                vertex.nZ = 0.0f;*/

                normals.add( 0.0f );
                normals.add( 0.0f );
                normals.add( 0.0f );

                uvs.add( terrain->uv0.x + x * uvSpacing.x );
                uvs.add( terrain->uv0.y + y * uvSpacing.y );

                if ( x < resolution.x - 1 && y < resolution.y - 1 )
                {
                    if ( !terrain->wireframe )
                    {
                        indices.add( y * resolution.x + x + 1 );
                        indices.add( y * resolution.x + x );
                        indices.add( ( y + 1 ) * resolution.x + x + 1 );

                        indices.add( ( y + 1 ) * resolution.x + x + 1 );
                        indices.add( y * resolution.x + x );
                        indices.add( ( y + 1 ) * resolution.x + x );
                    }
                    else
                    {
                        indices.add( y * resolution.x + x + 1 );
                        indices.add( y * resolution.x + x );

                        indices.add( y * resolution.x + x );
                        indices.add( ( y + 1 ) * resolution.x + x );

                        indices.add( ( y + 1 ) * resolution.x + x );
                        indices.add( y * resolution.x + x + 1 );
                    }
                }
            }
        }

        // Calculate normals
        Vector<float> corners[3], sides[2], normal;
        Array<unsigned> connections( coords.getLength() / 3 );
        //connections.resize( vertices.getLength() );

        for ( unsigned i = 0; i < indices.getLength(); i += 3 )
        {
            // Get the corners of this polygon
            for ( unsigned j = 0; j < 3; j++ )
                //corners[j] = Vector<float>( vertices[indices[i + j]].x, vertices[indices[i + j]].y, vertices[indices[i + j]].z );
                corners[j] = Vector<float>( coords[indices[i + j] * 3], coords[indices[i + j] * 3 + 1], coords[indices[i + j] * 3 + 2] );

            // Calculate vectors representing two sides of this poly
            sides[0] = corners[1] - corners[0];
            sides[1] = corners[2] - corners[0];

            // Get the cross-product of those two vectors
            //  and normalize it to get a direction vector.
            normal = sides[0].crossProduct( sides[1] ).normalize();

            for ( unsigned j = 0; j < 3; j++ )
            {
                // Remember that the normals for this polygon were calculated,
                //  accumulate this information per-vertex
                connections[indices[i + j]]++;

                // And add the normal itself to the normal-sum
                //vertices[indices[i + j]].nX += normal.x;
                //vertices[indices[i + j]].nY += normal.y;
                //vertices[indices[i + j]].nZ += normal.z;
                normals[indices[i + j] * 3] += normal.x;
                normals[indices[i + j] * 3+1] += normal.y;
                normals[indices[i + j] * 3+2] += normal.z;
            }
        }

        // Loop through the vertices and divide every normal-sum by the calculation-count
        //for each_in_list ( vertices, i )
        for ( size_t i = 0; i < coords.getLength() / 3; i++ )
        {
            if ( connections[i] > 0 )
            {
                //vertices[i].nX /= connections[i];
                //vertices[i].nY /= connections[i];
                //vertices[i].nZ /= connections[i];

                normals[i * 3] /= connections[i];
                normals[i * 3+1] /= connections[i];
                normals[i * 3+2] /= connections[i];
            }

            // In order for diffuse to work, we need all the normals to point up
            //if ( vertices[i].nZ < 0 )
            if ( normals[i*3+2] < 0 )
            {
                //vertices[i].nX = -vertices[i].nX;
                //vertices[i].nY = -vertices[i].nY;
                //vertices[i].nZ = -vertices[i].nZ;

                normals[i * 3] = -normals[i * 3];
                normals[i * 3+1] = -normals[i * 3+1];
                normals[i * 3+2] = -normals[i * 3+2];
            }
        }

        printf( "createFromHeightMap: Generated %" PRIuPTR " vertices, %" PRIuPTR " indices\n", coords.getLength() / 3, indices.getLength() );

        MeshCreationInfo3 creationInfo;
        memset( &creationInfo, 0, sizeof( creationInfo ) );

        creationInfo.format = terrain->wireframe ? MeshFormat::lineList : MeshFormat::triangleList;
        creationInfo.layout = MeshLayout::indexed;
        creationInfo.material = terrain->material;

        creationInfo.numVertices = coords.getLength() / 3;
        creationInfo.numIndices = indices.getLength();

        creationInfo.coords = coords.getPtr();
        creationInfo.normals = normals.getPtr();
        creationInfo.uvs[0] = uvs.getPtr();
        creationInfo.indices = indices.getPtr();

        return new Mesh( driver, &creationInfo, flags, finalized );
    }

    /*void Mesh::createVBO( unsigned flags, unsigned numVertices, const float* coords, const float* normals, const float* uvs )
    {
        this->numVertices = numVertices;

        GLenum usage = getVboUsageByFlags( flags );

        glApi.functions.glGenBuffers( 1, & ( GLuint& ) vbo );
        glApi.functions.glBindBuffer( GL_ARRAY_BUFFER, vbo );

        if ( ( !( flags & IModel::streamedVertices ) && coords != nullptr && normals == nullptr && uvs == nullptr ) || ( coords == nullptr && isSimple ) )
        {
            isSimple = true;
            vboCapacity = numVertices * 3 * sizeof( float );

            glApi.functions.glBufferData( GL_ARRAY_BUFFER, vboCapacity, coords, usage );
        }
        else
        {
            isSimple = false;
            vboCapacity = numVertices * sizeof( Vertex );

            glApi.functions.glBufferData( GL_ARRAY_BUFFER, vboCapacity, nullptr, usage );

            if ( coords != nullptr || normals != nullptr || uvs != nullptr )
            {
                Vertex* vertices = ( Vertex* ) glApi.functions.glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );

                for ( unsigned i = 0; i < numVertices; i++ )
                {
                    Vertex& vertex = vertices[i];

                    if ( coords != nullptr )
                    {
                        vertex.x = coords[i * 3];
                        vertex.y = coords[i * 3 + 1];
                        vertex.z = coords[i * 3 + 2];
                    }

                    if ( normals != nullptr )
                    {
                        vertex.nX = normals[i * 3];
                        vertex.nY = normals[i * 3 + 1];
                        vertex.nZ = normals[i * 3 + 2];
                    }
                    else
                    {
                        vertex.nX = 0;
                        vertex.nY = 0;
                        vertex.nZ = 0;
                    }

                    if ( uvs != nullptr )
                    {
                        vertex.u = uvs[i * 2];
                        vertex.v = 1.0f - uvs[i * 2 + 1];
                    }
                    else
                    {
                        vertex.u = 0;
                        vertex.v = 0;
                    }
                }

                glApi.functions.glUnmapBuffer( GL_ARRAY_BUFFER );
            }
        }

        glApi.functions.glBindBuffer( GL_ARRAY_BUFFER, 0 );
    }

    void Mesh::createIBO( unsigned flags, unsigned numIndices, const unsigned* indices )
    {
        if ( !( flags & IModel::streamedIndices ) )
        {
            indexFormat = GL_UNSIGNED_BYTE;

            for ( size_t i = 0; i < numIndices; i++ )
                if ( i > 0xFFFF )
                {
                    indexFormat = GL_UNSIGNED_INT;
                    break;
                }
                else if ( indexFormat == GL_UNSIGNED_BYTE && i > 0xFF )
                    indexFormat = GL_UNSIGNED_SHORT;
        }
        else
            indexFormat = GL_UNSIGNED_INT;

        glApi.functions.glGenBuffers( 1, & ( GLuint& ) ibo );
        glApi.functions.glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );

        indexSize = 0;

        switch ( indexFormat )
        {
            case GL_UNSIGNED_BYTE:      indexSize = sizeof( GLubyte ); break;
            case GL_UNSIGNED_SHORT:     indexSize = sizeof( GLushort ); break;
            case GL_UNSIGNED_INT:       indexSize = sizeof( GLuint ); break;
        }

        SG_assert( indexSize != 0 )

        iboCapacity = numIndices * indexSize;
        printf( "resizing IBO to %u bytes\n", (unsigned)iboCapacity );

        if ( indices != nullptr )
        {
            const void* indexSource = nullptr;

            Array<GLubyte> ubyteBuffer;
            Array<GLushort> ushortBuffer;

            switch ( indexFormat )
            {
                case GL_UNSIGNED_BYTE:
                    ubyteBuffer.resize( numIndices );

                    for ( size_t i = 0; i < numIndices; i++ )
                        ubyteBuffer.getUnsafe( i ) = indices[i];

                    indexSource = ubyteBuffer.getPtr();
                    break;

                case GL_UNSIGNED_SHORT:
                    ushortBuffer.resize( numIndices );

                    for ( size_t i = 0; i < numIndices; i++ )
                        ushortBuffer.getUnsafe( i ) = indices[i];

                    indexSource = ushortBuffer.getPtr();
                    break;

                case GL_UNSIGNED_INT:
                    indexSource = indices;
                    break;
            }

            SG_assert( indexSource != nullptr )

            glApi.functions.glBufferData( GL_ELEMENT_ARRAY_BUFFER, iboCapacity, indexSource, getIboUsageByFlags( flags ) );
        }
        else
        {
            glApi.functions.glBufferData( GL_ELEMENT_ARRAY_BUFFER, iboCapacity, nullptr, getIboUsageByFlags( flags ) );
        }

        glApi.functions.glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

        this->numIndices = numIndices;
    }
*/
    void Mesh::doRenderAll()
    {
        if ( driverShared.useVertexBuffers )
        {
            if ( remoteData->layout == MeshLayout::indexed )
                doRenderRange( 0, remoteData->numIndices );
            else
                doRenderRange( 0, remoteData->numVertices );
        }
        else
        {
            if ( localData->layout == MeshLayout::indexed )
                doRenderRange( 0, localData->indices.getLength() );
            else
                doRenderRange( 0, localData->coords.getLength() / 3 );
        }
    }

    void Mesh::doRenderRange( size_t offset, size_t count )
    {
        if ( driverShared.useVertexBuffers )
        {
            if ( remoteData->layout == MeshLayout::indexed )
            {
                glDrawElements( remoteData->renderMode, count, remoteData->indexFormat, offsetToPtr( offset * remoteData->indexSize ) );
                stats.numRenderCalls++;

                if ( remoteData->renderMode == GL_TRIANGLES )
                    stats.numPolys += count / 3;
            }
            else
            {
                glDrawArrays( remoteData->renderMode, offset, count );
                stats.numRenderCalls++;

                if ( remoteData->renderMode == GL_TRIANGLES )
                    stats.numPolys += count / 3;
            }
        }
        else
        {
            if ( localData->layout == MeshLayout::indexed )
            {
                glDrawElements( localData->renderMode, count, GL_UNSIGNED_INT, localData->indices.getPtr( offset ) );
                stats.numRenderCalls++;
                stats.numPolys += count / 3;
            }
            else
            {
                glDrawArrays( localData->renderMode, offset, count );
                stats.numRenderCalls++;
                stats.numPolys += count / 3;
            }
        }
    }

    void Mesh::endRender( bool flat )
    {
        /*if ( driverShared.useVertexBuffers )
        {
            if ( remoteData->layout == MeshLayout::indexed )
                glApi.functions.glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

            if ( !flat )
            {
                if ( remoteData->vertexProperties.hasNormals )
                    glDisableClientState( GL_NORMAL_ARRAY );

                for ( unsigned i = 0; i < remoteData->vertexProperties.numTextures; i++ )
                {
                    glApi.functions.glClientActiveTexture( GL_TEXTURE0 + i );
                    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
                }

                if ( remoteData->vertexProperties.hasLightUvs )
                {
                    glApi.functions.glClientActiveTexture( GL_TEXTURE0 + remoteData->vertexProperties.numTextures );
                    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
                }
            }

            glDisableClientState( GL_VERTEX_ARRAY );

            glApi.functions.glBindBuffer( GL_ARRAY_BUFFER, 0 );
        }
        else
        {
            if ( !flat )
            {
                if ( !localData->normals.isEmpty() )
                    glDisableClientState( GL_NORMAL_ARRAY );

                for ( unsigned i = 0; i < TEXTURES_PER_VERTEX && !localData->uvs[i].isEmpty(); i++ )
                {
                    glApi.functions.glClientActiveTexture( GL_TEXTURE0 + i );
                    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
                }
            }

            glDisableClientState( GL_VERTEX_ARRAY );
        }*/
    }

    Mesh* Mesh::finalize()
    {
        if ( localData != nullptr )
        {
            localData->material->finalize();

            if ( driverShared.useShaders )
            {
                static const unsigned flags = IModel::fullStatic;

                initStructs( localData->renderMode, localData->layout, localData->material );

                ConstVertices vertices;

                vertices.count = localData->coords.getLength() / 3;
                vertices.coords = localData->coords.getPtr();
                vertices.normals = !localData->normals.isEmpty() ? localData->normals.getPtr() : nullptr;

                for ( unsigned i = 0; i < TEXTURES_PER_VERTEX; i++ )
                    vertices.uvs[i] = !localData->uvs[i].isEmpty() ? localData->uvs[i].getPtr() : nullptr;

                vertices.lightUvs = !localData->lightUvs.isEmpty() ? localData->lightUvs.getPtr() : nullptr;

                loadVertices( flags, &vertices, false );

                if ( localData->layout == MeshLayout::indexed )
                    loadIndices( flags, localData->indices.getLength(), localData->indices.getPtr() );

                remoteData->material = localData->material;

                localData.release();
            }
        }

        return this;
    }

    void Mesh::fixLocalUvs()
    {
        // Convert from D3D to OpenGL coordinates (flip Y-axis)

        for ( unsigned i = 0; i < TEXTURES_PER_VERTEX; i++ )
            for ( unsigned j = 1; j < localData->uvs[i].getLength(); j += 2 )
                localData->uvs[i][j] = 1.0f - localData->uvs[i][j];
    }

    size_t Mesh::getNumVertices()
    {
        if ( driverShared.useVertexBuffers )
            return remoteData->numVertices;
        else
            return localData->coords.getLength() / 3;
    }

    GLenum Mesh::getRenderMode( MeshFormat format )
    {
        switch ( format )
        {
            case MeshFormat::lineList: return GL_LINES;
            case MeshFormat::lineStrip: return GL_LINE_STRIP;
            case MeshFormat::pointList: return GL_POINTS;
            case MeshFormat::triangleList: return GL_TRIANGLES;

            default:
                SG_assert4( false, "Unknown MeshFormat" )
        }
    }

    void Mesh::initStructs( GLenum renderMode, MeshLayout layout, Material* material )
    {
        if ( driverShared.useVertexBuffers )
        {
            remoteData = new RemoteData;
            memset( remoteData, 0, sizeof( RemoteData ) );

            remoteData->renderMode = renderMode;
            remoteData->layout = layout;
            remoteData->material = material;
        }
        else
        {
            localData = new MeshPreload;

            localData->renderMode = renderMode;
            localData->layout = layout;
            localData->material = material;
        }
    }

    /*void Mesh::loadToBuffers( unsigned flags, unsigned numVertices, unsigned numIndices, const float* coords, const float* normals, const float* uvs, const unsigned* indices )
    {
        switch ( data->layout )
        {
            case MeshLayout::linear:
                createVBO( flags, numVertices, coords, normals, uvs );

                if ( isSimple )
                    printf( "Simple mesh: VBO - %u x %" PRIuPTR " bytes = %" PRIuPTR " bytes\n", numVertices, 3 * sizeof( float ), numVertices * 3 * sizeof( float ) );
                else
                    printf( "Textured/lit mesh: VBO - %u x %" PRIuPTR " bytes = %" PRIuPTR " bytes\n", numVertices, sizeof( Vertex ), numVertices * sizeof( Vertex ) );
                break;

            case MeshLayout::indexed:
                createVBO( flags, numVertices, coords, normals, uvs );
                createIBO( flags, numIndices, indices );

                if ( isSimple )
                    printf( "Simple mesh: VBO - %u x %" PRIuPTR " bytes = %" PRIuPTR " bytes; IBO - %u x %u bytes = %u bytes; %" PRIuPTR " bytes total\n",
                            numVertices, 3 * sizeof( float ), numVertices * 3 * sizeof( float ), numIndices, indexSize, numIndices * indexSize, numVertices * 3 * sizeof( float ) + numIndices * indexSize );
                else
                    printf( "Textured/lit mesh: VBO - %u x %" PRIuPTR " bytes = %" PRIuPTR " bytes; IBO - %u x %u bytes = %u bytes; %" PRIuPTR " bytes total\n",
                            numVertices, sizeof( Vertex ), numVertices * sizeof( Vertex ), numIndices, indexSize, numIndices * indexSize, numVertices * sizeof( Vertex ) + numIndices * indexSize );
                break;
        }
    }*/

    /*void Mesh::loadToLocal( unsigned numVertices, unsigned numIndices, const float* coords, const float* normals, const float* uvs, const unsigned* indices )
    {
        data->coords.load( coords, numVertices * 3 );

        if ( normals )
            data->normals.load( normals, numVertices * 3 );

        if ( uvs )
            data->uvs.load( uvs, numVertices * 2 );

        if ( indices )
            data->indices.load( indices, numIndices );
    }*/

    void Mesh::loadIndices( unsigned flags, size_t count, const unsigned* indices )
    {
        if ( driverShared.useVertexBuffers )
        {
            RemoteData* remoteData = this->remoteData;

            if ( remoteData->layout != MeshLayout::indexed )
                return;

            if ( !( flags & IModel::streamedIndices ) )
            {
                remoteData->indexFormat = GL_UNSIGNED_BYTE;

                for ( size_t i = 0; i < count; i++ )
                    if ( i > 0xFFFF )
                    {
                        remoteData->indexFormat = GL_UNSIGNED_INT;
                        break;
                    }
                    else if ( remoteData->indexFormat == GL_UNSIGNED_BYTE && i > 0xFF )
                        remoteData->indexFormat = GL_UNSIGNED_SHORT;
            }
            else
                remoteData->indexFormat = GL_UNSIGNED_INT;

            glApi.functions.glGenBuffers( 1, &remoteData->ibo );
            glApi.functions.glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, remoteData->ibo );

            remoteData->indexSize = 0;

            switch ( remoteData->indexFormat )
            {
                case GL_UNSIGNED_BYTE: remoteData->indexSize = sizeof( GLubyte ); break;
                case GL_UNSIGNED_SHORT: remoteData->indexSize = sizeof( GLushort ); break;
                case GL_UNSIGNED_INT: remoteData->indexSize = sizeof( GLuint ); break;
            }

            SG_assert( remoteData->indexSize != 0 )

            driver->gpuStats.bytesInMeshes -= remoteData->iboCapacity;

            remoteData->numIndices = count;
            remoteData->iboCapacity = count * remoteData->indexSize;

            const void* indexPointer = ( remoteData->indexFormat == GL_UNSIGNED_INT ) ? indices : nullptr;

            glApi.functions.glBufferData( GL_ELEMENT_ARRAY_BUFFER, remoteData->iboCapacity, indexPointer, getIboUsageByFlags( flags ) );

            driver->gpuStats.bytesInMeshes += remoteData->iboCapacity;

            if ( indexPointer == nullptr && indices != nullptr )
            {
                void* indexBuffer = glApi.functions.glMapBuffer( GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY );

                SG_assert( indexBuffer != nullptr )

                if ( remoteData->indexFormat == GL_UNSIGNED_BYTE )
                {
                    GLubyte* indexBuffer2 = ( GLubyte* ) indexBuffer;

                    for ( size_t i = 0; i < count; i++ )
                        indexBuffer2[i] = ( GLubyte ) indices[i];
                }
                else
                {
                    GLushort* indexBuffer2 = ( GLushort* ) indexBuffer;

                    for ( size_t i = 0; i < count; i++ )
                        indexBuffer2[i] = ( GLushort ) indices[i];
                }

                glApi.functions.glUnmapBuffer( GL_ELEMENT_ARRAY_BUFFER );
            }

            glApi.functions.glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
        }
        else
        {
            localData->indices.load( indices, count );
        }

        int error = glGetError();

        if ( error != GL_NO_ERROR )
            throw Exception( "OpenGlDriver.Mesh.loadIndices", "OpenGlError", ( String ) "OpenGL runtime error: " + error );
    }

    void Mesh::loadVertices( unsigned flags, const ConstVertices* vertices, bool fixUvs )
    {
        SG_assert( vertices->coords != nullptr )

        if ( driverShared.useVertexBuffers )
        {
            RemoteData* remoteData = this->remoteData;

            GLenum usage = getVboUsageByFlags( flags );

            glApi.functions.glGenBuffers( 1, &remoteData->vbo );
            glApi.functions.glBindBuffer( GL_ARRAY_BUFFER, remoteData->vbo );

            remoteData->vertexSize = 3 * sizeof( float );

            if ( vertices->normals != nullptr )
            {
                remoteData->vertexProperties.hasNormals = true;
                remoteData->normalOffset = remoteData->vertexSize;
                remoteData->vertexSize += 3 * sizeof( float );
            }

            for ( size_t i = 0; i < TEXTURES_PER_VERTEX && vertices->uvs[i] != nullptr; i++ )
            {
                remoteData->vertexProperties.numTextures++;
                remoteData->uvOffset[i] = remoteData->vertexSize;
                remoteData->vertexSize += 2 * sizeof( float );
            }

            if ( vertices->lightUvs != nullptr )
            {
                remoteData->vertexProperties.hasLightUvs = true;
                remoteData->lightUvOffset = remoteData->vertexSize;
                remoteData->vertexSize += 2 * sizeof( float );
            }

            driver->gpuStats.bytesInMeshes -= remoteData->vboCapacity;

            remoteData->numVertices = vertices->count;
            remoteData->vboCapacity = vertices->count * remoteData->vertexSize;

            SG_assert ( remoteData->vboCapacity != 0 )

            glApi.functions.glBufferData( GL_ARRAY_BUFFER, remoteData->vboCapacity, nullptr, usage );

            driver->gpuStats.bytesInMeshes += remoteData->vboCapacity;

            float* vertexBuffer = ( float* ) glApi.functions.glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );
            SG_assert( vertexBuffer != nullptr )

            for ( size_t i = 0; i < vertices->count; i++ )
            {
                *( vertexBuffer++ ) = vertices->coords[i * 3];
                *( vertexBuffer++ ) = vertices->coords[i * 3 + 1];
                *( vertexBuffer++ ) = vertices->coords[i * 3 + 2];

                if ( remoteData->vertexProperties.hasNormals )
                {
                    *( vertexBuffer++ ) = vertices->normals[i * 3];
                    *( vertexBuffer++ ) = vertices->normals[i * 3 + 1];
                    *( vertexBuffer++ ) = vertices->normals[i * 3 + 2];
                }

                if ( fixUvs )
                {
                    for ( size_t j = 0; j < remoteData->vertexProperties.numTextures; j++ )
                    {
                        *( vertexBuffer++ ) = vertices->uvs[j][i * 2];
                        *( vertexBuffer++ ) = 1.0f - vertices->uvs[j][i * 2 + 1];
                    }
                }
                else
                {
                    for ( size_t j = 0; j < remoteData->vertexProperties.numTextures; j++ )
                    {
                        *( vertexBuffer++ ) = vertices->uvs[j][i * 2];
                        *( vertexBuffer++ ) = vertices->uvs[j][i * 2 + 1];
                    }
                }

                if ( remoteData->vertexProperties.hasLightUvs )
                {
                    *( vertexBuffer++ ) = vertices->lightUvs[i * 2];
                    *( vertexBuffer++ ) = 1.0f - vertices->lightUvs[i * 2 + 1];
                }
            }

            glApi.functions.glUnmapBuffer( GL_ARRAY_BUFFER );
            glApi.functions.glBindBuffer( GL_ARRAY_BUFFER, 0 );
        }
        else
        {
            if ( vertices->coords != nullptr )
                localData->coords.load( vertices->coords, vertices->count * 3 );

            if ( vertices->normals != nullptr )
                localData->normals.load( vertices->normals, vertices->count * 3 );

            for ( size_t i = 0; i < TEXTURES_PER_VERTEX && vertices->uvs[i] != nullptr; i++ )
            {
                localData->uvs[i].load( vertices->uvs[i], vertices->count * 2 );

                for ( size_t j = 1; j < vertices->count * 2; j += 2 )
                    localData->uvs[i].getUnsafe( j ) = 1.0f - localData->uvs[i].getUnsafe( j );
            }

            if ( vertices->lightUvs != nullptr )
            {
                localData->lightUvs.load( vertices->lightUvs, vertices->count * 2 );

                for ( size_t j = 1; j < vertices->count * 2; j += 2 )
                    localData->lightUvs.getUnsafe( j ) = 1.0f - localData->lightUvs.getUnsafe( j );
            }
        }

        int error = glGetError();

        if ( error != GL_NO_ERROR )
            throw Exception( "OpenGlDriver.Mesh.loadVertices", "OpenGlError", ( String ) "OpenGL runtime error: " + error );
    }

    void Mesh::pick()
    {
        beginRender( true );
        doRenderAll();
        endRender( true );
    }

    void Mesh::render( const glm::mat4& localToWorld, Material* material )
    {
        if ( material == nullptr )
        {
            if ( driverShared.useVertexBuffers )
                material = remoteData->material;
            else
                material = localData->material;
        }

        SG_assert3( material != nullptr, "OpenGlDriver.Mesh.render" )

        material->apply();
        driver->renderState.currentShaderProgram->setLocalToWorld( localToWorld );

        beginRender( false );
        doRenderAll();
        endRender( false );
    }

    void Mesh::render( Material* material, const Colour& blend )
    {
        if ( material == nullptr )
        {
            if ( driverShared.useVertexBuffers )
                material = remoteData->material;
            else
                material = localData->material;
        }

        SG_assert3( material != nullptr, "OpenGlDriver.Mesh.render" )

        material->apply( blend );

        beginRender( false );
        doRenderAll();
        endRender( false );
    }

    void Mesh::render( Material* material, const Colour& blend, Texture* texture0 )
    {
        if ( material == nullptr )
        {
            if ( driverShared.useVertexBuffers )
                material = remoteData->material;
            else
                material = localData->material;
        }

        SG_assert3( material != nullptr, "OpenGlDriver.Mesh.render" )

        material->apply( blend, texture0 );

        beginRender( false );
        doRenderAll();
        endRender( false );
    }

    void Mesh::renderRange( size_t offset, size_t count, Material* material )
    {
        if ( material == nullptr )
        {
            if ( driverShared.useVertexBuffers )
                material = remoteData->material;
            else
                material = localData->material;
        }

        SG_assert( material != nullptr )

        material->apply();
        driver->renderState.currentShaderProgram->setLocalToWorld( glm::mat4() );

        beginRender( false );
        doRenderRange( offset, count );
        endRender( false );
    }

    /*bool Mesh::retrieveVertices( unsigned offset, Vertex* vertices, unsigned count )
    {
        if ( vbo != 0 )
        {
            glApi.functions.glBindBuffer( GL_ARRAY_BUFFER, vbo );

            if ( !isSimple )
            {
                SG_assert( ( offset + count ) * sizeof( Vertex ) <= vboCapacity );

                glApi.functions.glGetBufferSubData( GL_ARRAY_BUFFER, offset * sizeof( Vertex ), count * sizeof( Vertex ), vertices );
            }

            glApi.functions.glBindBuffer( GL_ARRAY_BUFFER, 0 );
        }

        return false;
    }*/

    void Mesh::setMaterial( Material* material )
    {
        if ( driverShared.useVertexBuffers )
        {
            li::release( remoteData->material );

            remoteData->material = material;
        }
        else
        {
            li::release( localData->material );

            localData->material = material;
        }
    }

    bool Mesh::updateIndices( size_t offset, const unsigned* indices, size_t count )
    {
        if ( remoteData != nullptr )
        {
            size_t indexSize = remoteData->indexSize;

            SG_assert( ( offset + count ) * indexSize <= remoteData->iboCapacity );

            glApi.functions.glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, remoteData->ibo );

            if ( remoteData->indexFormat == GL_UNSIGNED_INT )
                glApi.functions.glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, offset * indexSize, count * indexSize, indices );
            else if ( remoteData->indexFormat == GL_UNSIGNED_SHORT )
            {
                Array<GLushort> converted( count );

                for ( size_t i = 0; i < count; i++ )
                    converted.getUnsafe( i ) = indices[i];

                glApi.functions.glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, offset * indexSize, count * indexSize, converted.getPtr() );
            }
            else if ( remoteData->indexFormat == GL_UNSIGNED_BYTE )
            {
                Array<GLubyte> converted( count );

                for ( size_t i = 0; i < count; i++ )
                    converted.getUnsafe( i ) = indices[i];

                glApi.functions.glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, offset * indexSize, count * indexSize, converted.getPtr() );
            }
            else
            {
                // Not supposed to happen, right?

                SG_assert( false )
            }

            glApi.functions.glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
        }
        else
        {
            localData->indices.load( indices, count, offset );
        }

        return false;
    }

    /*bool Mesh::updateVertexCoords( unsigned offset, float* coords, unsigned count )
    {
        if ( vbo != 0 )
        {
            SG_assert( ( offset + count ) * 3 * sizeof( float ) <= vboCapacity );

            glApi.functions.glBindBuffer( GL_ARRAY_BUFFER, vbo );

            if ( isSimple )
            {
                glApi.functions.glBufferSubData( GL_ARRAY_BUFFER, offset * sizeof( float ), count * sizeof( float ), coords );
            }

            glApi.functions.glBindBuffer( GL_ARRAY_BUFFER, 0 );
        }

        return false;
    }*/

    /*bool Mesh::updateVertices( unsigned offset, const Vertex* vertices, unsigned count )
    {
        if ( vbo != 0 )
        {
            SG_assert( ( offset + count ) * sizeof( Vertex ) <= vboCapacity );

            glApi.functions.glBindBuffer( GL_ARRAY_BUFFER, vbo );

            if ( !isSimple )
            {
                //printf( "Simple updateVertexCoords: updating %u vertices from %u (byte ranges %u to %u)\n", count, offset, offset * sizeof( float ), ( offset + count ) * sizeof( float ) );
                glApi.functions.glBufferSubData( GL_ARRAY_BUFFER, offset * sizeof( Vertex ), count * sizeof( Vertex ), vertices );
            }

            glApi.functions.glBindBuffer( GL_ARRAY_BUFFER, 0 );
        }
        //else
            //glApi.functions.glBufferData( GL_ARRAY_BUFFER, numVertices * sizeof( Vertex ), nullptr, usage );
        //else
//            data->vertices

        return false;
    }*/
}
