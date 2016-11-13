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

#include <StormGraph/IO/Bsp.hpp>
#include <StormGraph/ContentTools.hpp>

namespace StormGraph
{
    SgContentToolsClass Bsp
    {
        unsigned nodePolyLimit;
        Vector<float> nodeVolumeLimit;

        List<BspMaterial> materials;
        Array<unsigned> totalTriangles;
        Object<BspTree> tree;

        unsigned breakPoly( const BspPolygon& polygon, List<unsigned>& indices );
        unsigned getVertexIndex( unsigned materialIndex, const Vertex& vertex );

        public:
            Bsp( unsigned nodePolyLimit, const Vector<float>& nodeVolumeLimit );
            ~Bsp();

            BspTree* generate( const BspPolygon* polygons, size_t count );
            unsigned getMaterialIndex( const char* name );

            BspNode* partition( const BspPolygon* polygons, size_t count );
            unsigned registerMaterial( const char* name, MaterialStaticProperties* material );
    };

    //typedef Bsp BspGenerator;
#define BspGenerator Bsp

    SgContentToolsClass BspWriter
    {
        unsigned indexSize;

        void save( BspNode* node, OutputStream* output );

        public:
            void save( BspTree* tree, OutputStream* output );
    };
}
