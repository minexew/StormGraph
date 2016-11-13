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

#include <Moxillan/Common.hpp>

#include <littl/Base.hpp>
#include <littl/BaseIO.hpp>
#include <littl/Thread.hpp>

namespace Moxillan
{
    using namespace li;

    class Node;

    struct DirEntry
    {
        String name;
        bool isDirectory, isCompressed;
        uint64_t size, compressedSize;

        Node* node;
    };

    struct PackageInfo
    {
        uint64_t headerLength, dataLength, fileTableLength;
        bool fileTableCompressed;
    };

    class Package : public Mutex
    {
        public:
            enum AccessStrategy { sequential, random };

        protected:
            Reference<SeekableInputStream> input;
            Object<Node> rootNode;

            uint64_t fileTableLength, dataLength;
            bool fileTableCompressed;

            Array<uint8_t> ioBuffer;

            size_t decompress( uint8_t* buffer, uint64_t offset, uint64_t compressedSize, size_t size );
            Node* readNode( InputStream* input );

        public:
            Package( SeekableInputStream* input );
            ~Package();

            // I/O
            void setIoBufferCapacity( size_t capacity );

            // Package
            void getInfo( PackageInfo* info );

            // Nodes
            void getNodeInfo( Node* node, DirEntry* info );
            Node* findDirectory( const char* path );
            Node* findFile( const char* path );

            // Directories
            void listDirectory( Node* directory, List<DirEntry>& contents );

            // Files
            SeekableInputStream* openFile( Node* node, AccessStrategy strategy );
            SeekableInputStream* openFile( const char* path, AccessStrategy strategy );
    };
}
