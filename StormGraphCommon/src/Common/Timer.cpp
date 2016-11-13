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

#include <StormGraph/GraphicsDriver.hpp>

namespace StormGraph
{
#ifdef __li_MSW
    static bool frequencyKnown = false;
    static LARGE_INTEGER frequency, temp;
#endif

    Timer::Timer() : startTicks( 0 ), pauseTicks( 0 ), paused( false ), started( false )
    {
    }

    uint64_t Timer::getMicros() const
    {
        if ( started )
        {
            if ( paused )
                return pauseTicks;
            else
                return getRelativeMicroseconds() - startTicks;
        }

        return 0;
    }

    /*uint64_t Timer::getMilis() const
    {
        return getMicros() / 1000;
    }*/

    uint64_t Timer::getRelativeMicroseconds()
    {
#ifdef __li_MSW
        if ( !frequencyKnown )
        {
            QueryPerformanceFrequency( &frequency );
            frequencyKnown = true;
        }

        QueryPerformanceCounter( &temp );
        return temp.QuadPart * 1000000 / frequency.QuadPart;
#else
#error Define me!!!
#endif
    }

    void Timer::pause()
    {
        if ( started && !paused )
        {
            paused = true;
            pauseTicks = getRelativeMicroseconds() - startTicks;
        }
    }

    void Timer::reset()
    {
        if ( paused )
            pauseTicks = 0;
        else
            startTicks = getRelativeMicroseconds();
    }

    void Timer::start()
    {
        startTicks = getRelativeMicroseconds() - pauseTicks;
        pauseTicks = 0;

        started = true;
        paused = false;
    }

    void Timer::stop()
    {
        started = false;
        paused = false;
    }
}
