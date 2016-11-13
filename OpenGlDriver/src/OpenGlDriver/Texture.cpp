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

namespace OpenGlDriver
{
    // These two routines will fail:
    //      prevPo2 for n = 0 (returns 0)
    //      nextPo2 for n > 2^31 (`i` never gets larger or equal to `n` beacuse of overflow, loops forever)
    //
    // none of these cases should actually happen though

    static unsigned prevPo2( unsigned n )
    {
        // Don't expect any fancy tricks
        // Speed is like the last thing we care about here

        for ( unsigned i = 1; ; i <<= 1 )
        {
            if ( i > n )
                return i >> 1;
            else if ( i == ( unsigned )( 1 << 31 ) )
                // would overflow
                return i;
        }
    }

    static unsigned nextPo2( unsigned n )
    {
        for ( unsigned i = 1; ; i <<= 1 )
            if ( i >= n )
                return i;
    }

    TexturePreload::TexturePreload( OpenGlDriver* driver, const char* name, Image* image, ILodFunction* lodFunction )
            : driver( driver ), name( name ), image( image ), finalized( 0 ), lodFunction( lodFunction )
    {
    }

    TexturePreload::~TexturePreload()
    {
    }

    /*TexturePreload* TexturePreload::createFromStream( SeekableInputStream* input, const char* name, LodFunction* lodFunction )
    {
        Image* image = load( input );

        if ( image )
            return new TexturePreload( name, image, lodFunction );
        else
            return 0;
    }*/

    ITexture* TexturePreload::getFinalized()
    {
        if ( finalized == nullptr )
        {
            printf( "Finalizing texture `%s`\n", name.c_str() );
            finalized = new Texture( this );
        }
        else
            printf( "Already finalized: `%s`\n", name.c_str() );

        return finalized->reference();
    }

    Texture::Texture( TexturePreload* preload )
            : driver( preload->driver ), name( preload->name ), texture( 0 ), bytesAlloc( 0 )
    {
        init( preload->image, preload->lodFunction.detach() );

        Resource::add( this );
    }

    Texture::Texture( OpenGlDriver* driver, const char* name, Image* image, ILodFunction* lodFunction )
            : driver( driver ), name( name ), texture( 0 ), bytesAlloc( 0 )
    {
        init( image, lodFunction );

        Resource::add( this );
    }

    Texture::Texture( OpenGlDriver* driver, const char* name, SDL_Surface* surface, ILodFunction* lodFunction )
            : driver( driver ), name( name ), texture( 0 ), bytesAlloc( 0 )
    {
        init( surface, lodFunction );

        Resource::add( this );
    }

    Texture::Texture( OpenGlDriver* driver, const char* name, const Colour& colour )
            : driver( driver ), name( name ), texture( 0 ), bytesAlloc( 0 ), size( 2, 2 )
    {
        glGenTextures( 1, &texture );
        glBindTexture( GL_TEXTURE_2D, texture );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

        const float data[] =
        {
            colour.r, colour.g, colour.b, colour.a,
            colour.r, colour.g, colour.b, colour.a,
            colour.r, colour.g, colour.b, colour.a,
            colour.r, colour.g, colour.b, colour.a
        };

        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_FLOAT, data );
        driver->checkErrors( "OpenGlDriver.Texture.Texture" );

        bytesAlloc = size.x * size.y * 4;
        driver->gpuStats.bytesInTextures += bytesAlloc;

        Resource::add( this );
    }

    Texture::Texture( OpenGlDriver*driver, const char* name, unsigned width, unsigned height )
            : driver( driver ), name( name ), texture( 0 ), bytesAlloc( 0 ), size( width, height )
    {
        glGenTextures( 1, &texture );
        glBindTexture( GL_TEXTURE_2D, texture );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
        driver->checkErrors( "OpenGlDriver.Texture.Texture" );

        bytesAlloc = size.x * size.y * 4;
        driver->gpuStats.bytesInTextures += bytesAlloc;

        Resource::add( this );
    }

    Texture::Texture( OpenGlDriver* driver, const char* name, const Vector2<unsigned>& size )
            : driver( driver ), name( name ), texture( 0 ), bytesAlloc( 0 ), size( size )
    {
        glGenTextures( 1, &texture );
        glBindTexture( GL_TEXTURE_2D, texture );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

        glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size.x, size.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr );
        driver->checkErrors( "OpenGlDriver.Texture.Texture" );

        bytesAlloc = size.x * size.y * 4;
        driver->gpuStats.bytesInTextures += bytesAlloc;

        Resource::add( this );
    }

    Texture::~Texture()
    {
        driver->gpuStats.bytesInTextures -= bytesAlloc;

        glDeleteTextures( 1, &texture );

        Resource::remove( this );
    }

    Vector<unsigned> Texture::getDimensions()
    {
        return size;
    }

    void Texture::init( Image* image, ILodFunction* lodFunction )
    {
        /**
         *  For compressed texture support:
         *      If image is raw
         *          Apply LOD, reformat to RGBA, upload
         *      Else
         *          If compressed textures supported
         *              ;
         */

        SG_assert( image != nullptr )

	    // Prepare the data for upload

        size = image->size.getXy();

        if ( image->format == Image::Format::rgba || image->format == Image::Format::rgb )
        {
            unsigned opp = image->size.z;

            unsigned width2 = size.x, height2 = size.y;

            // Legacy mode? Oh...
            if ( driverShared.requirePo2Textures )
            {
                width2 = ( width2 < driverShared.maxPo2Upscale ) ? nextPo2( width2 ) : prevPo2( width2 );
                height2 = ( height2 < driverShared.maxPo2Upscale ) ? nextPo2( height2 ) : prevPo2( height2 );
            }

            // Apply Level-of-detail
            unsigned lod = lodFunction ? lodFunction->getLod( driverShared.textureLod ) : driverShared.textureLod;

            if ( lodFunction )
                delete lodFunction;

            width2 = width2 >> lod;
            height2 = height2 >> lod;

            // Limit texture dimensions
            width2 = minimum( width2, driverShared.maxTextureSize );
            height2 = minimum( height2, driverShared.maxTextureSize );

            // Oh...
            width2 = maximum<unsigned>( 2, width2 );
            height2 = maximum<unsigned>( 2, height2 );

            uint8_t* scaled = 0;
            unsigned pitch = width2 * 4;

            // Round to a 4-byte boundary (both SDL & OpenGL do that)
            pitch = ( ( pitch - 1 ) & ~0x03 ) + 4;

            scaled = Allocator<uint8_t>::allocate( height2 * pitch );

            printf( "[Texture: '%s'] %ux%u px @ %u octets/pixel; scaling to %ux%u.\n", name.c_str(), size.x, size.y, opp, width2, height2 );

            for ( unsigned y2 = 0; y2 < height2; y2++ )
                for ( unsigned x2 = 0; x2 < width2; x2++ )
                {
                    unsigned x = x2 * size.x / width2;
                    unsigned y = size.y - y2 * size.y / height2 - 1;

                    // We've had some problems when keeping encoding as RGB (large JPEG corruption)
                    scaled[y2 * pitch + x2 * 4 + 3] = 0xFF;

                    for ( unsigned i = 0; i < opp; i++ )
                        scaled[y2 * pitch + x2 * 4 + i] = image->data.getUnsafe( ( y * size.x + x ) * opp + i );
                }

            SG_assert( scaled != nullptr )

            glGenTextures( 1, &texture );
            glBindTexture( GL_TEXTURE_2D, texture );

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );

            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width2, height2, 0, /*image->format*/GL_RGBA, GL_UNSIGNED_BYTE, scaled );
            driver->checkErrors( "OpenGlDriver.Texture.init" );

            bytesAlloc = width2 * height2 * 4;
            driver->gpuStats.bytesInTextures += bytesAlloc;

            Allocator<uint8_t>::release( scaled );
        }
        else if ( image->format == Image::Format::dxt1 )
        {
            SG_assert( glApi.functions.glCompressedTexImage2DARB != nullptr )
            SG_assert( driverShared.haveS3tc )

            glGenTextures( 1, &texture );
            glBindTexture( GL_TEXTURE_2D, texture );

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );

            glApi.functions.glCompressedTexImage2DARB( GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, size.x, size.y, 0, image->size.x * image->size.y * 8 / 16, image->data.getPtr() );
            driver->checkErrors( "OpenGlDriver.Texture.init" );

            bytesAlloc = image->size.x * image->size.y * 8 / 16;
            driver->gpuStats.bytesInTextures += bytesAlloc;
        }
        else
        {
            SG_assert( false )
        }
    }

    void Texture::init( SDL_Surface* surface, ILodFunction* lodFunction )
    {
        SG_assert( surface != nullptr )

        size.x = surface->w;
        size.y = surface->h;

        // Create and configure a texture handle

	    glGenTextures( 1, &texture );
        glBindTexture( GL_TEXTURE_2D, texture );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );

	    // Upload the data into the texture

	    if ( SDL_MUSTLOCK( surface ) )
            SDL_LockSurface( surface );

        unsigned width2 = size.x;
        unsigned height2 = size.y;

        // Legacy mode? Oh...
        if ( driverShared.requirePo2Textures )
        {
            width2 = ( width2 < driverShared.maxPo2Upscale ) ? nextPo2( width2 ) : prevPo2( width2 );
            height2 = ( height2 < driverShared.maxPo2Upscale ) ? nextPo2( height2 ) : prevPo2( height2 );
        }

        // Ignore LOD
        if ( lodFunction )
            delete lodFunction;

        // Limit texture dimensions
        width2 = minimum( width2, driverShared.maxTextureSize );
        height2 = minimum( height2, driverShared.maxTextureSize );

        // Oh...
        width2 = maximum<unsigned>( 2, width2 );
        height2 = maximum<unsigned>( 2, height2 );

        uint8_t* scaled = 0;
        unsigned pitch = width2 * 4;

        // Round to a 4-byte boundary (both SDL & OpenGL do that)
        pitch = ( ( pitch - 1 ) & ~0x03 ) + 4;

        scaled = Allocator<uint8_t>::allocate( height2 * pitch );

        printf( "[Font Texture: '%s'] %ux%u px; scaling to %ux%u.\n", name.c_str(), size.x, size.y, width2, height2 );

        for ( unsigned y2 = 0; y2 < height2; y2++ )
            for ( unsigned x2 = 0; x2 < width2; x2++ )
            {
                unsigned x = x2 * size.x / width2;
                unsigned y = size.y - y2 * size.y / height2 - 1;

                for ( unsigned i = 0; i < 4; i++ )
                    scaled[y2 * pitch + x2 * 4 + i] = ( ( uint8_t* ) surface->pixels )[y * surface->pitch + x * 4 + i];
            }

        SG_assert3( scaled != NULL, "OpenGlDriver.Texture.init" );

        /*SDL_Surface* tmp = SDL_CreateRGBSurfaceFrom( scaled, width2, height2, 32, pitch, surface->format->Rmask, surface->format->Gmask,
                surface->format->Bmask, surface->format->Amask );
        SDL_SaveBMP( tmp, ( String ) "tex_" + width + "x" + height + ".bmp" );
        SDL_FreeSurface( tmp );*/

        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width2, height2, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled );
        Allocator<uint8_t>::release( scaled );

        bytesAlloc = width2 * height2 * 4;
        driver->gpuStats.bytesInTextures += bytesAlloc;

        if ( SDL_MUSTLOCK( surface ) )
            SDL_UnlockSurface( surface );
    }
}
