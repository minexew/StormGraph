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

#include <StormGraph/Common.hpp>

namespace StormGraph
{
    struct Image
    {
        li_enum_class( Format ) { dxt1, dxt3, dxt5, bgra, rgb, rgba };
        li_enum_class( StorageFormat ) { original, ddsDxt1, ddsDxt3, ddsDxt5 };

        Format format;
        Vector<size_t> size;
        Array<uint8_t> data;

        Object<Image> next;
    };

    class IImageLoader
    {
        public:
            virtual ~IImageLoader() {}

            /**
             *  Load an image from the specified stream.
             *  The following formats are currently supported: PNG, JPEG, DDS, StormGraph Dxt*, StormGraph Raw
             *
             *  @param input the input stream
             *  @param required throw an exception if loading fails?
             */
            virtual Image* load( SeekableInputStream* input, bool required = true ) = 0;
    };
}
