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

#include "Internal.hpp"

#include <StormGraph/Profiler.hpp>

namespace StormGraph
{
    class Profiler : public IProfiler
    {
        struct Section
        {
            const char* name;
            uint64_t begin, end;
            bool printed;
        };

        List<Section> sections;

        void printRangeStats( uint64_t min, uint64_t max, uint64_t offset, unsigned depth );

        public:
            virtual ~Profiler() {}

            virtual unsigned enter( const char* name ) override;
            virtual int64_t getDelta( unsigned id ) override { return sections[id].end - sections[id].begin; }
            virtual void leave( unsigned id ) override;
            virtual void reenter( unsigned id ) override;

            virtual void printStats( OutputStream* output ) override;
    };

    unsigned Profiler::enter( const char* name )
    {
#ifdef li_GCC4
        return sections.add( Section { name, Timer::getRelativeMicroseconds() } );
#else
        Section sect = { name, Timer::getRelativeMicroseconds() };
        return sections.add( sect );
#endif
    }

    void Profiler::leave( unsigned id )
    {
        sections[id].end = Timer::getRelativeMicroseconds();
    }

    void Profiler::printRangeStats( uint64_t min, uint64_t max, uint64_t offset, unsigned depth )
    {
        iterate2 ( i, sections )
        {
            Section& section = i;

            if ( section.printed )
                continue;

            if ( depth == 0 || ( section.begin >= min && section.end <= max ) )
            {
                section.printed = true;

                repeat ( depth )
                    printf( "  " );

                printf( "[%" PRIu64 " ..\t%" PRIu64 "]\t(%" PRIu64 " us)\t%s\n", section.begin - offset, section.end - offset, section.end - section.begin, section.name );

                printRangeStats( section.begin, section.end, offset, depth + 1 );
            }
        }
    }

    void Profiler::printStats( OutputStream* output )
    {
        if ( !sections.isEmpty() )
        {
            iterate2 ( i, sections )
                ( *i ).printed = false;

            printRangeStats( 0, 0, sections[0].begin, 0 );
        }
    }

    void Profiler::reenter( unsigned id )
    {
        sections[id].begin = Timer::getRelativeMicroseconds();
    }

    IProfiler* createProfiler( IEngine* engine )
    {
        return new Profiler();
    }
}
