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

#include <StormGraph/Abstract.hpp>

namespace StormGraph
{
    struct Ct2Line
    {
        Vector2<> a, b;

        // Pre-calculate those for ULTRA SPEEEEEEED
        Vector2<> length;
        float normalize;
        Vector2<> dir;

        void recalc()
        {
            length = b - a;
            normalize = 1.0f / length.getLength();
            dir = length * normalize;
        }
    };

    class Ct2Node
    {
        public:
            Vector2<> bounds[2];
            List<Ct2Line> lines;
            Ct2Node* children[2];

        public:
            Ct2Node()
            {
                children[0] = nullptr;
                children[1] = nullptr;
            }

            ~Ct2Node()
            {
                delete children[0];
                delete children[1];
            }
    };
}
