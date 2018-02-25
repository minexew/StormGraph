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
#if 0
#include "OpenGlDriver.hpp"

#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#define XMD_H
#include <jpeglib.h>

#include <png.h>

#define INPUT_BUFFER_SIZE	4096

namespace OpenGlDriver
{
    static bool isJpg( SeekableInputStream* input )
    {
        uint8_t magic[2];

        if ( !input || !input->read( magic, 2 ) )
            return false;

        input->seek( -2 );

        return magic[0] == 0xFF && magic[1] == 0xD8;
    }

    static bool isPng( SeekableInputStream* input )
    {
	    uint8_t magic[4];

	    if ( !input || !input->read( magic, 4 ) )
            return false;

        input->seek( -4 );

        return magic[0] == 0x89 && magic[1] == 'P' && magic[2] == 'N' && magic[3] == 'G';
    }

    struct JpegLoadingState
    {
        struct jpeg_source_mgr pub;

        SeekableInputStream* input;
        uint8_t buffer[INPUT_BUFFER_SIZE];
    };

    static void init_source( j_decompress_ptr cinfo )
    {
    	return;
    }

    static boolean fill_input_buffer( j_decompress_ptr cinfo )
    {
    	JpegLoadingState* state = ( JpegLoadingState* ) cinfo->src;

    	size_t numRead = state->input->read( state->buffer, INPUT_BUFFER_SIZE );

        if ( numRead == 0 )
        {
            state->buffer[0] = ( uint8_t ) 0xFF;
            state->buffer[1] = ( uint8_t ) JPEG_EOI;
            numRead = 2;
        }

        state->pub.next_input_byte = ( const JOCTET* ) state->buffer;
        state->pub.bytes_in_buffer = numRead;

        return TRUE;
    }

    static void skip_input_data( j_decompress_ptr cinfo, long numBytes )
    {
        JpegLoadingState* state = ( JpegLoadingState* ) cinfo->src;

        if ( numBytes > 0 )
        {
            while ( numBytes > ( long ) state->pub.bytes_in_buffer )
            {
                numBytes -= ( long ) state->pub.bytes_in_buffer;
                state->pub.fill_input_buffer( cinfo );
            }

            state->pub.next_input_byte += ( size_t ) numBytes;
            state->pub.bytes_in_buffer -= ( size_t ) numBytes;
        }
    }

    static void term_source( j_decompress_ptr cinfo )
    {
        return;
    }

    static void jpeg_SDL_RW_src( j_decompress_ptr cinfo, SeekableInputStream* input )
    {
        JpegLoadingState *state;

        if ( !cinfo->src )
            cinfo->src = ( struct jpeg_source_mgr* )( *cinfo->mem->alloc_small )( ( j_common_ptr ) cinfo, JPOOL_PERMANENT, sizeof( JpegLoadingState ) );

        state = ( JpegLoadingState* ) cinfo->src;
        state->pub.init_source = init_source;
        state->pub.fill_input_buffer = fill_input_buffer;
        state->pub.skip_input_data = skip_input_data;
        state->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
        state->pub.term_source = term_source;
        state->input = input;
        state->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
        state->pub.next_input_byte = 0; /* until buffer loaded */
    }

    struct my_error_mgr
    {
        jpeg_error_mgr errmgr;
        jmp_buf escape;
    };

    static void my_error_exit( j_common_ptr cinfo )
    {
        struct my_error_mgr *err = ( my_error_mgr* )cinfo->err;
        longjmp( err->escape, 1 );
    }

    static void output_no_message( j_common_ptr cinfo )
    {
    }

    static Image* loadJpg( SeekableInputStream* input )
    {
        Image* volatile image = 0;

        if ( !isReadable( input ) )
            return 0;

        /* Create a decompression structure and load the JPEG header */
        jpeg_decompress_struct cinfo;
        my_error_mgr jerr;

        cinfo.err = jpeg_std_error( &jerr.errmgr );

        jerr.errmgr.error_exit = my_error_exit;
        jerr.errmgr.output_message = output_no_message;

        if ( setjmp( jerr.escape ) )
        {
            /* If we get here, libjpeg found an error */
            jpeg_destroy_decompress( &cinfo );

            if ( image )
                delete image;

            return 0;
        }

        jpeg_create_decompress( &cinfo );
        jpeg_SDL_RW_src( &cinfo, input );
        jpeg_read_header( &cinfo, TRUE );

        image = new Image;

        if ( cinfo.num_components == 4 )
        {
            cinfo.out_color_space = JCS_CMYK;
            cinfo.quantize_colors = FALSE;
            jpeg_calc_output_dimensions( &cinfo );

            image->format = GL_BGRA;
        }
        else
        {
            cinfo.out_color_space = JCS_RGB;
            cinfo.quantize_colors = FALSE;
            jpeg_calc_output_dimensions( &cinfo );

            image->format = GL_RGBA;
        }

        image->dimensions = Vector<unsigned>( cinfo.output_width, cinfo.output_height, cinfo.num_components );
        image->data.resize( cinfo.output_height * cinfo.output_width * cinfo.num_components );

        JSAMPROW rowptr[1];

        jpeg_start_decompress( &cinfo );

        while ( cinfo.output_scanline < cinfo.output_height )
        {
            rowptr[0] = ( JSAMPROW ) image->data.getPtr( cinfo.output_scanline * cinfo.output_width * cinfo.num_components );
            jpeg_read_scanlines( &cinfo, rowptr, ( JDIMENSION ) 1 );
        }

        jpeg_finish_decompress( &cinfo );
        jpeg_destroy_decompress( &cinfo );

        return image;
    }

    static void png_read_data( png_structp ctx, png_bytep area, png_size_t size )
    {
	    SeekableInputStream* input = ( SeekableInputStream* ) png_get_io_ptr( ctx );

	    input->read( area, size );
    }

    static Image* loadPng( SeekableInputStream* input )
    {
	    Image* volatile image = 0;
        png_structp png_ptr = 0;
        png_infop info_ptr = 0;

        png_uint_32 width, height;
        int bit_depth, color_type, interlace_type;
        unsigned numChannels;

        png_bytep* volatile row_pointers = 0;

        if ( !input )
            return 0;

	    png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, 0, 0, 0 );

        if ( !png_ptr )
		    goto error;

	    info_ptr = png_create_info_struct( png_ptr );

	    if ( !info_ptr )
		    goto error;

        if ( setjmp( *png_set_longjmp_fn( png_ptr, longjmp, sizeof( jmp_buf ) ) ) )
            goto error;

	    png_set_read_fn( png_ptr, input, png_read_data );

	    png_read_info( png_ptr, info_ptr );
        png_get_IHDR( png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, 0, 0 );

	    png_set_strip_16( png_ptr );
	    png_set_packing( png_ptr );

	    if ( color_type == PNG_COLOR_TYPE_GRAY )
    		png_set_expand( png_ptr );

	    if ( color_type == PNG_COLOR_TYPE_GRAY_ALPHA )
    		png_set_gray_to_rgb( png_ptr );

	    png_read_update_info( png_ptr, info_ptr );
	    png_get_IHDR( png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, 0, 0 );

        numChannels = png_get_channels( png_ptr, info_ptr );

        if ( color_type == PNG_COLOR_TYPE_PALETTE || !( numChannels == 3 || numChannels == 4 ) )
            goto error;

        image = new Image;
        image->format = GL_RGBA;
        image->dimensions = Vector<unsigned>( width, height, numChannels );
        image->data.resize( height * width * numChannels );

	    row_pointers = ( png_bytep* ) malloc( sizeof( png_bytep ) * height );

	    if ( !row_pointers )
		    goto error;

	    for ( unsigned row = 0; row < height; row++ )
	        row_pointers[row] = ( png_bytep ) image->data.getPtr( row * width * numChannels );

	    png_read_image( png_ptr, row_pointers );
	    png_read_end( png_ptr, info_ptr );

        goto done;

error:
        if ( image )
        {
            delete image;
            image = 0;
        }

done:
        if ( png_ptr )
            png_destroy_read_struct( &png_ptr, info_ptr ? &info_ptr : ( png_infopp ) 0, ( png_infopp ) 0 );

        if ( row_pointers )
            free( row_pointers );

        return image;
    }

    Image* ImageLoader::load( SeekableInputStream* input )
    {
        if ( isJpg( input ) )
            return loadJpg( input );

        if ( isPng( input ) )
            return loadPng( input );

        String id = input->readString();

        if ( id == "Sg_Raw#u8" )
        {
            Object<Image> image = new Image;

            image->format = GL_RGB;
            image->dimensions.x = input->read<uint32_t>();
            image->dimensions.y = input->read<uint32_t>();
            image->dimensions.z = input->read<uint8_t>();
            image->data.resize( image->dimensions.x * image->dimensions.y * image->dimensions.z );

            input->read( image->data.getPtr(), image->dimensions.x * image->dimensions.y * image->dimensions.z );

            return image.detach();
        }

        return nullptr;
    }
}
#endif
