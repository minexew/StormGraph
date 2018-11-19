
#include "Internal.hpp"

namespace StormGraph
{
    struct GlyphInfo
    {
        SDL_Surface* surface;
        unsigned x, baseY;
        int ix, iy, advance;
    };

    Font::Font( const char* fileName, int size, unsigned cpMin, unsigned cpMax ) : font( 0 ), fontTexture( 0 )
    {
        LARGE_INTEGER freq, begin, end;
        QueryPerformanceFrequency( &freq );

        if ( fileName )
            font = load( fileName, size );

        if ( !font )
            throw Exception( "StormGraph.Font", "Font", "FontLoadError",
                    ( String )"Failed to load `" + fileName + "`. The file probably either doesn't exist at all or wasn't recognized by FreeType." );

        lineSkip = TTF_FontLineSkip( font );

        if ( cpMax == 0 )
            return;

        QueryPerformanceCounter( &begin );

        // some temporary structures
        Array<GlyphInfo> gi( cpMax );

        unsigned totalWidth = 0, totalHeight = 0, x = 0, row = 0;
        static const SDL_Color white = { 255, 255, 255, 255 };

        unsigned rowSpacing = TTF_FontLineSkip( font ) + 2;
        unsigned rowAscent = TTF_FontAscent( font );

        // render the glyphs

        for ( unsigned i = 0; i < cpMax; i++ )
        {
            gi[i].surface = 0;

            if ( i < cpMin || !TTF_GlyphIsProvided( font, i ) )
                continue;

            SDL_Surface* glyph = TTF_RenderGlyph_Blended( font, i, white );

            if ( glyph )
            {
                int minX, maxX, minY, maxY, advance;

                TTF_GlyphMetrics( font, i, &minX, &maxX, &minY, &maxY, &advance );

                if ( x + advance + 2 > maxTextureSize )
                {
                    row++;
                    x = 0;
                }

                SDL_SetAlpha( glyph, 0, SDL_ALPHA_OPAQUE );

                gi[i].surface = glyph;
                gi[i].x = x + 1;
                gi[i].baseY = row * rowSpacing;
                gi[i].ix = minX;
                gi[i].iy = maxY;
                gi[i].advance = advance;

                x += advance + 2;

                if ( x > totalWidth )
                    totalWidth = x;
            }
        }

        row++;
        totalHeight = row * rowSpacing;

        if ( totalWidth == 0 || totalHeight == 0 )
            return;

        // build one final surface
        printf( " - %s@%i: font texture is %u x %u pixels\n", fileName, size, totalWidth, totalHeight );

        // will be released by Texture::Texture()
        SDL_Surface* fontSurface = SDL_CreateRGBSurface( SDL_SWSURFACE, totalWidth, totalHeight, 32,
                0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000 );
        //fontTexture = new Texture( totalWidth, totalHeight );

        for ( unsigned i = 0; i < cpMax; i++ )
        {
            Glyph glyph = {};

            if ( gi[i].surface )
            {
                SDL_Rect dest = { gi[i].x + gi[i].ix, gi[i].baseY + rowAscent - gi[i].iy };
                SDL_BlitSurface( gi[i].surface, 0, fontSurface, &dest );
                //fontTexture->blitSurface( gi[i].surface, gi[i].x + gi[i].ix, gi[i].baseY + rowAscent - gi[i].iy );
                SDL_FreeSurface( gi[i].surface );

                glyph.defined = true;
                glyph.u[0] = ( float )( gi[i].x ) / totalWidth;
                glyph.v[0] = ( float )( gi[i].baseY ) / totalHeight;
                glyph.u[1] = ( float )( gi[i].x + gi[i].advance ) / totalWidth;
                glyph.v[1] = ( float )( gi[i].baseY + lineSkip ) / totalHeight;
                glyph.width = ( float )( gi[i].advance ) / lineSkip;
            }
            else
                glyph.defined = false;

            glyphs.add( glyph );
        }

//SDL_SaveBMP( fontSurface, "font_2.bmp" );

        fontTexture = new Texture( fontSurface );
        heightCorrection = ( float )lineSkip / size;

        QueryPerformanceCounter( &end );
        printf( "generated in %g ms\nheight correction is %g\n\n", ( end.QuadPart - begin.QuadPart ) * 1000.0 / ( double )freq.QuadPart, heightCorrection );

        nominalSize = size;
    }

    Font::~Font()
    {
        TTF_CloseFont( font );
    }

    TTF_Font* Font::load( const char* fileName, int size )
    {
        SeekableInputStream* input = Engine::getInstance()->open( fileName );

        if ( !input )
            return 0;

        return TTF_OpenFontRW( getRwOps( input ), 1, size );
    }

    Texture* Font::render( const char* text, Colour colour )
    {
        SG_assert( font != NULL, "StormGraph.Font", "render" )

        if ( !text || !text[0] )
            return 0;

        SDL_Color c;
        c.r = colour.getR();
        c.g = colour.getG();
        c.b = colour.getB();

        return new Texture( TTF_RenderUTF8_Blended( font, text, c ) );
    }

    float Font::getCharWidth( Utf8Char c, float size )
    {
        if ( !glyphs[c].defined )
            return 0.0f;

        return glyphs[c].width * size * heightCorrection;
    }

    unsigned Font::getLineSkip()
    {
        return lineSkip;
    }

    float Font::getLineSkip( float size )
    {
        return lineSkip * size / nominalSize;
    }

    float Font::renderChar( float x, float y, Utf8Char c, float size, const Colour& colour )
    {
        if ( !fontTexture || !glyphs[c].defined )
            return 0.0f;

        size *= heightCorrection;
        fontTexture->render2D( x, y, glyphs[c].width * size, size, glyphs[c].u[0], glyphs[c].v[0], glyphs[c].u[1], glyphs[c].v[1], colour );

        return glyphs[c].width * size;
    }

    enum Escape
    {
        Escape_none,
        Escape_unk,
        Escape_colour,
        Escape_colour_r,
        Escape_colour_rg,
    };

    Font::TextDim Font::layoutString( float x0, float y0, const char* text, intptr_t numBytes, float size, Colour colour, bool render )
    {
        Utf8Char next;
        bool bold = false/*, shadow = false*/;

        static const float boldDist = 1.0f, shadowDist = 1.0f;

        if ( !text )
            return TextDim( 0, 0 );

        if ( numBytes < 0 )
            numBytes = strlen( text );

        Escape escape = Escape_none;
        float x = x0, y = y0;

        while ( numBytes > 0 )
        {
            unsigned numRead = Utf8::decode( next, text, numBytes );

            if ( !next || next == Utf8::invalidChar )
                break;

            text += numRead;
            numBytes -= numRead;

            switch ( escape )
            {
                case Escape_none:
                    if ( next == '\n' )
                    {
                        x = x0;
                        y += lineSkip * size / nominalSize;
                    }
                    else if ( next == '\\' )
                        escape = Escape_unk;
                    else
                    {
                        if ( render )
                        {
                            //if ( shadow )
                                renderChar( x + shadowDist, y + shadowDist, next, size, Colour() );

                            if ( bold )
                                renderChar( x + boldDist, y, next, size, colour );

                            x += renderChar( x, y, next, size, colour );
                        }
                        else
                            x += getCharWidth( next, size );
                    }
                    break;

                case Escape_unk:
                    if ( next == '#' )
                        escape = Escape_colour;
                    else
                    {
                        if ( next == '\\' )
                        {
                            if ( render )
                                x += renderChar( x, y, next, size, colour );
                            else
                                x += getCharWidth( next, size );
                        }
                        else if ( next >= '0' && next <= '9' )
                            colour = Colour::grey( 0.1f * ( next - '0' ) );
                        else if ( next == 'b' )
                            colour = Colour( 0.1f, 0.2f, 0.9f );
                        else if ( next == 'g' )
                            colour = Colour( 0.2f, 0.9f, 0.1f );
                        else if ( next == 'l' )
                            colour = Colour( 0.5f, 0.9f, 0.1f );
                        else if ( next == 'o' )
                            colour = Colour( 0.9f, 0.5f, 0.1f );
                        else if ( next == 'p' )
                            colour = Colour( 0.9f, 0.1f, 0.5f );
                        else if ( next == 'r' )
                            colour = Colour( 0.9f, 0.1f, 0.2f );
                        else if ( next == 's' )
                            colour = Colour( 0.1f, 0.5f, 0.9f );
                        else if ( next == 'w' )
                            colour = Colour( 1.0f, 1.0f, 1.0f );
                        else if ( next == 'y' )
                            colour = Colour( 0.9f, 0.9f, 0.1f );
                        else if ( next == 'B' )
                            bold = true;
                        //else if ( next == 'S' )
                        //    shadow = true;

                        escape = Escape_none;
                    }
                    break;

                case Escape_colour:
                    if ( render )
                        colour.r = ( next >= '0' && next <= '9' ) ? ( next - '0' ) / 9.0f : 0.0f;

                    escape = Escape_colour_r;
                    break;

                case Escape_colour_r:
                    if ( render )
                        colour.g = ( next >= '0' && next <= '9' ) ? ( next - '0' ) / 9.0f : 0.0f;

                    escape = Escape_colour_rg;
                    break;

                case Escape_colour_rg:
                    if ( render )
                        colour.b = ( next >= '0' && next <= '9' ) ? ( next - '0' ) / 9.0f : 0.0f;

                    escape = Escape_none;
                    break;
            }
        }

        return TextDim( ( unsigned )ceil( x - x0 ), ( unsigned )ceil( y + lineSkip - y0 ) );
    }

#if 0
    Font::TextDim Font::wrapString( float x0, float y0, const char* text, intptr_t numBytes, float size, Colour colour, bool render, unsigned width )
    {
        Utf8Char next;
        bool bold = false

        static const float boldDist = 1.0f, shadowDist = 1.0f;

        if ( !text )
            return TextDim( 0, 0 );

        if ( numBytes < 0 )
            numBytes = strlen( text );

        Escape escape = Escape_none;
        float x = x0, y = y0, maxX = x0 + width;

        String word;

        while ( numBytes > 0 )
        {
            unsigned numRead = Utf8::decode( next, text, numBytes );

            if ( !next || next == Utf8::invalidChar )
                break;

            text += numRead;
            numBytes -= numRead;

            if ( Utf8::isAlphaNumeric( next ) )
            {
                word += next;
                wordLength += layoutChar( x, y, next, size, colour, false );
            }
            else
            {
                if ( !text.isEmpty() )
                {
                    if ( x + wordLength > maxX )
                    {
                        if ( wordLength > maxX )
                        {
                        }
                    }
                }
                else
                    x += layoutChar( x, y, next, size, colour, render );
            }
        }

        return TextDim( ( unsigned )ceil( x - x0 ), ( unsigned )ceil( y + lineSkip - y0 ) );
    }
#endif

    Font::TextDim Font::render( float x0, float y0, const char* text, intptr_t numBytes, float size, Colour colour, unsigned align )
    {
        if ( align != 0 )
        {
            TextDim dimensions = layoutString( x0, y0, text, numBytes, size, colour, false );

            if ( align & Align::centered )
                x0 -= dimensions.w / 2;
            else if ( align & Align::right )
                x0 -= dimensions.w;

            if ( align & Align::middle )
                y0 -= dimensions.h / 2;
            else if ( align & Align::bottom )
                y0 -= dimensions.h;
        }

        return layoutString( x0, y0, text, numBytes, size, colour, true );
    }

    Font::TextDim Font::render( float x, float y, const String& text, float size, Colour colour, unsigned align )
    {
        return render( x, y, text, text.getNumBytes(), size, colour, align );
    }

    Font::TextDim Font::render( float x, float y, const String& text, Colour colour, unsigned align )
    {
        return render( x, y, text, text.getNumBytes(), ( float )getNominalSize(), colour, align );
    }

    void Font::render( float x, float y, const Text& text )
    {
        layoutString( x + text.x, y + text.y, text.string, text.string.getNumBytes(), text.size, text.colour, true );
    }

    Text Font::layout( const String& text, float size, Colour colour, unsigned align )
    {
        TextDim dimensions = layoutString( 0.0f, 0.0f, text, text.getNumBytes(), size, colour, false );
        Text laidOut = { dimensions.w, dimensions.h, colour, size, text, 0.0f, 0.0f };

        if ( align != 0 )
        {
            if ( align & Align::centered )
                laidOut.x -= dimensions.w / 2;
            else if ( align & Align::right )
                laidOut.x -= dimensions.w;

            if ( align & Align::middle )
                laidOut.y -= dimensions.h / 2;
            else if ( align & Align::bottom )
                laidOut.y -= dimensions.h;
        }

        return laidOut;
    }

    Text Font::layout( const String& text, Colour colour, unsigned align )
    {
        return layout( text, ( float )getNominalSize(), colour, align );
    }
}
