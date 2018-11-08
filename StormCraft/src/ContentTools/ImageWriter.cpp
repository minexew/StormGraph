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

#include <StormGraph/IO/ImageWriter.hpp>

#ifdef WITH_DXT
#include <squish.h>
#endif

namespace StormGraph
{
    /*static void ()
    {
    }*/

    void ImageWriter::save( const Image* image, OutputStream* output, Image::StorageFormat format )
    {
        if ( image->format == Image::Format::rgb && ( /*format == Image::StorageFormat::rgb || */format == Image::StorageFormat::original ) )
        {
            output->writeString( "Sg_Raw#u8" );

            output->write<uint32_t>( image->size.x );
            output->write<uint32_t>( image->size.y );
            output->write<uint8_t>( 3 );

            output->write( image->data.getPtrUnsafe(), image->size.x * image->size.y * 3 );
        }
#ifdef WITH_DXT
        else if ( image->format == Image::Format::rgb && format == Image::StorageFormat::ddsDxt1 )
        {
            SG_assert( image->size.x % 4 == 0 )
            SG_assert( image->size.y % 4 == 0 )

            output->writeString( "Sg_Dxt1#0" );
            output->write<uint16_t>( image->size.x / 4 );
            output->write<uint16_t>( image->size.y / 4 );

            for ( unsigned y = 0; y < image->size.y; y += 4 )
                for ( unsigned x = 0; x < image->size.x; x += 4 )
                {
                    squish::u8 rgba[64];
                    uint8_t block[8];

                    size_t srcIndex = ( y * image->size.y + x ) * 3;

                    for ( int yy = 0; yy < 4; yy++ )
                    {
                        for ( int xx = 0; xx < 4; xx++ )
                        {
                            rgba[( yy * 4 + xx ) * 4] = image->data[srcIndex++];
                            rgba[( yy * 4 + xx ) * 4 + 1] = image->data[srcIndex++];
                            rgba[( yy * 4 + xx ) * 4 + 2] = image->data[srcIndex++];
                            rgba[( yy * 4 + xx ) * 4 + 3] = 0xFF;
                        }

                        srcIndex += image->size.y * 3 - 12;
                    }

                    squish::Compress( rgba, block, squish::kDxt1 );
                    output->write( block, 8 );
                }
        }
#endif
        else
            throw Exception( "StormGraph.ImageWriter.save", "UnsupportedFormat", "Unsupported format combination." );
    }
}
