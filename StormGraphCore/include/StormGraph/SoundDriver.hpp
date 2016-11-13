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

namespace StormGraph
{
    class ISoundStream : public IResource
    {
        public:
            struct Info
            {
                unsigned numChannels, frequency, bitsPerSample;
            };

        public:
            li_ReferencedClass_override( ISoundStream )

            virtual ~ISoundStream() {}

            virtual void getInfo( Info& output ) = 0;
            virtual size_t read( void* output, size_t numSamples ) = 0;
    };

    class ISoundSource : public ReferencedClass
    {
        protected:
            virtual ~ISoundSource() {}

        public:
            li_ReferencedClass_override( ISoundSource )

            virtual void play() = 0;
    };

    class ISoundDriver : public IEventListener
    {
        public:
            virtual ~ISoundDriver() {}

            virtual ISoundSource* createSoundSource( ISoundStream* stream ) = 0;
    };
}
