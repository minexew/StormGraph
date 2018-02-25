
#include <StormGraph/StormGraph.hpp>

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

    class ParserMs3d
    {
        List<Mesh*>& meshes;
        InputStream* input;
        ResourceManager* resMgr;

        public:
            ParserMs3d( List<Mesh*>& meshes, InputStream* input, ResourceManager* resMgr );
            ~ParserMs3d();

            void parseInput();
    };

    ParserMs3d::ParserMs3d( List<Mesh*>& meshes, InputStream* input, ResourceManager* resMgr )
            : meshes( meshes ), input( input ), resMgr( resMgr )
    {
        if ( !input || !input->isReadable() )
            throw Exception( "StormGraph.ParserMs3d", "ParserMs3d", "InvalidInput", "none or unreadable input stream (possibly a file is missing?)" );
    }

    ParserMs3d::~ParserMs3d()
    {
        input->release();
    }

    void ParserMs3d::parseInput()
    {
        List<Material*> materials;
        List<int> materialIndices;

        List<ms3d_vertex_t> vertices;
        List<ms3d_triangle_t> polygons;

        int numVertices, numPolygons, numMeshes, numMaterials/*, linkMeshBegin = -1*/;

        ms3d_header_t header = input->readUnsafe<ms3d_header_t>();

        if ( header.version != 4 )
            throw Exception( "StormGraph.ParserMs3d", "parseInput", "InvalidFormat", ( String )"expected MS3D version 4 scene (got version " + header.version + ")" );

        numVertices = input->read<unsigned short>();

        while ( numVertices-- )
            vertices.add( input->readUnsafe<ms3d_vertex_t>() );

        numPolygons = input->read<unsigned short>();

        while ( numPolygons-- )
            polygons.add( input->readUnsafe<ms3d_triangle_t>() );

        numMeshes = input->read<unsigned short>();

        while ( numMeshes-- )
        {
            ms3d_group_prologue_t meshInfo = input->readUnsafe<ms3d_group_prologue_t>();
            
            List<float> coords, normals, uvs;

            while ( meshInfo.numTriangles-- )
            {
                int polyIdx = input->read<unsigned short>();

                for ( int faceVtx = 0; faceVtx < 3; faceVtx++ )
                {
                    //* RHS -> LHS
                    coords.add( vertices[polygons[polyIdx].vertexIndices[faceVtx]].vertex[0] );
                    normals.add( polygons[polyIdx].vertexNormals[faceVtx][0] );

                    coords.add( vertices[polygons[polyIdx].vertexIndices[faceVtx]].vertex[2] );
                    normals.add( polygons[polyIdx].vertexNormals[faceVtx][2] );

                    coords.add( vertices[polygons[polyIdx].vertexIndices[faceVtx]].vertex[1] );
                    normals.add( polygons[polyIdx].vertexNormals[faceVtx][1] );

                    uvs.add( polygons[polyIdx].s[faceVtx] );
                    uvs.add( polygons[polyIdx].t[faceVtx] );
                }
            }

            /*unsigned index = */meshes.add( new Mesh( meshInfo.name, 0, coords, normals, uvs ) );
/*
            if ( linkMeshBegin < 0 || index > ( unsigned )linkMeshBegin )
                linkMeshBegin = index;
*/
            materialIndices.add( input->read<char>() );
        }

        numMaterials = input->read<unsigned short>();

        while ( numMaterials-- )
        {
            ms3d_material_t matInfo = input->readUnsafe<ms3d_material_t>();
            Material* mat = new Material;

            mat->name = matInfo.name;
            mat->ambient = Colour( matInfo.ambient );
            mat->diffuse = Colour( matInfo.diffuse );
            mat->specular = Colour( matInfo.specular );
            mat->emissive = Colour( matInfo.emissive );
            mat->shininess = matInfo.shininess;
            mat->alpha = matInfo.transparency;
            mat->texture = resMgr->getTexture( matInfo.texture, true );

            materials.add( mat );
            /*printf( "'%s': ambient(%g, %g, %g), diffuse(%g, %g, %g),\n\tspecular(%g, %g, %g), emissive(%g, %g, %g),\n\tshininess(%g), alpha(%g), mode(%i), texture('%s')\n\n",
                    matInfo.name, matInfo.ambient[0], matInfo.ambient[1], matInfo.ambient[2],
                    matInfo.diffuse[0], matInfo.diffuse[1], matInfo.diffuse[2],
                    matInfo.specular[0], matInfo.specular[1], matInfo.specular[2],
                    matInfo.emissive[0], matInfo.emissive[1], matInfo.emissive[2],
                    matInfo.shininess, matInfo.transparency, matInfo.mode, matInfo.texture );*/
        }

        iterate ( meshes )
        {
            //printf( "linking %i(%s): material #%i\n", meshes.iter(), meshes.current()->name.c_str(), materialIndices[meshes.iter()] );

            if ( materialIndices[meshes.iter()] >= 0 && ( unsigned )materialIndices[meshes.iter()] < materials.getLength() )
                meshes.current()->material = materials[materialIndices[meshes.iter()]]->reference();
        }

        iterate ( materials )
            materials.current()->release();
    }

    void loadMs3d( List<Mesh*>& meshes, InputStream* input, ResourceManager* textureManager )
    {
        ParserMs3d( meshes, input, textureManager ).parseInput();
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
