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

#pragma once

//#include <StormGraph/Core.hpp>
#include <StormGraph/GraphicsDriver.hpp>

namespace StormGraph
{
    class IMaterial;

    class BspMesh
    {
        public:
            List<unsigned> indices;
            unsigned material;

        public:
            BspMesh( unsigned material ) : material( material )
            {
            }
    };

    class BspNode
    {
        public:
            Vector<float> bounds[2];
            List<BspMesh*> meshes;
            BspNode* children[2];

        public:
            BspNode()
            {
                children[0] = nullptr;
                children[1] = nullptr;
            }

            ~BspNode()
            {
                iterate ( meshes )
                    delete meshes.current();

                delete children[0];
                delete children[1];
            }
    };

    struct BspMaterial
    {
        BspMaterial& operator = ( BspMaterial&& other )
        {
            name = ( String&& ) other.name;
            properties = ( Object<MaterialStaticProperties>&& ) other.properties;

            return *this;
        }

        String name;
        Object<MaterialStaticProperties> properties;
    };

    class BspTree
    {
        public:
            List<BspMaterial> materials;

            Array<List<Vertex>> vertices;
            Array<unsigned> totalTriangles;

            Object<BspNode> root;
    };
}
