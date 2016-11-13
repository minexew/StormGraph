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

#include <littl/Exception.hpp>
#include <littl/File.hpp>
#include <littl/FileName.hpp>

namespace Moxillan
{
    NativeFileNode::NativeFileNode( const char* nativeFileName, const char* fileName, int compression )
            : nativeFileName( nativeFileName ), compression( compression )
    {
        this->fileName = ( fileName != nullptr ) ? ( String ) fileName : FileName( nativeFileName ).getFileName();
    }

    NativeFileNode::~NativeFileNode()
    {
    }

    int NativeFileNode::getCompression()
    {
        return compression;
    }

    InputStream* NativeFileNode::getInputStream()
    {
        File* file = File::open( nativeFileName );

        if ( file == nullptr )
            throw Exception( "Moxillan.NativeFileNode.getInputStream", "FileOpenError", "Failed to open " + FileName::format( nativeFileName ) );

        return file;
    }

    String NativeFileNode::getName()
    {
        return fileName;
    }
}
