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

#include <StormGraph/Core.hpp>
#include <StormGraph/IO/Bsp.hpp>

namespace StormGraph
{
    struct TerrainBuildInfo
    {
        Vector2<unsigned> resolution;
        Vector<> pos, origin, size;
        Vector2<> uv[3][2], lightUv[2];
        unsigned materialIndex;
    };

    class IHeightMap
    {
        public:
            virtual ~IHeightMap() {}

            virtual void buildTerrain( const TerrainBuildInfo* buildInfo, List<Vertex>& vertices ) = 0;
            virtual void buildTerrain( const TerrainBuildInfo* buildInfo, List<BspPolygon>& polygons ) = 0;

            virtual float get( unsigned x, unsigned y ) = 0;
            virtual float get( Vector2<float> uv ) = 0;
            virtual const Vector2<unsigned>& getResolution() = 0;
            virtual void set( unsigned x, unsigned y, float value ) = 0;
    };
}
