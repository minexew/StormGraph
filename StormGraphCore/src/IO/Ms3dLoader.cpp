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

#include <StormGraph/ResourceManager.hpp>
#include <StormGraph/IO/ModelLoader.hpp>

#define MAX_VERTICES    8192
#define MAX_TRIANGLES   16384
#define MAX_GROUPS      128
#define MAX_MATERIALS   128
#define MAX_JOINTS      128
#define MAX_KEYFRAMES   216

#define SELECTED        1
#define HIDDEN          2
#define SELECTED2       4
#define DIRTY           8

#pragma pack(1)

namespace StormGraph
{
    struct ms3d_header_t
    {
        char id[10];
        int version;
    }
#ifdef __GNUC__
    __attribute(( packed ))
#endif
    ;

    struct ms3d_vertex_t
    {
        unsigned char flags;
        float vertex[3];
        char boneId;
        unsigned char referenceCount;
    }
#ifdef __GNUC__
    __attribute(( packed ))
#endif
    ;

    struct ms3d_triangle_t
    {
        unsigned short flags;
        unsigned short vertexIndices[3];
        float vertexNormals[3][3];
        float s[3];
        float t[3];
        unsigned char smoothingGroup;
        unsigned char groupIndex;
    }
#ifdef __GNUC__
    __attribute(( packed ))
#endif
    ;

    struct ms3d_group_prologue_t
    {
        unsigned char flags;
        char name[32];
        unsigned short numTriangles;
    }
#ifdef __GNUC__
    __attribute(( packed ))
#endif
    ;

    struct ms3d_material_t
    {
        char name[32];
        float ambient[4];
        float diffuse[4];
        float specular[4];
        float emissive[4];
        float shininess;
        float transparency;
        char mode;
        char texture[128];
        char alphamap[128];
    }
#ifdef __GNUC__
    __attribute(( packed ))
#endif
    ;

    struct Mesh
    {
        String name;
        List<Vertex> vertices;
    };

    class Ms3dLoaderImpl
    {
        List<Mesh*> meshes;

        public:
            ~Ms3dLoaderImpl()
            {
                iterate2 ( mesh, meshes )
                    delete mesh;
            }

            void doLoad( IGraphicsDriver* driver, const char* name, SeekableInputStream* input, IResourceManager* resMgr, List<MeshCreationInfo2*>& meshCreationInfos );
    };

    void Ms3dLoaderImpl::doLoad( IGraphicsDriver* driver, const char* name, SeekableInputStream* input, IResourceManager* resMgr, List<MeshCreationInfo2*>& meshCreationInfos )
    {
        static const bool finalized = true;

        Reference<> inputGuard( input );

        List<IMaterial*> materials;
        List<int> materialIndices;

        ms3d_header_t header;
        List<ms3d_vertex_t> vertices;
        List<ms3d_triangle_t> polygons;

        unsigned numVertices, numPolygons, numMeshes, numMaterials;

        SG_assert( input->readItems( &header, 1 ) == 1 )

        if ( header.version != 4 )
            throw StormGraph::Exception( "OpenGlDriver.Ms3dLoader.load", "InvalidFormat", ( String )"expected MS3D version 4 scene (got version " + header.version + ")" );

        numVertices = input->read<unsigned short>();

        while ( numVertices-- )
            vertices.add( input->readUnsafe<ms3d_vertex_t>() );

        numPolygons = input->read<unsigned short>();

        while ( numPolygons-- )
            polygons.add( input->readUnsafe<ms3d_triangle_t>() );

        numMeshes = input->read<unsigned short>();

        for ( unsigned i = 0; i < numMeshes; i++ )
        {
            ms3d_group_prologue_t meshInfo = input->readUnsafe<ms3d_group_prologue_t>();

            Object<Mesh> mesh = new Mesh;

            mesh->name = meshInfo.name;

            for ( unsigned j = 0; j < meshInfo.numTriangles; j++ )
            {
                int polyIdx = input->read<unsigned short>();

                for ( int faceVtx = 0; faceVtx < 3; faceVtx++ )
                {
                    Vertex v;

                    v.pos.x = vertices[polygons[polyIdx].vertexIndices[faceVtx]].vertex[0];
                    v.pos.y = vertices[polygons[polyIdx].vertexIndices[faceVtx]].vertex[2];
                    v.pos.z = vertices[polygons[polyIdx].vertexIndices[faceVtx]].vertex[1];

                    v.normal.x = polygons[polyIdx].vertexNormals[faceVtx][0];
                    v.normal.y = polygons[polyIdx].vertexNormals[faceVtx][2];
                    v.normal.z = polygons[polyIdx].vertexNormals[faceVtx][1];

                    v.uv[0].x = polygons[polyIdx].s[faceVtx];
                    v.uv[0].y = polygons[polyIdx].t[faceVtx];

                    mesh->vertices.add( v );
                }
            }

            //preload->meshes.add( meshPreload );

            meshes.add( mesh.detach() );
            materialIndices.add( input->read<char>() );
        }

        numMaterials = input->read<unsigned short>();

        for ( unsigned i = 0; i < numMaterials; i++ )
        {
            ms3d_material_t matInfo = input->readUnsafe<ms3d_material_t>();

            MaterialProperties2 properties;
            memset( &properties, 0, sizeof( properties ) );

            properties.colour = Colour( 1.0f, 1.0f, 1.0f, matInfo.transparency );

            if ( matInfo.texture[0] != 0 )
            {
                resMgr->getTexturePreload( matInfo.texture, &properties.textures[0], &properties.texturePreloads[0] );
                properties.numTextures++;
            }

            if ( resMgr->getLoadFlag( LoadFlag::useDynamicLighting ) != 0 )
            {
                properties.dynamicLighting = true;
                properties.dynamicLightingResponse.ambient = Colour( matInfo.ambient );
                properties.dynamicLightingResponse.diffuse = Colour( matInfo.diffuse );
                properties.dynamicLightingResponse.specular = Colour( matInfo.specular );
                properties.dynamicLightingResponse.emissive = Colour( matInfo.emissive );
                properties.dynamicLightingResponse.shininess = matInfo.shininess;
            }

            if ( resMgr->getLoadFlag( LoadFlag::useShadowMapping ) == 1 )
                properties.receivesShadows = true;

            materials.add( driver->createMaterial( matInfo.name, &properties, finalized ) );
        }

        iterate ( meshes )
        {
            int materialIndex = materialIndices[meshes.iter()];
            IMaterial* material = nullptr;

            if ( materialIndex >= 0 && ( unsigned )materialIndex < materials.getLength() )
                material = materials[materialIndices[meshes.iter()]]->reference();

            MeshCreationInfo2* creationInfo = new MeshCreationInfo2;

            creationInfo->name = meshes.current()->name;
            creationInfo->format = MeshFormat::triangleList;
            creationInfo->layout = MeshLayout::linear;
            creationInfo->material = material;
            creationInfo->numVertices = meshes.current()->vertices.getLength();
            creationInfo->numIndices = 0;
            creationInfo->vertices = meshes.current()->vertices.getPtr();
            creationInfo->indices = nullptr;

            meshCreationInfos.add( creationInfo );
        }

        iterate ( materials )
            materials.current()->release();
    }

    IModel* Ms3dLoader::load( IGraphicsDriver* driver, const char* name, SeekableInputStream* input, IResourceManager* resMgr )
    {
        // TODO: test header

        List<MeshCreationInfo2*> meshCreationInfos;

        Ms3dLoaderImpl loader;
        loader.doLoad( driver, name, input, resMgr, meshCreationInfos );

        IModel* model = nullptr;

        try
        {
            model = driver->createModelFromMemory( name, meshCreationInfos.getPtr(), meshCreationInfos.getLength() );
        }
        catch ( Exception& ex )
        {
            iterate ( meshCreationInfos )
                delete meshCreationInfos.current();

            throw;
        }

        iterate ( meshCreationInfos )
            delete meshCreationInfos.current();

        return model;
    }

    IModelPreload* Ms3dLoader::preload( IGraphicsDriver* driver, const char* name, SeekableInputStream* input, IResourceManager* resMgr )
    {
        List<MeshCreationInfo2*> meshCreationInfos;

        Ms3dLoaderImpl loader;
        loader.doLoad( driver, name, input, resMgr, meshCreationInfos );

        IModelPreload* preload = nullptr;

        try
        {
            preload = driver->preloadModelFromMemory( name, meshCreationInfos.getPtr(), meshCreationInfos.getLength() );
        }
        catch ( Exception& ex )
        {
            iterate ( meshCreationInfos )
                delete meshCreationInfos.current();

            throw;
        }

        iterate ( meshCreationInfos )
            delete meshCreationInfos.current();

        return preload;
    }
}

/*
//
// save some keyframer data
//
float fAnimationFPS;
float fCurrentTime;
int iTotalFrames;

//
// number of joints
//
word nNumJoints;

//
// nNumJoints * sizeof (ms3d_joint_t)
//
typedef struct {
    float           time;                               // time in seconds
    float           rotation[3];                        // x, y, z angles
} ms3d_keyframe_rot_t;

typedef struct {
    float           time;                               // time in seconds
    float           position[3];                        // local position
} ms3d_keyframe_pos_t;

typedef struct {
    byte            flags;                              // SELECTED | DIRTY
    char            name[32];                           //
    char            parentName[32];                     //
    float           rotation[3];                        // local reference matrix
    float           position[3];

    word            numKeyFramesRot;                    //
    word            numKeyFramesTrans;                  //

    ms3d_keyframe_rot_t keyFramesRot[numKeyFramesRot];      // local animation matrices
    ms3d_keyframe_pos_t keyFramesTrans[numKeyFramesTrans];  // local animation matrices
} ms3d_joint_t;

//
// Mesh Transformation:
//
// 0. Build the transformation matrices from the rotation and position
// 1. Multiply the vertices by the inverse of local reference matrix (lmatrix0)
// 2. then translate the result by (lmatrix0 * keyFramesTrans)
// 3. then multiply the result by (lmatrix0 * keyFramesRot)
//
// For normals skip step 2.
//
// NOTE:  this file format may change in future versions!
//
// - Mete Ciragan
//
*/

