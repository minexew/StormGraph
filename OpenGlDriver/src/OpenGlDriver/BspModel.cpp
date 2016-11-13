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

#include <StormGraph/IO/Bsp.hpp>
#include <StormGraph/ResourceManager.hpp>

namespace OpenGlDriver
{
    static const bool wireframe = false;

    BspModel::BspModel( OpenGlDriver* driver, const char* name, BspTree* bsp, IResourceManager* resMgr, bool finalized )
            : driver( driver ), name( name )
    {
        printf( "#### Initializing BspModel with %u materials\n", unsigned( bsp->materials.getLength() ) );

        for ( size_t i = 0; i < bsp->materials.getLength(); i++ )
        {
            MaterialProperties2 properties;
            Reference<Material> material;

            printf( "#### Material: %s\n", bsp->materials[i].name.c_str() );

            if ( bsp->materials[i].properties != nullptr )
            {
                resMgr->initializeMaterial( bsp->materials[i].properties, &properties, finalized );

                material = static_cast<Material*>( driver->createMaterial( bsp->materials[i].name, &properties, finalized ) );
            }
            else
            {
                material = static_cast<Material*>( resMgr->getMaterial( bsp->materials[i].name, finalized ) );

                material->query( Material::getNumTextures, &properties );
            }

            VertexProperties vertexProperties;
            memset( &vertexProperties, 0, sizeof( vertexProperties ) );

            vertexProperties.hasNormals = true;
            vertexProperties.numTextures = properties.numTextures;
            vertexProperties.hasLightUvs = properties.lightMapping;

            List<float> coords, normals, uvs[TEXTURES_PER_VERTEX], lightUvs;

            printf( "####     %u vertices\n", unsigned( bsp->vertices[i].getLength() ) );

            for ( size_t j = 0; j < bsp->vertices[i].getLength(); j++ )
            {
                const Vertex& vertex = bsp->vertices[i][j];

                coords.add( vertex.pos.x );
                coords.add( vertex.pos.y );
                coords.add( vertex.pos.z );

                if ( vertexProperties.hasNormals )
                {
                    normals.add( vertex.normal.x );
                    normals.add( vertex.normal.y );
                    normals.add( vertex.normal.z );
                }

                for ( unsigned k = 0; k < vertexProperties.numTextures; k++ )
                {
                    uvs[k].add( vertex.uv[k].x );
                    uvs[k].add( vertex.uv[k].y );
                }

                if ( vertexProperties.hasLightUvs )
                {
                    lightUvs.add( vertex.lightUv.x );
                    lightUvs.add( vertex.lightUv.y );
                }
            }

            MeshCreationInfo3 creationInfo;
            memset( &creationInfo, 0, sizeof( creationInfo ) );

            creationInfo.format = ( !wireframe ) ? MeshFormat::triangleList : MeshFormat::lineList;
            creationInfo.layout = MeshLayout::indexed;
            creationInfo.material = material.detach();

            creationInfo.numVertices = bsp->vertices[i].getLength();
            creationInfo.numIndices = bsp->totalTriangles[i] * ( !wireframe ? 3 : 6 );

            creationInfo.coords = coords.getPtr();

            if ( vertexProperties.hasNormals )
                creationInfo.normals = normals.getPtr();

            for ( unsigned k = 0; k < vertexProperties.numTextures; k++ )
                creationInfo.uvs[k] = uvs[k].getPtr();

            if ( vertexProperties.hasLightUvs )
                creationInfo.lightUvs = lightUvs.getPtr();

            materialGroups.add( new Mesh( driver, &creationInfo, IModel::fullStatic, finalized ) );
        }

        root = create( bsp->root );

        printf( "### BspModel creation complete\n" );
        Resource::add( this );
    }

    BspModel::~BspModel()
    {
        iterate2 ( i, materialGroups )
            delete i;

        Resource::remove( this );
    }

    BspModel::BspRenderNode* BspModel::create( BspNode* node )
    {
        if ( node == nullptr )
            return nullptr;

        Object<BspRenderNode> renderNode = new BspRenderNode;

        renderNode->bounds[0] = node->bounds[0];
        renderNode->bounds[1] = node->bounds[1];

        printf( "#####       Creating BspRenderNode: %u meshes [%s to %s]\n", unsigned( node->meshes.getLength() ),
                renderNode->bounds[0].toString().c_str(), renderNode->bounds[1].toString().c_str() );

        renderNode->meshes.resize( node->meshes.getLength() );

        iterate ( node->meshes )
        {
            unsigned materialIndex = node->meshes.current()->material;

            SG_assert( materialIndex < materialGroups.getLength() )

            Object<BspRenderMesh> mesh = new BspRenderMesh;
            mesh->material = materialIndex;

            if ( !wireframe )
            {
                mesh->indexOffset = currentOffset[materialIndex];
                mesh->indexCount = node->meshes.current()->indices.getLength();

                materialGroups[materialIndex]->updateIndices( mesh->indexOffset, node->meshes.current()->indices.getPtr(), mesh->indexCount );
            }
            else
            {
                size_t numIndices = node->meshes.current()->indices.getLength();

                Array<unsigned> indices( numIndices * 2 );

                for ( size_t i = 0; i < node->meshes.current()->indices.getLength() / 3; i++ )
                {
                    indices[i * 6] = node->meshes.current()->indices[i * 3];
                    indices[i * 6 + 1] = node->meshes.current()->indices[i * 3 + 1];
                    indices[i * 6 + 2] = node->meshes.current()->indices[i * 3 + 1];
                    indices[i * 6 + 3] = node->meshes.current()->indices[i * 3 + 2];
                    indices[i * 6 + 4] = node->meshes.current()->indices[i * 3 + 2];
                    indices[i * 6 + 5] = node->meshes.current()->indices[i * 3];
                }

                mesh->indexOffset = currentOffset[materialIndex];
                mesh->indexCount = numIndices * 2;

                materialGroups[materialIndex]->updateIndices( mesh->indexOffset, indices.getPtr(), mesh->indexCount );
            }

            currentOffset[materialIndex] += mesh->indexCount;

            renderNode->meshes.add( mesh.detach() );
        }

        renderNode->children[0] = create( node->children[0] );
        renderNode->children[1] = create( node->children[1] );

        return renderNode.detach();
    }

    BspModel* BspModel::finalize()
    {
        iterate2 ( i, materialGroups )
            i = i->finalize();

        return this;
    }

    size_t BspModel::getMeshCount()
    {
        return 0;
    }

    size_t BspModel::getMeshVertexCount( size_t mesh )
    {
        return 0;
    }

    /*unsigned BspModel::pick( const List<Transform>& transforms )
    {
        return pick( transforms.getPtrUnsafe(), transforms.getLength() );
    }

    unsigned BspModel::pick( const Transform* transforms, size_t numTransforms )
    {
        return 0;
    }

    void BspModel::render( const List<Transform>& transforms )
    {
        //render( transforms.getPtrUnsafe(), transforms.getLength() );
    }

    void BspModel::render( const List<Transform>** transforms, size_t count )
    {
        glPushMatrix();

        for ( int i = count - 1; i >= 0; i-- )
            driver->applyTransforms( *transforms[i] );

        renderNode( root );

        glPopMatrix();
    }

    void BspModel::render( const Transform* transforms, size_t numTransforms, bool inWorldSpace )
    {
        glPushMatrix();

        OpenGlDriver::applyTransforms( transforms, numTransforms );

        renderNode( root );

        glPopMatrix();
    }

    void BspModel::render( const Transform* transforms, size_t numTransforms, const Colour& blend )
    {
        render( transforms, numTransforms, true );
    }*/

    void BspModel::render()
    {
        renderNode( root );
    }

    void BspModel::renderNode( BspRenderNode* node )
    {
        //if ( driver->frustum.boxInFrustum( node->bounds[0], node->bounds[1] ) != ViewFrustum::outside )
        {
            if ( node->children[0] != nullptr && node->children[1] != nullptr )
            {
                renderNode( node->children[0] );
                renderNode( node->children[1] );
            }
            else// if ( driver->frustum.boxInFrustum( node->bounds[0], node->bounds[1] ) == ViewFrustum::inside )
                iterate ( node->meshes )
                {
                    unsigned materialIndex = node->meshes.current()->material;

                    materialGroups[materialIndex]->renderRange( node->meshes.current()->indexOffset, node->meshes.current()->indexCount );
                }
        }
    }

    /*bool BspModel::retrieveVertices( size_t mesh, size_t offset, Vertex* vertices, size_t count )
    {
        return false;
    }*/

    /*bool BspModel::updateVertexCoords( unsigned mesh, unsigned offset, float* coords, unsigned count )
    {
        return false;
    }*/

    /*bool BspModel::updateVertices( size_t mesh, size_t offset, const Vertex* vertices, size_t count )
    {
        return false;
    }*/
}
