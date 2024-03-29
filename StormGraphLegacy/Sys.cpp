/*
    Copyright (c) 2012 Xeatheran Minexew

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

#include <StormGraph/Common.hpp>
#include <StormGraph/Sys.hpp>

namespace StormBase
{
    using namespace StormGraph;

    class Sys : public ISys
    {
        Timer frameDelta;

        public:
            virtual ~Sys() {}

            virtual double Update() override;
    };

    double Sys::Update()
    {
        double t;

        if ( frameDelta.isStarted() )
            t = frameDelta.getMicros() / 1000000.0;
        else
            t = 0.0;

        frameDelta.start();

        return t;
    }

    ISys* createSys(const char* app, int argc, char** argv)
    {
        return new Sys();
    }
}
