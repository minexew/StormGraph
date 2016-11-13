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

#include <Moxillan/Node.hpp>

#include <littl.hpp>

namespace Moxillan
{
    static const uint8_t node_file = 0x01, node_dir = 0x02, node_sys = 0x04, node_compressed = 0x08;

    static const char headerMagic[6] = "\xCCMox1";

    // 8 bytes
    struct CommonHeader
    {
        char magic[6];
        uint16_t formatVersion;
    };

    // 8 bytes
    struct Header_0x0110
    {
        uint32_t fileTableBegin;
        uint32_t fileTableEnd;
    };

    // 12 bytes
    struct Header_0x0111
    {
        enum Flags { fileTableCompressed = 1, usesDeflate = 2 };

        uint32_t fileTableBegin;
        uint32_t fileTableEnd;

        uint32_t flags;
    };
}
