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
#include <Moxillan/PackageBuilder.hpp>

#ifndef moxillan_no_zlib
#include <zlib.h>

#include <littl/ZlibCompressor.hpp>
#endif

namespace Moxillan
{
    void PackageBuilder::buildPackage( IDirectoryNode* rootDir, SeekableOutputStream* output, int compression, int tableCompression )
    {
        Reference<> outputGuard( output );

        CommonHeader commonHeader;
        memcpy( commonHeader.magic, headerMagic, 6 );
        commonHeader.formatVersion = 0x0111;

        Header_0x0111 header = { 0, 0, 0 };

        if ( tableCompression > 0 )
            header.flags |= Header_0x0111::fileTableCompressed;

        // Headers
        output->write( commonHeader );
        output->write( header );

        // File contents
        writeNodeData( rootDir, output, maximum( 0, compression ) );

        // File database
        // TODO: safe conversion
        header.fileTableBegin = ( uint32_t ) output->getPos();

        output->write<uint16_t>( rootDir->iterableGetLength() );

#ifndef moxillan_no_zlib
        if ( tableCompression > 0 )
        {
            Reference<ZlibCompressor> compressor = new ZlibCompressor( output->reference(), tableCompression );

            iterate ( *rootDir )
                writeNode( rootDir->current(), compressor );
        }
        else
#endif
        {
            iterate ( *rootDir )
                writeNode( rootDir->current(), output );
        }

        // TODO: safe conversion
        header.fileTableEnd = ( uint32_t ) output->getPos();

        output->setPos( sizeof( commonHeader ) );
        output->write( header );
    }

    uint64_t PackageBuilder::writeStream( InputStream* input, OutputStream* output, int compression )
    {
        Reference<> inputGuard( input );

        static const size_t ioBufferCapacity = 0x10000;
        static uint8_t ioBuffer[ioBufferCapacity];

        uint64_t size = 0;

#ifndef moxillan_no_zlib
        if ( compression == 0 )
        {
#endif
            while ( true )
            {
                size_t count = input->read( ioBuffer, ioBufferCapacity );

                //printf( "copying %u bytes\n", count );

                if ( count == 0 )
                    break;

                size += count;

                output->write( ioBuffer, count );
            }
#ifndef moxillan_no_zlib
        }
        else
        {
            z_stream stream;
            stream.zalloc = Z_NULL;
            stream.zfree = Z_NULL;
            deflateInit( &stream, compression );

            static uint8_t compressionBuffer[ioBufferCapacity];

            while ( true )
            {
                size_t count = input->read( ioBuffer, ioBufferCapacity );

                if ( count == 0 )
                    break;

                size += count;

                stream.next_in = ioBuffer;
                stream.avail_in = count;

                int status = 0;
                do
                {
                    stream.next_out = compressionBuffer;
                    stream.avail_out = ioBufferCapacity;
                    //printf( " -- status is %i, %u more bytes to compress into %u...", status, stream.avail_in, stream.avail_out );
                    status = deflate( &stream, Z_NO_FLUSH );
                    //printf( "%u used\n", ioBufferCapacity - stream.avail_out );
                    output->write( compressionBuffer, ioBufferCapacity - stream.avail_out );
                }
                while ( status == Z_OK && stream.avail_in );

                if ( status != Z_OK )
                {
                    printf( "ERROR: status %i/'%s'; terminating.\n", status, zError( status ) );
                    deflateEnd( &stream );
                    return false;
                }
            }

            //printf( "compressed ok, now writing the rest...\n" );

            int status = 0;
            do
            {
                stream.next_out = compressionBuffer;
                stream.avail_out = ioBufferCapacity;
                //printf( " -- finalize: status is %i, writing into %u...", status, stream.avail_out );
                status = deflate( &stream, Z_FINISH );

                //printf( "%u used\n", ioBufferCapacity - stream.avail_out );
                output->write( compressionBuffer, ioBufferCapacity - stream.avail_out );
            }
            while ( status == Z_OK );

            if ( status != Z_STREAM_END )
            {
                printf( "ERROR: status %i/'%s'; terminating.\n", status, zError( status ) );
                deflateEnd( &stream );
                return false;
            }

            //printf( "file OK, closing streams and encrypting.\n\n" );
            deflateEnd( &stream );
        }
#endif

        return size;
    }

    void PackageBuilder::writeNode( INode* node, OutputStream* output )
    {
        output->writeString( node->getName() );

        IFileNode* file = dynamic_cast<IFileNode*>( node );

        if ( file != nullptr )
        {
            output->write<uint8_t>( file->compressed ? ( node_file | node_compressed ) : node_file );
            output->write<uint32_t>( ( uint32_t ) file->offset );
            output->write<uint32_t>( ( uint32_t ) file->compressedSize );
            output->write<uint32_t>( ( uint32_t ) file->size );
            return;
        }

        IDirectoryNode* directory = dynamic_cast<IDirectoryNode*>( node );

        if ( directory != nullptr )
        {
            output->write<uint8_t>( node_dir );
            output->write<uint16_t>( directory->iterableGetLength() );

            iterate ( *directory )
                writeNode( directory->current(), output );

            return;
        }

        throw Exception( "Moxillan.PackageBuilder.writeNode", "UnknownNodeType", "Unknown type for node `" + node->getName() + "`" );
    }

    void PackageBuilder::writeNodeData( INode* node, SeekableOutputStream* output, int compression )
    {
        if ( node->getCompression() != inherit )
            compression = node->getCompression();

        IFileNode* file = dynamic_cast<IFileNode*>( node );

        if ( file != nullptr )
        {
            file->compressed = ( compression > 0 );
            file->offset = output->getPos();
            //printf( "%s begins @ %u\n", node->getName().c_str(), ( unsigned ) output->getPos() );
            file->size = writeStream( file->getInputStream(), output, compression );
            //printf( "%s ends @ %u\n", node->getName().c_str(), ( unsigned ) output->getPos() );
            file->compressedSize = output->getPos() - file->offset;
            return;
        }

        IDirectoryNode* directory = dynamic_cast<IDirectoryNode*>( node );

        if ( directory != nullptr )
        {
            iterate ( *directory )
                writeNodeData( directory->current(), output, compression );

            return;
        }

        throw Exception( "Moxillan.PackageBuilder.writeNodeData", "UnknownNodeType", "Unknown type for file `" + node->getName() + "`" );
    }
}

