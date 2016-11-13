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

#include "OpenGlDriver.hpp"

extern "C" __declspec( dllexport ) StormGraph::IGraphicsDriver* createGraphicsDriver( const char* driverName, StormGraph::IEngine* engine )
{
    if ( strcmp( driverName, "OpenGl" ) == 0 )
        return new OpenGlDriver::OpenGlDriver( engine );
    else
        return 0;
}

#ifdef __li_MSW
extern "C" BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
    switch ( fdwReason )
    {
        case DLL_PROCESS_ATTACH:
            // attach to process
            // return FALSE to fail DLL load
            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }

    return TRUE; // succesful
}
#endif

namespace OpenGlDriver
{
    unsigned numPolysThisFrame;
    OpenGlApi glApi;
    Shared driverShared;
    FrameStats stats;

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

        return ( int ) stream->getPos();
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

        SG_assert3( rwOps != NULL, "OpenGlDriver.getRwOps" );

        rwOps->seek = RwOps_seek;
        rwOps->read = RwOps_read;
        rwOps->write = RwOps_write;
        rwOps->close = RwOps_close;
        rwOps->hidden.unknown.data1 = stream;

        return rwOps;
    }
}
