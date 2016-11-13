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

#include <StormGraph/Image.hpp>

#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#ifdef _WIN32
// On Windows, system headers define boolean
#define HAVE_BOOLEAN
#endif

#include <jpeglib.h>

#include <lodepng.h>

#define INPUT_BUFFER_SIZE	4096

namespace StormGraph
{
    class ImageLoader : public IImageLoader
    {
        public:
            virtual Image* load( SeekableInputStream* input, bool required = true );
    };

    static bool isJpg( SeekableInputStream* input )
    {
        uint8_t magic[2] = { 0, 0 };

        intptr_t read = input->read( magic, 2 );
        input->seek( -read );

        return magic[0] == 0xFF && magic[1] == 0xD8;
    }

    static bool isPng( SeekableInputStream* input )
    {
	    uint8_t magic[4] = { 0, 0, 0, 0 };

	    intptr_t read = input->read( magic, 4 );
        input->seek( -read );

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

            //image->format = GL_BGRA;
            image->format = Image::Format::bgra;
        }
        else
        {
            cinfo.out_color_space = JCS_RGB;
            cinfo.quantize_colors = FALSE;
            jpeg_calc_output_dimensions( &cinfo );

            //image->format = GL_RGBA;
            image->format = Image::Format::rgba;
        }

        image->size = Vector<unsigned>( cinfo.output_width, cinfo.output_height, cinfo.num_components );
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

    static Image* loadPng( SeekableInputStream* input )
    {
		const size_t inputLength = input->getSize();

		std::vector<uint8_t> buffer;
		buffer.resize( inputLength );

		if ( input->read( &buffer[0], inputLength ) != inputLength)
			return false;

		std::vector<uint8_t> pixels;
		unsigned width, height;

		if ( lodepng::decode( pixels, width, height, &buffer[0], inputLength, LCT_RGBA ) != 0 )
			return false;

        Image* image = new Image;
        //image->format = GL_RGBA;
        image->format = Image::Format::rgba;
        image->size = Vector<unsigned>( width, height, 4 );
        image->data.resize( height * width * 4 );
		memcpy( &image->data[0], &pixels[0], height * width * 4 );
        return image;
    }

    Image* ImageLoader::load( SeekableInputStream* input, bool required )
    {
        SG_assert( input != nullptr )

        if ( isJpg( input ) )
            return loadJpg( input );

        if ( isPng( input ) )
            return loadPng( input );

        String id = input->readString();

        if ( id == "Sg_Raw#u8" )
        {
            Object<Image> image = new Image;

            image->format = Image::Format::rgb;
            image->size.x = input->read<uint32_t>();
            image->size.y = input->read<uint32_t>();
            image->size.z = input->read<uint8_t>();
            image->data.resize( image->size.x * image->size.y * image->size.z );

            input->read( image->data.getPtr(), image->size.x * image->size.y * image->size.z );

            return image.detach();
        }
        else if ( id == "Sg_Dxt1#0" )
        {
            Object<Image> image = new Image;

            image->format = Image::Format::dxt1;
            image->size.x = input->read<uint16_t>() * 4;
            image->size.y = input->read<uint16_t>() * 4;
            image->data.resize( image->size.x * image->size.y * 8 / 16 );

            input->read( image->data.getPtr(), image->size.x * image->size.y * 8 / 16 );

            return image.detach();
        }

        return nullptr;
    }

    IImageLoader* createImageLoader( IEngine* engine )
    {
        return new ImageLoader();
    }
}
