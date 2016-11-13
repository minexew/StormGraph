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

#include <StormGraph/IO/FileSystem.hpp>

#include <Moxillan/Package.hpp>

#include <littl/File.hpp>

namespace StormGraph
{
    class MoxFileSystemDriver : public IFileSystemDriver
    {
        public:
            virtual ~MoxFileSystemDriver();

            virtual IFileSystem* createFileSystem( const char* name ) override;
            virtual void getDriverInfo( Info* info ) override;
    };

    class NativeFileSystemDriver : public IFileSystemDriver
    {
        public:
            virtual ~NativeFileSystemDriver();

            virtual IFileSystem* createFileSystem( const char* name ) override;
            virtual void getDriverInfo( Info* info ) override;
    };

    class UnionFileSystemDriver : public IFileSystemDriver
    {
        public:
            virtual ~UnionFileSystemDriver();

            virtual IFileSystem* createFileSystem( const char* name ) override;
            virtual void getDriverInfo( Info* info ) override;
    };

    class MoxFileSystem : public IFileSystem
    {
        Object<Moxillan::Package> package;
        String prefix;

        public:
            MoxFileSystem( Moxillan::Package* package, const String& prefix );
            virtual ~MoxFileSystem();

            virtual SeekableInputStream* openInput( const char* fileName ) override;
    };

    class NativeFileSystem : public IFileSystem
    {
        String prefix;

        public:
            NativeFileSystem( const char* prefix );
            virtual ~NativeFileSystem();

            virtual SeekableInputStream* openInput( const char* fileName ) override;
    };

    class UnionFileSystem : public IUnionFileSystem
    {
        List<IFileSystem*> fileSystems;

        public:
            UnionFileSystem();
            virtual ~UnionFileSystem();

            virtual void add( IFileSystem* fs ) override;
            virtual void clear() override;
            virtual SeekableInputStream* openInput( const char* fileName ) override;
            virtual size_t getNumFileSystems() const override;
    };

    MoxFileSystemDriver::~MoxFileSystemDriver()
    {
    }

    IFileSystem* MoxFileSystemDriver::createFileSystem( const char* name )
    {
        String fs( name );
        String packageName, prefix;

        int semiPos = fs.findChar( ';' );

        if ( semiPos < 0 )
            packageName = fs;
        else
        {
            packageName = fs.leftPart( semiPos );
            prefix = fs.dropLeftPart( semiPos + 1 );
        }

        File* file = File::open( packageName );

        if ( file )
            return new MoxFileSystem( new Moxillan::Package( file ), prefix );

        return nullptr;
    }

    void MoxFileSystemDriver::getDriverInfo( Info* info )
    {
        info->description = "Moxillan package [read-only]";
    }

    NativeFileSystemDriver::~NativeFileSystemDriver()
    {
    }

    IFileSystem* NativeFileSystemDriver::createFileSystem( const char* name )
    {
        return new NativeFileSystem( name );
    }

    void NativeFileSystemDriver::getDriverInfo( Info* info )
    {
        info->description = "Native OS file system";
    }

    UnionFileSystemDriver::~UnionFileSystemDriver()
    {
    }

    IFileSystem* UnionFileSystemDriver::createFileSystem( const char* name )
    {
        return new UnionFileSystem();
    }

    void UnionFileSystemDriver::getDriverInfo( Info* info )
    {
        info->description = "Combines multiple file systems in a single namespace";
    }

    MoxFileSystem::MoxFileSystem( Moxillan::Package* package, const String& prefix ) : package( package ), prefix( prefix )
    {
    }

    MoxFileSystem::~MoxFileSystem()
    {
    }

    SeekableInputStream* MoxFileSystem::openInput( const char* fileName )
    {
        if ( !package )
            return 0;

        Moxillan::Node* node = package->findFile( prefix + fileName );

        if ( node != nullptr )
            return package->openFile( node, Moxillan::Package::random );
        else
            return 0;
    }

    NativeFileSystem::NativeFileSystem( const char* prefix )
            : prefix( prefix )
    {
    }

    NativeFileSystem::~NativeFileSystem()
    {
    }

    SeekableInputStream* NativeFileSystem::openInput( const char* fileName )
    {
        File* file = File::open( prefix + fileName );

        return file;
    }

    UnionFileSystem::UnionFileSystem()
    {
    }

    UnionFileSystem::~UnionFileSystem()
    {
        clear();
    }

    void UnionFileSystem::add( IFileSystem* fs )
    {
        SG_assert3( fs != nullptr, "StormGraph.UnionFileSystem.add" )

        fileSystems.add( fs );
    }

    void UnionFileSystem::clear()
    {
        reverse_iterate ( fileSystems )
            fileSystems.current()->release();

        fileSystems.clear();
    }

    SeekableInputStream* UnionFileSystem::openInput( const char* fileName )
    {
        iterate ( fileSystems )
        {
            SeekableInputStream* input = fileSystems.current()->openInput( fileName );

            if ( input != nullptr )
                return input;
        }

        return nullptr;
    }

    size_t UnionFileSystem::getNumFileSystems() const
    {
        return fileSystems.getLength();
    }

    IFileSystemDriver* createMoxFileSystemDriver()
    {
        return new MoxFileSystemDriver();
    }

    IFileSystemDriver* createNativeFileSystemDriver()
    {
        return new NativeFileSystemDriver();
    }

    IFileSystemDriver* createUnionFileSystemDriver()
    {
        return new UnionFileSystemDriver();
    }

    IUnionFileSystem* createUnionFileSystem()
    {
        return new UnionFileSystem();
    }
}
