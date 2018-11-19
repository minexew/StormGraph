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

#include <StormGraph/ResourceManager.hpp>

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

namespace OpenGlDriver
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

    ModelPreload* Ms3dLoader::load( OpenGlDriver* driver, const char* name, SeekableInputStream* input, ResourceManager* resMgr )
    {
        Reference<> inputGuard( input );

        ModelPreload* preload = new ModelPreload( driver, name );

        List<Material*> materials;
        List<int> materialIndices;

        List<ms3d_vertex_t> vertices;
        List<ms3d_triangle_t> polygons;

        unsigned numVertices, numPolygons, numMeshes, numMaterials;

        ms3d_header_t header = input->readUnsafe<ms3d_header_t>();

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

            MeshPreload* meshPreload = new MeshPreload;

            meshPreload->name = meshInfo.name;
            meshPreload->layout = MeshLayout::linear;
            meshPreload->renderMode = GL_TRIANGLES;

            for ( unsigned j = 0; j < meshInfo.numTriangles; j++ )
            {
                int polyIdx = input->read<unsigned short>();

                for ( int faceVtx = 0; faceVtx < 3; faceVtx++ )
                {
                    //* RHS -> LHS
                    meshPreload->coords.add( vertices[polygons[polyIdx].vertexIndices[faceVtx]].vertex[0] );
                    meshPreload->normals.add( polygons[polyIdx].vertexNormals[faceVtx][0] );

                    meshPreload->coords.add( vertices[polygons[polyIdx].vertexIndices[faceVtx]].vertex[2] );
                    meshPreload->normals.add( polygons[polyIdx].vertexNormals[faceVtx][2] );

                    meshPreload->coords.add( vertices[polygons[polyIdx].vertexIndices[faceVtx]].vertex[1] );
                    meshPreload->normals.add( polygons[polyIdx].vertexNormals[faceVtx][1] );

                    meshPreload->uvs.add( polygons[polyIdx].s[faceVtx] );
                    meshPreload->uvs.add( polygons[polyIdx].t[faceVtx] );
                }
            }

            preload->meshes.add( meshPreload );

            materialIndices.add( input->read<char>() );
        }

        numMaterials = input->read<unsigned short>();

        for ( unsigned i = 0; i < numMaterials; i++ )
        {
            ms3d_material_t matInfo = input->readUnsafe<ms3d_material_t>();

            //Material::Type matType = ( matInfo.texture[0] != 0 ) ? Material::textured : Material::solid;
            Material* mat = new Material( driver, matInfo.name, Material::textured );

            mat->ambient = Colour( matInfo.ambient );
            mat->diffuse = Colour( matInfo.diffuse );
            mat->specular = Colour( matInfo.specular );
            mat->emissive = Colour( matInfo.emissive );
            mat->shininess = matInfo.shininess;
            mat->colour = Colour( 1.0f, 1.0f, 1.0f, matInfo.transparency );

            if ( matInfo.texture[0] != 0 )
            {
                ITexture* texture = 0;
                ITexturePreload* texturePreload = 0;

                if ( resMgr->getTexturePreload( matInfo.texture, &texture, &texturePreload ) )
                    mat->texture = ( Texture* ) texture;
                else
                    mat->texturePreload = ( TexturePreload* ) texturePreload;
            }
            else
            {
                mat->texture = driver->getSolidTexture();
            }

            materials.add( mat );
            /*printf( "'%s': ambient(%g, %g, %g), diffuse(%g, %g, %g),\n\tspecular(%g, %g, %g), emissive(%g, %g, %g),\n\tshininess(%g), alpha(%g), mode(%i), texture('%s')\n\n",
                    matInfo.name, matInfo.ambient[0], matInfo.ambient[1], matInfo.ambient[2],
                    matInfo.diffuse[0], matInfo.diffuse[1], matInfo.diffuse[2],
                    matInfo.specular[0], matInfo.specular[1], matInfo.specular[2],
                    matInfo.emissive[0], matInfo.emissive[1], matInfo.emissive[2],
                    matInfo.shininess, matInfo.transparency, matInfo.mode, matInfo.texture );*/
        }

        iterate ( preload->meshes )
        {
            //printf( "linking %i(%s): material #%i\n", meshes.iter(), meshes.current()->name.c_str(), materialIndices[meshes.iter()] );

            int materialIndex = materialIndices[preload->meshes.iter()];

            if ( materialIndex >= 0 && ( unsigned )materialIndex < materials.getLength() )
                preload->meshes.current()->material = materials[materialIndices[preload->meshes.iter()]]->reference();
        }

        iterate ( materials )
            materials.current()->release();

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
