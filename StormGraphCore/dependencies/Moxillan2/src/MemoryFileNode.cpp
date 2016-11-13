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
    MemoryFileNode::MemoryFileNode( const char* fileName, uint8_t* data, size_t length, int compression )
            : fileName( fileName ), compression( compression )
    {
        this->data = new ArrayIOStream( data, length );
    }

    MemoryFileNode::MemoryFileNode( const char* fileName, const String& data, int compression )
            : fileName( fileName ), compression( compression )
    {
        this->data = new ArrayIOStream( data );
    }

    MemoryFileNode::~MemoryFileNode()
    {
    }

    int MemoryFileNode::getCompression()
    {
        return compression;
    }

    InputStream* MemoryFileNode::getInputStream()
    {
        return data->reference();
    }

    String MemoryFileNode::getName()
    {
        return fileName;
    }
}

