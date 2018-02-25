
#include "Internal.hpp"

#include <SDL_image.h>

namespace StormGraph
{
#define bufferOffset( i_ ) ( ( unsigned char* )0 + ( i_ ) )

    struct Vertex
    {
        float x, y, z;
        float nX, nY, nZ;
        float u, v;
    };

    Mesh::Mesh( const String& name, Material* material )
            : type( Mesh_raw ), name( name ), material( material ), vbo( 0 ), ibo( 0 ), numVertices( 0 ), numIndices( 0 )
    {
    }

    Mesh::Mesh( const String& name, Material* material, const List<float>& coords, const List<float>& normals, const List<float>& uvs )
            : type( Mesh_raw ), name( name ), material( material ), vbo( 0 ), ibo( 0 ), numVertices( 0 ), numIndices( 0 )
    {
        buildVBO( coords, normals, uvs );
    }

    Mesh::Mesh( const String& name, Material* material, const List<float>& coords, const List<float>& normals, const List<float>& uvs, const List<unsigned short>& indices )
            : type( Mesh_indexed ), name( name ), material( material ), vbo( 0 ), ibo( 0 ), numVertices( 0 ), numIndices( 0 )
    {
        buildVBO( coords, normals, uvs );
        buildIBO( indices );
    }

    Mesh::~Mesh()
    {
        if ( material )
            material->release();

        if ( vbo )
            glFs.glDeleteBuffers( 1, &vbo );

        if ( ibo )
            glFs.glDeleteBuffers( 1, &ibo );
    }

    void Mesh::createTerrain( HeightMap* heightMap, const Vector<float>& uvRepeat )
    {
        const Vector<unsigned>& mapSize = heightMap->getSize();
        const Vector<float>& dimensions = heightMap->getDimensions();

        Vector<float> spacing( dimensions.x / ( mapSize.x - 1 ), dimensions.y / ( mapSize.y - 1 ) );
        Vector<float> uvSpacing( uvRepeat.x / ( mapSize.x - 1 ), uvRepeat.y / ( mapSize.y - 1 ) );

        List<Vertex> vertices;
        List<unsigned> indices;

        for ( unsigned y = 0; y < mapSize.y; y++ )
        {
            for ( unsigned x = 0; x < mapSize.x; x++ )
            {
                Vertex vertex;

                vertex.x = x * spacing.x;
                vertex.y = y * spacing.y;
                vertex.z = heightMap->get( x, y );

                vertex.nX = 0.0f;
                vertex.nY = 0.0f;
                vertex.nZ = 0.0f;

                vertex.u = x * uvSpacing.x;
                vertex.v = y * uvSpacing.y;

                vertices.add( vertex );

                if ( x < mapSize.x - 1 && y < mapSize.y - 1 )
                {
                    // Polygons need to be defined counter-clockwise because of backface culling

                    indices.add( y * mapSize.x + x + 1 );
                    indices.add( y * mapSize.x + x );
                    indices.add( ( y + 1 ) * mapSize.x + x + 1 );

                    indices.add( ( y + 1 ) * mapSize.x + x + 1 );
                    indices.add( y * mapSize.x + x );
                    indices.add( ( y + 1 ) * mapSize.x + x );
                }
            }
        }

        //* Calculate normals
        Vector<float> corners[3], sides[2], normal;
        Array<unsigned> connections;
        connections.resize( vertices.getLength() );

        for ( unsigned i = 0; i < indices.getLength(); i += 3 )
        {
            //* Get the corners of this polygon
            for ( unsigned j = 0; j < 3; j++ )
                corners[j] = Vector<float>( vertices[indices[i + j]].x, vertices[indices[i + j]].y, vertices[indices[i + j]].z );

            //* Calculate vectors representing two sides of this poly
            sides[0] = corners[1] - corners[0];
            sides[1] = corners[2] - corners[0];

            //* Get the cross-product of those two vectors
            //*  and normalize it to get a direction vector.
            normal = sides[0].crossProduct( sides[1] ).normalize();

            for ( unsigned j = 0; j < 3; j++ )
            {
                //* Remember that the normals for this polygon were calculated,
                //*  accumulate this information per-vertex
                connections[indices[i + j]]++;

                //* And add the normal itself to the normal-sum
                vertices[indices[i + j]].nX += normal.x;
                vertices[indices[i + j]].nY += normal.y;
                vertices[indices[i + j]].nZ += normal.z;
            }
        }

        //* Loop through the vertices and divide every normal-sum by the calculation-count
        for each_in_list ( vertices, i )
        {
            if ( connections[i] > 0 )
            {
                vertices[i].nX /= connections[i];
                vertices[i].nY /= connections[i];
                vertices[i].nZ /= connections[i];
            }

            // In order for diffuse to work, we need all the normals to point up
            if ( vertices[i].nZ < 0 )
            {
                vertices[i].nX = -vertices[i].nX;
                vertices[i].nY = -vertices[i].nY;
                vertices[i].nZ = -vertices[i].nZ;
            }
        }

        printf( "createTerrain: Generated %u vertices, %u indices\n", vertices.getLength(), indices.getLength() );
        buildVBO( vertices );
        buildIBO( indices );

        type = Mesh_indexed;
    }

    void Mesh::buildVBO( const List<Vertex>& vertices )
    {
        glFs.glGenBuffers( 1, &vbo );
        glFs.glBindBuffer( GL_ARRAY_BUFFER, vbo );
        glFs.glBufferData( GL_ARRAY_BUFFER, vertices.getLength() * sizeof( Vertex ), *vertices, GL_STATIC_DRAW );
        glFs.glBindBuffer( GL_ARRAY_BUFFER, 0 );

        numVertices = vertices.getLength();
    }

    void Mesh::buildVBO( const List<float>& coords, const List<float>& normals, const List<float>& uvs )
    {
        List<Vertex> vertices;

        for ( unsigned i = 0; i < coords.getLength() / 3 && i < normals.getLength() / 3 && i < uvs.getLength() / 2; i++ )
        {
            Vertex vertex = { coords[i * 3], coords[i * 3 + 1], coords[i * 3 + 2], normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2], uvs[i * 2], uvs[i * 2 + 1] };
            vertices.add( vertex );
        }

        buildVBO( vertices );
    }

    void Mesh::buildIBO( const List<unsigned short>& indices )
    {
        indexLength = sizeof( unsigned short );

        glFs.glGenBuffers( 1, &ibo );
        glFs.glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
        glFs.glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.getLength() * indexLength, *indices, GL_STATIC_DRAW );
        glFs.glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

        numIndices = indices.getLength();
        indexFormat = GL_UNSIGNED_SHORT;
    }

    void Mesh::buildIBO( const List<unsigned>& indices )
    {
        indexLength = sizeof( unsigned );

        glFs.glGenBuffers( 1, &ibo );
        glFs.glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
        glFs.glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.getLength() * indexLength, *indices, GL_STATIC_DRAW );
        glFs.glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

        numIndices = indices.getLength();
        indexFormat = GL_UNSIGNED_INT;
    }

    void Mesh::doRender( bool simple )
    {
        glFs.glBindBuffer( GL_ARRAY_BUFFER, vbo );

        glEnableClientState( GL_VERTEX_ARRAY );
        glVertexPointer( 3, GL_FLOAT, sizeof( Vertex ), bufferOffset( 0 ) );

        if ( !simple )
        {
            glEnableClientState( GL_NORMAL_ARRAY );
            glNormalPointer( GL_FLOAT, sizeof( Vertex ), bufferOffset( 12 ) );

            glEnableClientState( GL_TEXTURE_COORD_ARRAY );
            glTexCoordPointer( 2, GL_FLOAT, sizeof( Vertex ), bufferOffset( 24 ) );
        }

        if ( type == Mesh_indexed )
        {
            glFs.glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
            glDrawElements( GL_TRIANGLES, numIndices, indexFormat, bufferOffset( 0 ) );
            glFs.glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

            numPolysThisFrame += numIndices / 3;
        }
        else
        {
            glDrawArrays( GL_TRIANGLES, 0, numVertices );

            numPolysThisFrame += numVertices / 3;
        }

        glDisableClientState( GL_NORMAL_ARRAY );

        if ( !simple )
        {
            glDisableClientState( GL_VERTEX_ARRAY );
            glDisableClientState( GL_TEXTURE_COORD_ARRAY );
        }

        glFs.glBindBuffer( GL_ARRAY_BUFFER, 0 );
    }

    void Mesh::pickingRender( Picking* picking, unsigned id )
    {
        Engine::getInstance()->setPickingId( id );

        doRender( true );
    }

    void Mesh::render()
    {
        if ( !material )
        {
            printf( "[StormGraph::Mesh::render] Warning :\n\ttrying to render mesh '%s' without any material assigned!\n", name.c_str() );
            return;
        }

        material->apply();
        doRender( false );
    }

    void Mesh::renderTerrain( const Vector<float>& location, HeightMap* heightMap )
    {
        if ( !material )
        {
            printf( "[StormGraph::Mesh::render] Warning :\n\ttrying to render mesh '%s' without any material assigned!\n", name.c_str() );
            return;
        }

        glPushMatrix();
        glTranslatef( location.x, location.y, location.z );

        material->apply();

        glFs.glBindBuffer( GL_ARRAY_BUFFER, vbo );
        glFs.glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );

        glEnableClientState( GL_VERTEX_ARRAY );
        glEnableClientState( GL_NORMAL_ARRAY );
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );

        glVertexPointer( 3, GL_FLOAT, sizeof( Vertex ), bufferOffset( 0 ) );
        glNormalPointer( GL_FLOAT, sizeof( Vertex ), bufferOffset( 12 ) );
        glTexCoordPointer( 2, GL_FLOAT, sizeof( Vertex ), bufferOffset( 24 ) );

        const Vector<unsigned>& mapSize = heightMap->getSize();
        const Vector<float>& dimensions = heightMap->getDimensions();

        Vector<float> spacing( dimensions.x / ( mapSize.x - 1 ), dimensions.y / ( mapSize.y - 1 ) );

        // we need some margin for steep slopes
        float maxSpacing = maximum( spacing.x, spacing.y ) * 2.5f;

        Vector<float> worldLoc;
        worldLoc.y = location.y + spacing.y / 2.0f;

        unsigned polygonOffset = 0;

        for ( unsigned y = 0; y < mapSize.y - 1; y++ )
        {
            worldLoc.x = location.x + spacing.x / 2.0f;

            for ( unsigned x = 0; x < mapSize.x - 1; x++ )
            {
                worldLoc.z = location.z + heightMap->get( x, y );

                if ( Engine::getInstance()->isSphereVisible( worldLoc, maxSpacing ) )
                {
                    glDrawElements( GL_TRIANGLES, 6, indexFormat, bufferOffset( polygonOffset ) );
                    numPolysThisFrame += 2;
                }

                worldLoc.x += spacing.x;
                polygonOffset += 6 * indexLength;
            }

            worldLoc.y += spacing.y;
        }

        glDisableClientState( GL_NORMAL_ARRAY );
        glDisableClientState( GL_VERTEX_ARRAY );
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );

        glFs.glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
        glFs.glBindBuffer( GL_ARRAY_BUFFER, 0 );

        glPopMatrix();
    }

    Model::Model()
    {
    }

    Model::Model( const char* fileName, ResourceManager* resourceMgr )
    {
        InputStream* input = 0;

        if ( fileName )
            input = open( fileName );

        if ( input )
            loadMs3d( meshes, input, resourceMgr );
        else
            throw Exception( "StormGraph.Model", "Model", "ModelLoadError",
                    ( String )"Failed to load `" + fileName + "`. The file probably either doesn't exist at all or is not a well-formed scene." );
    }

    Model::Model( InputStream* input, ResourceManager* resourceMgr )
    {
        loadMs3d( meshes, input, resourceMgr );
    }

    Model::~Model()
    {
        iterate ( meshes )
            meshes.current()->release();
    }

    Model* Model::tryLoad( const char* fileName, ResourceManager* resourceMgr )
    {
        InputStream* input = 0;

        if ( fileName )
            input = open( fileName );

        if ( input )
            return new Model( input, resourceMgr );
        else
            return 0;
    }

    Model* Model::createTerrain( HeightMap* heightMap, const Vector<float>& uvRepeat, Material* material )
    {
        Model* model = new Model();

        Mesh* mesh = new Mesh( "terrain", material );
        mesh->createTerrain( heightMap, uvRepeat );

        model->meshes.add( mesh );
        return model;
    }

    InputStream* Model::open( const char* uri )
    {
        return Engine::getInstance()->open( uri );
    }

    void Model::renderBegin()
    {
        glPushMatrix();
    }

    void Model::renderEnd()
    {
        glPopMatrix();
    }

    unsigned Model::pickingRender( Picking* picking )
    {
        unsigned id = picking->generateId();

        iterate ( meshes )
            meshes.current()->pickingRender( picking, id );

        return id;
    }

    void Model::render()
    {
        iterate ( meshes )
            meshes.current()->render();
    }

    void Model::renderTerrain( const Vector<float>& location, HeightMap* heightMap )
    {
        iterate ( meshes )
            meshes.current()->renderTerrain( location, heightMap );
    }

    void Model::rotate( float angle, const Vector<float>& vec )
    {
        glRotated( -angle * 180.0 / M_PI, vec.x, vec.y, vec.z );
    }

    void Model::scale( const Vector<float>& vec )
    {
        glScalef( vec.x, vec.y, vec.z );
    }

    void Model::translate( const Vector<float>& vec )
    {
        glTranslatef( vec.x, vec.y, vec.z );
    }
}
