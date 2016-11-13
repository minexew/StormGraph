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
    class IFileSystem : public ReferencedClass
    {
        public:
            struct DirEntry
            {
                String name, path;
                bool isDirectory;
                int64_t size;
            };

        protected:
            virtual ~IFileSystem() {}

        public:
            li_ReferencedClass_override( IFileSystem )

            ///virtual unsigned listDirectory( const char* path, List<DirEntry>& entries ) = 0;
            virtual SeekableInputStream* openInput( const char* fileName ) = 0;
    };

    class IUnionFileSystem : public IFileSystem
    {
        protected:
            virtual ~IUnionFileSystem() {}

        public:
            li_ReferencedClass_override( IUnionFileSystem )

            virtual void add( IFileSystem* fs ) = 0;
            virtual void clear() = 0;
            virtual size_t getNumFileSystems() const = 0;
    };

    class IFileSystemDriver
    {
        public:
            struct Info
            {
                const char* description;
            };

        public:
            virtual ~IFileSystemDriver() {}

            virtual IFileSystem* createFileSystem( const char* name ) = 0;
            virtual void getDriverInfo( Info* info ) = 0;
    };
}
