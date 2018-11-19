
/*

String "#StormGraph.Model"

CHUNK "HEADER.0"
    uint8_t numMeshes, numMaterials;

CHUNK "MAT.0..."
    uint8_t id;

CHUNK "MESH.0.."
    uint8_t id;
    uint8_t type;
    float[4] colour, ambient, diffuse, specular, emissive;
    float alpha, shininess;

*/

#include <littl.hpp>

namespace StormGraph
{
    namespace IO
    {
        class Material
        {
            public:
                enum Type { solid = 0x01, textured = 0x02 } type;

                Colour colour, ambient, diffuse, specular, emissive;
                float alpha, shininess;

                String texture;
        };

        class Model
        {
            public:
                class Mesh
                {
                    String name;
                    enum Type { raw, indexed } type;
                    Material* material;

                    List<float> coords, normals, uvs;
                    List<unsigned> indices;
                };

            public:
                Model();

                static Model* load( SeekableInputStream* input );
        };
    }
}
