
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_STROKER_H
#include FT_GLYPH_H
#include FT_TRUETYPE_IDS_H

#define TTF_STYLE_NORMAL	0x00
#define TTF_STYLE_BOLD		0x01
#define TTF_STYLE_ITALIC	0x02


using namespace StormGraph;

/* Cached glyph information */
struct FtGlyph
{
	FT_Bitmap pixmap;
	int minx, maxx, miny, maxy, yoffset, advance;
};

/* The structure used to hold internal font information */
struct FtFont
{
	/* Freetype2 maintains all sorts of useful info itself */
	FT_Face face;

	/* We'll cache these ourselves */
	int height;
	int ascent;
	int descent;
	int lineskip;

	/* The font style */
	int face_style;
	int style;

	/* Extra width in glyph bounds for text styles */
	int glyph_overhang;
	float glyph_italics;

	/* We are responsible for closing the font stream */
	Reference<SeekableInputStream> input;
	FT_Open_Args args;

	/* For non-scalable formats, we must remember which font index size */
	int font_size_family;

	/* really just flags passed into FT_Load_Glyph */
	int hinting;
};

static FT_Library library;
static bool ftIsInit = false;

#define FxP_FLOOR( x_ )     ( ( ( x_ ) & -64 ) / 64 )
#define FxP_CEIL( x_ )      ( ( ( ( x_ ) + 63 ) & -64 ) / 64 )

static bool initFreeType()
{
    FT_Error error = FT_Init_FreeType( &library );

    if ( error )
    {
        printf( "Couldn't init FreeType engine (%i)\n", error );
        return false;
    }

    ftIsInit = true;
    return true;
}

static void exitFreeType()
{
    if ( ftIsInit )
    {
        FT_Done_FreeType( library );
        ftIsInit = false;
    }
}

static unsigned long readFunc( FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count )
{
    SeekableInputStream* input = reinterpret_cast<SeekableInputStream*>( stream->descriptor.pointer );

    input->setPos( offset );
    return input->read( buffer, count );
}

FtFont* openFont( SeekableInputStream* input, int ptsize, unsigned style )
{
	FT_Error error;
	FT_Fixed scale;
	int64_t position;

	if ( !ftIsInit )
		if ( !initFreeType() )
		    return nullptr;

    position = input->getPos();

	Object<FtFont> font = new FtFont;
    memset( font, 0, sizeof( FtFont ) );
	SG_assert( font != nullptr )

	font->input = input;

    FT_Stream stream = reinterpret_cast<FT_Stream>( Allocator<uint8_t>::allocate( sizeof( *stream ) ) );
    SG_assert( stream != nullptr )

	stream->read = readFunc;
	stream->descriptor.pointer = input;
	stream->pos = ( unsigned long ) position;
	stream->size = ( unsigned long )( input->getSize() - position);

	font->args.flags = FT_OPEN_STREAM;
	font->args.stream = stream;

	error = FT_Open_Face( library, &font->args, 0, &font->face );

	if ( error )
		return nullptr;

    SG_assert( FT_IS_SCALABLE( font->face ) )

    error = FT_Set_Char_Size( font->face, 0, ptsize * 64, 0, 0 );

    if ( error )
        return nullptr;

    scale = font->face->size->metrics.y_scale;
    font->ascent = FxP_CEIL( FT_MulFix( font->face->ascender, scale ) );
    font->descent = FxP_CEIL( FT_MulFix( font->face->descender, scale ) );
    font->height = font->ascent - font->descent + 1;
    font->lineskip = FxP_CEIL( FT_MulFix( font->face->height, scale ) );

	/* Initialize the font face style */
	font->face_style = TTF_STYLE_NORMAL;

	if ( font->face->style_flags & FT_STYLE_FLAG_BOLD )
		font->face_style |= TTF_STYLE_BOLD;

	if ( font->face->style_flags & FT_STYLE_FLAG_ITALIC )
		font->face_style |= TTF_STYLE_ITALIC;

	font->style = font->face_style;

	if ( style & IFont::bold )
	    font->style |= TTF_STYLE_BOLD;

	if ( style & IFont::italic )
	    font->style |= TTF_STYLE_ITALIC;

	font->glyph_overhang = font->face->size->metrics.y_ppem / 10;
	font->glyph_italics = 0.207f;
	font->glyph_italics *= font->height;

	return font.detach();
}

static void releaseGlyph( FtGlyph* glyph )
{
	if ( glyph->pixmap.buffer )
	{
		free( glyph->pixmap.buffer );
		glyph->pixmap.buffer = nullptr;
	}
}

/* Handle a style only if the font does not already handle it */
#define TTF_HANDLE_STYLE_BOLD(font) (((font)->style & TTF_STYLE_BOLD) && !((font)->face_style & TTF_STYLE_BOLD))
#define TTF_HANDLE_STYLE_ITALIC(font) (((font)->style & TTF_STYLE_ITALIC) && !((font)->face_style & TTF_STYLE_ITALIC))
//#define TTF_HANDLE_STYLE_UNDERLINE(font) ((font)->style & TTF_STYLE_UNDERLINE)
//#define TTF_HANDLE_STYLE_STRIKETHROUGH(font) ((font)->style & TTF_STYLE_STRIKETHROUGH)

static FT_Error loadGlyph( FtFont* font, Unicode::Char c, FtGlyph* cached )
{
	FT_Error error;
	FT_GlyphSlot glyph;
	FT_Glyph_Metrics* metrics;
	FT_Outline* outline;

	if ( !font || !font->face )
		return FT_Err_Invalid_Handle;

	/* Load the glyph */
    FT_UInt index = FT_Get_Char_Index( font->face, c );

    error = FT_Load_Glyph( font->face, index, FT_LOAD_DEFAULT | font->hinting );

	if ( error )
		return error;

	/* Get our glyph shortcuts */
	glyph = font->face->glyph;
	metrics = &glyph->metrics;
	outline = &glyph->outline;

	/* Get the glyph metrics if desired */
    /* Get the bounding box */
    cached->minx = FxP_FLOOR(metrics->horiBearingX);
    cached->maxx = cached->minx + FxP_CEIL(metrics->width);
    cached->maxy = FxP_FLOOR(metrics->horiBearingY);
    cached->miny = cached->maxy - FxP_CEIL(metrics->height);
    cached->yoffset = font->ascent - cached->maxy;
    cached->advance = FxP_CEIL(metrics->horiAdvance);

    /* Adjust for bold and italic text */
    if ( TTF_HANDLE_STYLE_BOLD( font ) )
        cached->maxx += font->glyph_overhang;

    if ( TTF_HANDLE_STYLE_ITALIC( font ) )
        cached->maxx += ( int ) ceil( font->glyph_italics );

    int i;
    FT_Bitmap* src, * dst;

    /* Handle the italic style */
    if ( TTF_HANDLE_STYLE_ITALIC( font ) )
    {
        FT_Matrix shear;

        shear.xx = 1 << 16;
        shear.xy = ( int ) ( font->glyph_italics * ( 1 << 16 ) ) / font->height;
        shear.yx = 0;
        shear.yy = 1 << 16;

        FT_Outline_Transform( outline, &shear );
    }

    /* Render the glyph */
    error = FT_Render_Glyph( glyph, ft_render_mode_normal );

    if ( error )
        return error;

    src = &glyph->bitmap;

    SG_assert( src->pixel_mode == FT_PIXEL_MODE_GRAY )

    /* Copy over information to cache */
    dst = &cached->pixmap;

    memcpy( dst, src, sizeof( *dst ) );

    /* Adjust for bold and italic text */
    if ( TTF_HANDLE_STYLE_BOLD(font) )
    {
        int bump = font->glyph_overhang;
        dst->pitch += bump;
        dst->width += bump;
    }

    if ( TTF_HANDLE_STYLE_ITALIC(font) )
    {
        int bump = ( int ) ceil( font->glyph_italics );
        dst->pitch += bump;
        dst->width += bump;
    }

    if ( dst->rows != 0 )
    {
        dst->buffer = ( uint8_t* )malloc( dst->pitch * dst->rows );

        if ( !dst->buffer )
            return FT_Err_Out_Of_Memory;

        memset( dst->buffer, 0, dst->pitch * dst->rows );

        for ( i = 0; i < src->rows; i++ )
            memcpy( dst->buffer+i * dst->pitch, src->buffer+i * src->pitch, src->pitch );
    }

    /* Handle the bold style */
    if ( TTF_HANDLE_STYLE_BOLD( font ) )
    {
        uint8_t* pixmap;

        for ( int row = dst->rows - 1; row >= 0; --row )
        {
            pixmap = ( uint8_t* ) dst->buffer + row * dst->pitch;

            for ( int offset = 1; offset <= font->glyph_overhang; offset++ )
            {
                for ( int col = dst->width - 1; col > 0; --col )
                    pixmap[col] = ( uint8_t ) minimum( pixmap[col] + pixmap[col - 1], 0xFF );
            }
        }
    }

	return 0;
}

bool renderGlyph( FtFont* font, Unicode::Char c, Vector<unsigned>& size, uint8_t*& buffer, int* minx, int* maxx, int* miny, int* maxy, int* advance )
{
    FtGlyph glyph;

    FT_Error error = loadGlyph( font, c, &glyph );

    if ( error )
        return false;

    if ( minx )
		*minx = glyph.minx;

	if ( maxx )
	{
		*maxx = glyph.maxx;

		if ( TTF_HANDLE_STYLE_BOLD( font ) )
			*maxx += font->glyph_overhang;

        //if ( TTF_HANDLE_STYLE_ITALIC( font ) )
        //    *maxx += font->glyph_italics;
	}
	if ( miny )
		*miny = glyph.miny;

	if ( maxy )
		*maxy = glyph.maxy;

	if ( advance )
	{
		*advance = glyph.advance;

		if ( TTF_HANDLE_STYLE_BOLD( font ) )
			*advance += font->glyph_overhang;
	}

    size.x = glyph.pixmap.width;
    size.y = glyph.pixmap.rows;

	buffer = Allocator<uint8_t>::allocate( size.x * size.y * 4 );

	if ( buffer == nullptr )
	{
	    releaseGlyph( &glyph );
	    return false;
	}

	for ( unsigned y = 0; y < size.y; y++ )
	{
		const uint8_t* src = glyph.pixmap.buffer + y * glyph.pixmap.pitch;
		uint32_t* dst = ( uint32_t* ) buffer + y * size.x;

		for ( unsigned x = 0; x < size.x; x++ )
			*dst++ = 0xFFFFFF | ( ( *src++ ) << 24 );
	}

    releaseGlyph( &glyph );
	return true;
}

void closeFont( FtFont*& font )
{
    if ( font )
    {
        if ( font->face )
    	    FT_Done_Face( font->face );

        if ( font->args.stream )
    	    free( font->args.stream );

        delete font;
        font = nullptr;
    }
}