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

#include <Moxillan/BinaryFormat.hpp>
#include <Moxillan/Package.hpp>

#ifndef moxillan_no_zlib
#include <zlib.h>

#include <littl/ZlibDecompressor.hpp>
#endif

//static const size_t ioBufferCapacity = 0x10000;
//static uint8_t ioBuffer[ioBufferCapacity];

namespace Moxillan
{
    class Node : public DirEntry
    {
        public:
            uint64_t offset;

            List<Node*> contents;

        public:
            Node( const String& name, bool isDirectory, size_t contentsLength = 0 ) : contents( contentsLength )
            {
                node = this;

                this->name = name;
                this->isDirectory = isDirectory;
            }

            ~Node()
            {
                iterate ( contents )
                    delete contents.current();
            }

            Node* find( const char* name, bool wantDirectory )
            {
                if ( name == nullptr || *name == 0 )
                    return wantDirectory ? this : nullptr;

                iterate ( contents )
                    if ( contents.current()->name == name )
                    {
                        if ( contents.current()->isDirectory == wantDirectory )
                            return contents.current();
                        else
                            return nullptr;
                    }

                return nullptr;
            }

            void print( unsigned indent ) const
            {
                printf( "  " );
                repeat ( indent )
                    printf( " |" );

                printf( "- %s", name.c_str() );

                if ( !isDirectory )
                    printf( "(%" PRIu64 "/%" PRIu64 ")", compressedSize, size );

                printf( "\n" );

                if ( isDirectory )
                    for each_in_list ( contents, i )
                        contents[i]->print( indent + 1 );
            }
    };

    class MoxillanFileStream : public SeekableInputStream
    {
        protected:
            Package* package;
            Reference<SeekableInputStream> input;
            uint64_t offset, size, pos;

        public:
            MoxillanFileStream( Package* package, SeekableInputStream* input, uint64_t offset, uint64_t size )
                    : package( package ), input( input ), offset( offset ), size( size ), pos( 0 )
            {
            }

            ~MoxillanFileStream()
            {
            }

            virtual uint64_t getPos()
            {
                return pos;
            }

            virtual uint64_t getSize()
            {
                return size;
            }

            virtual bool setPos( uint64_t pos )
            {
                this->pos = pos;
                return true;
            }

            virtual bool isEof()
            {
                return pos >= size;
            }

            virtual bool isReadable()
            {
                return size > 0;
            }

            virtual size_t read( void* output, size_t length )
            {
                CriticalSection lock( package );

                if ( isEof() )
                    return 0;

                if ( pos + length > size )
                    length = ( size_t )( size - pos );

                input->setPos( offset + pos );
                pos += length;

                return input->read( output, length );
            }

            virtual size_t rawRead( void* output, size_t length )
            {
                return read( output, length );
            }
    };

#ifndef moxillan_no_zlib
    class MoxillanCompressedFileStream : public MoxillanFileStream
    {
        uint64_t pos, size;
        z_stream stream;

        size_t inputBufferSize;
        uint8_t* inputBuffer;

        public:
            MoxillanCompressedFileStream( Package* package, SeekableInputStream* input, uint64_t offset, uint64_t compressedSize, uint64_t size, size_t inputBufferSize )
                    : MoxillanFileStream( package, input, offset, compressedSize ), pos( 0 ), size( size ), inputBufferSize( inputBufferSize ), inputBuffer( nullptr )
            {
                inputBuffer = Allocator<>::allocate( inputBufferSize );

                size_t available = MoxillanFileStream::read( inputBuffer, inputBufferSize );
                stream.avail_in = available;
                stream.next_in = ( Bytef* ) inputBuffer;

                stream.zalloc = Z_NULL;
                stream.zfree = Z_NULL;

                inflateInit( &stream );
            }

            ~MoxillanCompressedFileStream()
            {
                inflateInit( &stream );

                Allocator<>::release( inputBuffer );
            }

            virtual uint64_t getPos()
            {
                return pos;
            }

            virtual uint64_t getSize()
            {
                return size;
            }

            virtual bool setPos( uint64_t pos )
            {
                if ( pos == 0 )
                {
                    this->pos = pos;

                    inflateEnd( &stream );

                    size_t available = MoxillanFileStream::read( inputBuffer, inputBufferSize );
                    stream.avail_in = available;
                    stream.next_in = ( Bytef* ) inputBuffer;

                    stream.zalloc = Z_NULL;
                    stream.zfree = Z_NULL;

                    inflateInit( &stream );

                    return true;
                }

                return false;
            }

            virtual bool isEof()
            {
                return pos >= size;
            }

            virtual size_t read( void* output, size_t length )
            {
                if ( isEof() )
                    return 0;

                if ( pos + length > size )
                    length = ( size_t )( size - pos );

                stream.avail_out = length;
                stream.next_out = ( Bytef* ) output;

                // Go on as long as we need more data
                while ( stream.avail_out > 0 )
                {
                    // Provide more input, if possible
                    if ( stream.avail_in == 0 )
                    {
                        size_t available = MoxillanFileStream::read( inputBuffer, inputBufferSize );

                        if ( available == 0 )
                            break;

                        stream.avail_in = available;
                        stream.next_in = ( Bytef* ) inputBuffer;
                    }

                    printf( "inb4 inflate(): %u to %u\n", stream.avail_in, stream.avail_out );

                    inflate( &stream, Z_SYNC_FLUSH );
                }

                pos += length - stream.avail_out;
                return length - stream.avail_out;
            }

            virtual size_t rawRead( void* output, size_t length )
            {
                return read( output, length );
            }
    };
#endif

    static Node* findChild( Node* currentDir, String path, bool wantDirectory )
    {
        while ( currentDir != nullptr )
        {
            while ( path.beginsWith( '/' ) || path.beginsWith( '\\' ) )
                path = path.dropLeftPart( 1 );

            int pos = maximum( path.findChar( '/' ), path.findChar( '\\' ) );

            if ( pos < 0 )
                return currentDir->find( path, wantDirectory );

            currentDir = currentDir->find( path.leftPart( pos ), true );
            path = path.dropLeftPart( pos + 1 );
        }

        return nullptr;
    }

    Package::Package( SeekableInputStream* input ) : input( input )
    {
        CommonHeader commonHeader;

        if ( !input->read( &commonHeader, sizeof( CommonHeader ) ) )
            throw Exception( "Moxillan.Package.Package", "PackageOpenError", "Failed to read package header" );

        if ( memcmp( commonHeader.magic, headerMagic, 6 ) != 0 )
            throw Exception( "Moxillan.Package.Package", "NotAPackage", "The input is not a valid Moxillan package" );

        Header_0x0111 header;

        if ( commonHeader.formatVersion == 0x0110 )
        {
            Header_0x0110 altHeader;

            if ( !input->read( &altHeader, sizeof( altHeader ) ) )
                throw Exception( "Moxillan.Package.Package", "UnexpectedEOF", "Unexpected end of package" );

            header.fileTableBegin = altHeader.fileTableBegin;
            header.fileTableEnd = altHeader.fileTableEnd;
            header.flags = 0;
        }
        else if ( commonHeader.formatVersion == 0x0111 )
        {
            if ( !input->read( &header, sizeof( header ) ) )
                throw Exception( "Moxillan.Package.Package", "UnexpectedEOF", "Unexpected end of package" );
        }
        else
            throw Exception( "Moxillan.Package.Package", "VersionError", "Unsupported package version: 0x" + String::formatInt( commonHeader.formatVersion, -1, String::hexadecimal ) );

#ifdef moxillan_no_zlib
        if ( ( header.flags & Header_0x0111::fileTableCompressed ) | ( header.flags & Header_0x0111::usesDeflate ) )
            throw Exception( "Moxillan.Package.Package", "FeatureUnsupported", "Package uses compression which is disabled in this build" );
#endif

        input->setPos( header.fileTableBegin );
        setIoBufferCapacity( 0x4000 );

        fileTableLength = header.fileTableEnd - header.fileTableBegin;
        dataLength = 0;

        size_t rootLength = input->read<uint16_t>();
        rootNode = new Node( ( char* ) nullptr, true, rootLength );

#ifndef moxillan_no_zlib
        if ( header.flags & Header_0x0111::fileTableCompressed )
        {
            fileTableCompressed = true;

            Reference<ZlibDecompressor> decompressor = new ZlibDecompressor( input->reference(), fileTableLength );

            for ( size_t i = 0; i < rootLength; i++ )
            rootNode->contents.add( readNode( decompressor ) );
        }
        else
#endif
        {
            fileTableCompressed = false;

            for ( size_t i = 0; i < rootLength; i++ )
                rootNode->contents.add( readNode( input ) );
        }

        rootNode->size = rootNode->contents.getLength();

        //rootNode->print( 1 );
    }

    Package::~Package()
    {
    }

#ifndef moxillan_no_zlib
    size_t Package::decompress( uint8_t* buffer, uint64_t offset, uint64_t compressedSize, size_t size )
    {
        z_stream stream;
        bool started = false;

        stream.avail_out = size;
        stream.next_out = buffer;

        input->setPos( offset );

        uint64_t maxPos = offset + compressedSize;

        while ( stream.avail_out > 0 )
        {
            size_t count = input->read( ioBuffer.getPtr(), minimum<size_t>( ioBuffer.getCapacity(), ( size_t )( maxPos - input->getPos() ) ) );

            if ( count == 0 )
                break;

            stream.avail_in = count;
            stream.next_in = ( Bytef* ) ioBuffer.getPtr();

            if ( !started )
            {
                stream.zalloc = Z_NULL;
                stream.zfree = Z_NULL;
                inflateInit( &stream );
                started = true;
            }

            inflate( &stream, Z_SYNC_FLUSH );
        }

        inflateEnd( &stream );

        return size - stream.avail_out;
    }
#endif

    Node* Package::findDirectory( const char* path )
    {
        return findChild( rootNode, path, true );
    }

    Node* Package::findFile( const char* path )
    {
        return findChild( rootNode, path, false );
    }

    void Package::getInfo( PackageInfo* info )
    {
        info->headerLength = sizeof( CommonHeader ) + sizeof( Header_0x0110 );
        info->dataLength = dataLength;
        info->fileTableLength = fileTableLength;
        info->fileTableCompressed = fileTableCompressed;
    }

    void Package::getNodeInfo( Node* node, DirEntry* info )
    {
        *info = * static_cast<const DirEntry*>( node );
    }

    void Package::listDirectory( Node* directory, List<DirEntry>& contents )
    {
        if ( directory == nullptr )
            return;

        iterate ( directory->contents )
            contents.add( * static_cast<const DirEntry*>( directory->contents.current() ) );
    }

    SeekableInputStream* Package::openFile( Node* node, AccessStrategy strategy )
    {
        if ( node == nullptr )
            return nullptr;

        if ( !node->isCompressed )
            return new MoxillanFileStream( this, input->reference(), node->offset, node->size );
        else
        {
#ifndef moxillan_no_zlib
            if ( strategy == sequential )
                return new MoxillanCompressedFileStream( this, input->reference(), node->offset, node->compressedSize, node->size, 4096 );
            else
            {
                Reference<ArrayIOStream> buffer = new ArrayIOStream( ( size_t ) node->size );

                buffer->setSize( decompress( buffer->getPtr(), node->offset, node->compressedSize, ( size_t ) node->size ) );

                return buffer.detach();
			}
#else
			throw Exception( "Moxillan.Package.Package", "FeatureUnsupported", "Package uses compression which is disabled in this build" );
#endif
        }

        return nullptr;
    }

    SeekableInputStream* Package::openFile( const char* path, AccessStrategy strategy )
    {
        return openFile( findFile( path ), strategy );
    }

    Node* Package::readNode( InputStream* input )
    {
        String name = input->readString();
        uint8_t type = input->read<uint8_t>();

        //printf( "%02X %s\n", type, name.c_str() );

        if ( type & node_file )
        {
            Object<Node> file = new Node( name, false );

            file->isCompressed = ( type & node_compressed ) != 0;
            file->offset = input->read<uint32_t>();
            file->compressedSize = input->read<uint32_t>();
            file->size = input->read<uint32_t>();

            dataLength += file->size;

            return file.detach();
        }
        else if ( type & node_dir )
        {
            size_t directoryLength = input->read<uint16_t>();
            Object<Node> directory = new Node( name, true, directoryLength );

            for ( size_t i = 0; i < directoryLength; i++ )
                directory->contents.add( readNode( input ) );

            directory->size = directory->contents.getLength();

            return directory.detach();
        }
        else
            throw Exception( "Moxillan.Package.Package", "PackageFormatError", "Invalid node type 0x" + String::formatInt( type, -1, String::hexadecimal ) );
    }

    void Package::setIoBufferCapacity( size_t capacity )
    {
        ioBuffer.resize( capacity, false );
    }
}
