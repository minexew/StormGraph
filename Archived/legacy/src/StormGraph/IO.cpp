
#include "Internal.hpp"
#include "Moxillan.hpp"

namespace StormGraph
{
    static int RwOps_close( struct SDL_RWops* context )
    {
        SeekableInputStream* stream = ( SeekableInputStream* )( context->hidden.unknown.data1 );

        release( stream );
        SDL_FreeRW( context );

        return 0;
    }

    static int RwOps_read( struct SDL_RWops* context, void* ptr, int size, int maxnum )
    {
        SeekableInputStream* stream = ( SeekableInputStream* )( context->hidden.unknown.data1 );

        if ( size <= 0 )
            return 0;

        return stream->read( ptr, size * maxnum ) / size;
    }

    static int RwOps_seek( struct SDL_RWops* context, int offset, int whence )
    {
        SeekableInputStream* stream = ( SeekableInputStream* )( context->hidden.unknown.data1 );

        if ( whence == RW_SEEK_SET )
            stream->setPos( offset );
        else if ( whence == RW_SEEK_CUR )
            stream->seek( offset );
        else
            stream->setPos( stream->getSize() - offset );

        return stream->getPos();
    }

    static int RwOps_write( struct SDL_RWops* context, const void* ptr, int size, int num )
    {
        printf( "OH SHI- RwOpsWrapper::write(%p, %p, %i, %i)\n", context, ptr, size, num );
        getchar();
        return -1;
    }

    SDL_RWops* getRwOps( SeekableInputStream* stream )
    {
        SDL_RWops* rwOps = SDL_AllocRW();

        SG_assert( rwOps != NULL, "StormGraph::IO", "getRwOps" );

        rwOps->seek = RwOps_seek;
        rwOps->read = RwOps_read;
        rwOps->write = RwOps_write;
        rwOps->close = RwOps_close;
        rwOps->hidden.unknown.data1 = stream;

        return rwOps;
    }
}
