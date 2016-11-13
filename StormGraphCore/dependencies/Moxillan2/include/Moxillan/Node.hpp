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

#include <littl/BaseIO.hpp>
#include <littl/Directory.hpp>
#include <littl/String.hpp>

namespace Moxillan
{
    using namespace li;

    class INode
    {
        public:
            virtual ~INode() {}

            virtual int getCompression() = 0;
            virtual String getName() = 0;
    };

    class IDirectoryNode : public INode, public Iterable<INode*, size_t>
    {
        public:
            virtual ~IDirectoryNode() {}
    };

    class IFileNode : public INode
    {
        friend class PackageBuilder;

        bool compressed;
        uint64_t offset, compressedSize, size;

        public:
            virtual ~IFileNode() {}

            virtual InputStream* getInputStream() = 0;
    };

    class DirectoryNode : public IDirectoryNode
    {
        String directoryName;
        int compression;

        List<INode*> nodes;
        size_t iterator;

        public:
            DirectoryNode( const char* directoryName = nullptr, int compression = inherit );
            virtual ~DirectoryNode();

            virtual void add( INode* node );

            // Moxillan::INode
            virtual int getCompression() override;
            virtual String getName() override;

            // li::Iterable
            virtual size_t iterableGetLength() const;
            virtual INode* iterableGetItem( size_t index );
    };

    class MemoryFileNode : public IFileNode
    {
        String fileName;
        Reference<ArrayIOStream> data;
        int compression;

        public:
            MemoryFileNode( const char* fileName, uint8_t* data, size_t length, int compression = inherit );
            MemoryFileNode( const char* fileName, const String& data, int compression = inherit );
            virtual ~MemoryFileNode();

            // Moxillan::IFileNode
            virtual int getCompression() override;
            virtual InputStream* getInputStream() override;
            virtual String getName() override;
    };

    class NativeDirectoryNode : public IDirectoryNode
    {
        String nativeDirectoryName, directoryName;
        int compression;

        List<INode*> nodes;
        size_t iterator;

        public:
            NativeDirectoryNode( const char* nativeDirectoryName, const char* directoryName = nullptr, int compression = inherit, Directory* dir = nullptr );
            virtual ~NativeDirectoryNode();

            // Moxillan::INode
            virtual int getCompression() override;
            virtual String getName() override;

            // li::Iterable
            virtual size_t iterableGetLength() const;
            virtual INode* iterableGetItem( size_t index );
    };

    class NativeFileNode : public IFileNode
    {
        String nativeFileName, fileName;
        int compression;

        public:
            NativeFileNode( const char* nativeFileName, const char* fileName = nullptr, int compression = inherit );
            virtual ~NativeFileNode();

            // Moxillan::IFileNode
            virtual int getCompression() override;
            virtual InputStream* getInputStream() override;
            virtual String getName() override;
    };
}
