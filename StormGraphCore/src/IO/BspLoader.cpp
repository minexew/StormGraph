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

#include <StormGraph/IO/Bsp.hpp>
#include <StormGraph/IO/ModelLoader.hpp>
#include <StormGraph/ResourceManager.hpp>

#define Cbsp_printf( a_ )

namespace StormGraph
{
    static BspNode* readNode( InputStream* input, unsigned indexSize )
    {
        Object<BspNode> node = new BspNode;

        node->bounds[0] = input->read<Vector<float>>();
        node->bounds[1] = input->read<Vector<float>>();

        size_t numMeshes = input->read<uint32_t>();

        //Cbsp_printf(( "@@ READ: Node contains %"PRIuPTR" meshes\n", numMeshes ));

        for ( size_t i = 0; i < numMeshes; i++ )
        {
            size_t numTriangles = input->read<uint32_t>();
            unsigned material = input->read<uint32_t>();

            //printf( "@@ READ: Mesh %"PRIuPTR" contains %"PRIuPTR" indices\n", i, numTriangles * 3 );

            Object<BspMesh> mesh = new BspMesh( material );

            mesh->indices.resize( numTriangles * 3 );

            if ( indexSize == 2 )
            {
                for ( size_t j = 0; j < numTriangles * 3; j++ )
                    mesh->indices.add( input->read<uint16_t>() );
            }
            else
            {
                for ( size_t j = 0; j < numTriangles * 3; j++ )
                    mesh->indices.add( input->read<uint32_t>() );
            }

            node->meshes.add( mesh.detach() );
        }

        bool hasChildren = ( input->read<uint8_t>() > 0 );

        if ( hasChildren )
        {
            node->children[0] = readNode( input, indexSize );
            node->children[1] = readNode( input, indexSize );
        }

        return node.detach();
    }

    static BspTree* doLoad( InputStream* input, IResourceManager* resMgr )
    {
        Reference<> inputGuard( input );

        /*String header = input->readString();

        if ( header != "Sg_Bsp#0" )
            throw Exception( "StormGraph.BspLoader.doLoad", "StreamFormatError", "The input is not a valid StormGraph BSP file" );*/

        Object<BspTree> tree = new BspTree;

        unsigned indexSize = input->read<uint8_t>();

        size_t numMaterials = input->read<uint32_t>();
        tree->materials.resize( numMaterials );

        for ( size_t i = 0; i < numMaterials; i++ )
        {
            String name = input->readString();
            bool isEmbedded = input->read<uint8_t>() != 0;

            if ( isEmbedded )
            {
                Object<MaterialStaticProperties> properties = new MaterialStaticProperties;
                //printf( "[Cbsp] Material %s\n", name.c_str() );

                properties->colour = Colour::fromRgbaUint32( input->read<uint32_t>() );
                //printf( "[Cbsp] Material Colour %s\n", properties->colour.toString().c_str() );

                properties->numTextures = input->read<uint32_t>();
                //printf( "[Cbsp] Textures %u\n", properties->numTextures );

                for ( unsigned i = 0; i < properties->numTextures; i++ )
                    properties->textureNames[i] = input->readString();

                properties->dynamicLighting = input->read<uint8_t>() != 0;
                //printf( "[Cbsp] dynamicLighting %i\n", properties->dynamicLighting );

                if ( properties->dynamicLighting )
                {
                    properties->dynamicLightingResponse.ambient = Colour::fromRgbaUint32( input->read<uint32_t>() );
                    properties->dynamicLightingResponse.diffuse = Colour::fromRgbaUint32( input->read<uint32_t>() );
                    properties->dynamicLightingResponse.emissive = Colour::fromRgbaUint32( input->read<uint32_t>() );
                    properties->dynamicLightingResponse.specular = Colour::fromRgbaUint32( input->read<uint32_t>() );
                    properties->dynamicLightingResponse.shininess = input->read<float>();
                }

                properties->lightMapping = input->read<uint8_t>() != 0;
                //printf( "[Cbsp] lightMapping %i\n\n", properties->lightMapping );

                if ( properties->lightMapping )
                {
                    if ( resMgr->getLoadFlag( LoadFlag::useLightMapping ) != 0 )
                        properties->lightMapName = input->readString();
                    else
                        input->readString();
                }

                properties->castsShadows = input->read<uint8_t>() != 0;
                properties->receivesShadows = input->read<uint8_t>() != 0;

                switch ( resMgr->getLoadFlag( LoadFlag::useShadowMapping ) )
                {
                    case 0: properties->receivesShadows = false; break;
                    case 1: properties->receivesShadows = true; break;
                }

#ifdef li_GCC4
                tree->materials.add( BspMaterial { name, properties.detach() } );
#else
                BspMaterial mat = { name, properties.detach() };
                tree->materials.add( ( BspMaterial&& ) mat );
#endif
            }
            else
#ifdef li_GCC4
                tree->materials.add( BspMaterial { name, nullptr } );
#else
            {
                BspMaterial mat = { name, nullptr };
                tree->materials.add( ( BspMaterial&& ) mat );
            }
#endif

            size_t numVertices = input->read<uint32_t>();
            tree->totalTriangles[i] = input->read<uint32_t>();

            //printf( "@@ READ: MaterialGroup %"PRIuPTR" contains %"PRIuPTR" vertices\n", i, numVertices );

            tree->vertices[i].resize( numVertices );

            for ( size_t j = 0; j < numVertices; j++ )
            {
                Vertex v;

                v.pos = input->read<Vector<float>>();
                v.normal = input->read<Vector<float>>();
                v.uv[0] = input->read<Vector2<float>>();
                v.lightUv = input->read<Vector2<float>>();

                tree->vertices[i].add( v );
            }
        }

        tree->root = readNode( input, indexSize );

        return tree.detach();
    }

    IStaticModel* BspLoader::loadStaticModel( IGraphicsDriver* driver, const char* name, SeekableInputStream* input, IResourceManager* resMgr, bool finalized )
    {
        Object<BspTree> tree = doLoad( input, resMgr );

        return driver->createStaticModelFromBsp( name, tree, resMgr, finalized );
    }
}

