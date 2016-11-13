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
    static MeshPreload* createMeshPreload( MeshCreationInfo2* creationInfo )
    {
        MeshPreload* mesh = new MeshPreload;
        mesh->name = creationInfo->name;
        mesh->renderMode = Mesh::getRenderMode( creationInfo->format );
        mesh->layout = creationInfo->layout;

        for ( size_t j = 0; j < creationInfo->numVertices; j++ )
        {
            const Vertex& v = creationInfo->vertices[j];

            mesh->coords.add( v.pos.x );
            mesh->coords.add( v.pos.y );
            mesh->coords.add( v.pos.z );

            mesh->normals.add( v.normal.x );
            mesh->normals.add( v.normal.y );
            mesh->normals.add( v.normal.z );

            mesh->uvs[0].add( v.uv[0].x );
            mesh->uvs[0].add( v.uv[0].y );
        }

        for ( size_t i = 0; i < creationInfo->numIndices; i++ )
            mesh->indices.add( creationInfo->indices[i] );

        mesh->material = static_cast<Material*>( creationInfo->material );
        return mesh;
    }

    ModelPreload::ModelPreload( OpenGlDriver* driver, const char* name )
            : driver( driver ), name( name )
    {
        Resource::add( this );
    }

    ModelPreload::~ModelPreload()
    {
        iterate ( meshes )
            delete meshes.current();

        Resource::remove( this );
    }

    ModelPreload* ModelPreload::createFromMemory( OpenGlDriver* driver, const char* name, MeshCreationInfo2* meshes, size_t count, unsigned flags )
    {
        ModelPreload* preload = new ModelPreload( driver, name );

        for ( size_t i = 0; i < count; i++ )
            preload->meshes.add( createMeshPreload( meshes + i ) );

        return preload;
    }

    ModelPreload* ModelPreload::createFromMemory( OpenGlDriver* driver, const char* name, MeshCreationInfo2** meshes, size_t count, unsigned flags )
    {
        ModelPreload* preload = new ModelPreload( driver, name );

        for ( size_t i = 0; i < count; i++ )
            preload->meshes.add( createMeshPreload( meshes[i] ) );

        return preload;
    }

    IModel* ModelPreload::getFinalized()
    {
        if ( finalized == nullptr )
            finalized = new Model( driver, this, true );

        return finalized->reference();
    }

    Model::Model( OpenGlDriver* driver, ModelPreload* preload, bool finalized )
            : driver( driver ), name( preload->getName() )
    {
        iterate ( preload->meshes )
            meshes.add( new Mesh( driver, preload->meshes.current(), finalized ) );

        preload->meshes.clear();

        Resource::add( this );
    }

    /*Model::Model( OpenGlDriver* driver, const char* name, const List<MeshCreationInfo*>& meshes, unsigned flags )
            : IModel( name ), driver( driver )
    {
        for each_in_list ( meshes, i )
            this->meshes.add( new Mesh( meshes[i], flags ) );
    }*/

    Model::Model( OpenGlDriver* driver, const char* name, MeshCreationInfo3* meshes, size_t count, unsigned flags, bool finalized )
            : driver( driver ), name( name )
    {
        for ( size_t i = 0; i < count; i++ )
            this->meshes.add( new Mesh( driver, meshes + i, flags, finalized ) );

        Resource::add( this );
    }

    Model::Model( OpenGlDriver* driver, const char* name, MeshCreationInfo3** meshes, size_t count, unsigned flags, bool finalized )
            : driver( driver ), name( name )
    {
        for ( size_t i = 0; i < count; i++ )
            this->meshes.add( new Mesh( driver, meshes[i], flags, finalized ) );

        Resource::add( this );
    }

    /*Model::Model( OpenGlDriver* driver, const char* name, List<Mesh*>& meshes )
            : IModel( name ), driver( driver ), meshes( meshes )
    {
    }*/

    Model::Model( OpenGlDriver* driver, const char* name, Mesh** meshes, size_t count, bool finalized )
            : driver( driver ), name( name )
    {
        if ( finalized )
        {
            for ( size_t i = 0; i < count; i++ )
                this->meshes.add( meshes[i]->finalize() );
        }
        else
        {
            for ( size_t i = 0; i < count; i++ )
                this->meshes.add( meshes[i] );
        }

        Resource::add( this );
    }

    Model::~Model()
    {
        iterate ( meshes )
            delete meshes.current();

        Resource::remove( this );
    }

    Model* Model::finalize()
    {
        iterate2 ( mesh, meshes )
            mesh = mesh->finalize();

        return this;
    }

    size_t Model::getMeshCount()
    {
        return meshes.getLength();
    }

    size_t Model::getMeshVertexCount( size_t mesh )
    {
        SG_assert( mesh < meshes.getLength() )

        return meshes[mesh]->getNumVertices();
    }

    unsigned Model::pick( const List<Transform>& transforms )
    {
        return pick( transforms.getPtrUnsafe(), transforms.getLength() );
    }

    unsigned Model::pick( const Transform* transforms, size_t numTransforms )
    {
        unsigned id = driver->getPickingId();

        if ( driverShared.useShaders )
        {
            ShaderProgram* shader = driver->getPickingShaderProgram();

            shader->setBlendColour( driver->getPickingColour( id ) );
        }
        else
            glColor3bv( ( const GLbyte* ) &id );

        glPushMatrix();

        OpenGlDriver::applyTransforms( transforms, numTransforms );

        iterate ( meshes )
            meshes.current()->pick();

        glPopMatrix();

        return id;
    }

    void Model::render( const List<Transform>& transforms )
    {
        glPushMatrix();

        driver->applyTransforms( transforms );

        iterate ( meshes )
            meshes.current()->render( glm::mat4() );

        glPopMatrix();
    }

    void Model::render( const List<Transform>** transforms, size_t count )
    {
        glPushMatrix();

        for ( int i = count - 1; i >= 0; i-- )
            driver->applyTransforms( *transforms[i] );

        iterate ( meshes )
            meshes.current()->render( glm::mat4() );

        glPopMatrix();
    }

    void Model::render( const Transform* transforms, size_t numTransforms, bool inWorldSpace )
    {
        glPushMatrix();

        glm::mat4 localToWorld = OpenGlDriver::applyTransforms( transforms, numTransforms );

        iterate ( meshes )
            meshes.current()->render( localToWorld );

        glPopMatrix();
    }

    void Model::render( const Transform* transforms, size_t numTransforms, const Colour& blend )
    {
        glPushMatrix();

        OpenGlDriver::applyTransforms( transforms, numTransforms );

        iterate ( meshes )
            meshes.current()->render( nullptr, blend );

        glPopMatrix();
    }

    /*bool Model::retrieveVertices( size_t mesh, size_t offset, Vertex* vertices, size_t count )
    {
        SG_assert( mesh < meshes.getLength() )

        return meshes[mesh]->retrieveVertices( offset, vertices, count );
    }*/

/*    bool Model::updateVertexCoords( unsigned mesh, unsigned vertex, float* coords, unsigned count )
    {
        SG_assert3( mesh < meshes.getLength(), "OpenGlDriver.Model.updateVertexCoords" )

        return meshes[mesh]->updateVertexCoords( vertex, coords, count );
    }*/

   /* bool Model::updateVertices( size_t mesh, size_t offset, const Vertex* vertices, size_t count )
    {
        SG_assert( mesh < meshes.getLength() )

        return meshes[mesh]->updateVertices( offset, vertices, count );
    }*/
}
