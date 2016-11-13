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

#ifndef Use_Sdl_Ttf
#include "FreeType.hpp"
#endif

namespace OpenGlDriver
{
    static const unsigned cpMin = 32, cpMax = 256;

    enum Escape
    {
        Escape_none,
        Escape_unk,
        Escape_colour,
        Escape_colour_r,
        Escape_colour_rg,
    };

    struct GlyphInfo
    {
        SDL_Surface* surface;

        unsigned x, baseY;
        int ix, iy, advance, maxX, offset;
    };

    class FontLodFunction : public ILodFunction
    {
        public:
            virtual ~FontLodFunction() {}

            virtual unsigned getLod( unsigned lod ) { return 0; }
    };

    Font::Font( OpenGlDriver* driver, const char* name, SeekableInputStream* input, unsigned size, unsigned style )
            : driver( driver ), name( name ), size( size ), style( style ), plane( 0 ), material( 0 )
    {
        SG_assert3( input != nullptr, "OpenGlDriver.Font.Font" )

        LARGE_INTEGER freq, begin, end;
        QueryPerformanceFrequency( &freq );

#ifdef Use_Sdl_Ttf
        font = TTF_OpenFontRW( getRwOps( input ), 1, size );
#else
        font = openFont( input, size, style );
#endif

        if ( !font )
            throw StormGraph::Exception( "OpenGlDriver.Font.Font", "FontLoadError", ( String )"Failed to parse font `" + name + "`." );

#ifdef Use_Sdl_Ttf
        lineSkip = TTF_FontLineSkip( font );
#else
        lineSkip = font->lineskip;
#endif

        //printf( "DBG: lineskip %u\n", lineSkip );

        QueryPerformanceCounter( &begin );

        // some temporary structures
        Array<GlyphInfo> gi( cpMax );

        unsigned totalWidth = 0, totalHeight = 0, x = 0, row = 0;
#ifdef Use_Sdl_Ttf
        static const SDL_Color white = { 255, 255, 255, 255 };
#endif

        unsigned rowSpacing = lineSkip + 2;
#ifdef Use_Sdl_Ttf
        unsigned rowAscent = TTF_FontAscent( font );
#else
        unsigned rowAscent = font->ascent;
#endif

        // render the glyphs

        for ( unsigned i = 0; i < cpMax; i++ )
        {
            gi[i].surface = 0;

#ifdef Use_Sdl_Ttf
            if ( i < cpMin || !TTF_GlyphIsProvided( font, i ) )
#else
            if ( i < cpMin || !FT_Get_Char_Index( font->face, i ) )
#endif
                continue;

#ifdef Use_Sdl_Ttf
            SDL_Surface* glyph = TTF_RenderGlyph_Blended( font, i, white );
#else
            uint8_t* buffer = nullptr;
            Vector<unsigned> size;

            int minX, maxX, minY, maxY, advance;

            if ( !renderGlyph( font, i, size, buffer, &minX, &maxX, &minY, &maxY, &advance ) )
                continue;

            //SDL_Surface* glyph = SDL_CreateRGBSurfaceFrom( buffer, size.x, size.y, 32, size.x * 4,
            //        0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000 );

            /*SDL_Surface* glyph = SDL_CreateRGBSurface( SDL_SWSURFACE, 0, 0, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        
            glyph->pixels = buffer;
            glyph->w = size.x;
            glyph->h = size.y;
            glyph->pitch = size.x * 4;
            SDL_SetClipRect( glyph, NULL );*/

            SDL_Surface* glyph = SDL_CreateRGBSurface( SDL_SWSURFACE, size.x, size.y, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000 );
            
            SDL_LockSurface( glyph );
            memcpy( glyph->pixels, buffer, size.x * size.y * 4 );
            SDL_UnlockSurface( glyph );

            free( buffer );
#endif

            if ( glyph )
            {
#ifdef Use_Sdl_Ttf
                int minX, maxX, minY, maxY, advance;

                TTF_GlyphMetrics( font, i, &minX, &maxX, &minY, &maxY, &advance );
#endif

                if ( x + maxX + 2 > driverShared.maxTextureSize )
                {
                    row++;
                    x = 0;
                }

                SDL_SetAlpha( glyph, 0, SDL_ALPHA_OPAQUE );

                if ( maxY - minY > ( int ) rowSpacing )
                    rowSpacing = maxY - minY;

                gi[i].surface = glyph;
                gi[i].x = x + 1;
                gi[i].baseY = row * rowSpacing;
                gi[i].ix = minX;
                gi[i].iy = maxY - minY;
                gi[i].advance = advance;
                gi[i].maxX = maxX;
                gi[i].offset = minY;

                x += maxX + 2;

                if ( x > totalWidth )
                    totalWidth = x;
            }
        }

        row++;
        totalHeight = row * rowSpacing;

        if ( totalWidth == 0 || totalHeight == 0 )
            return;

        // build one final surface
        //printf( " - %s@%i: font texture is %u x %u pixels\n", name, size, totalWidth, totalHeight );

        SDL_Surface* fontSurface = SDL_CreateRGBSurface( SDL_SWSURFACE, totalWidth, totalHeight, 32,
                0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000 );

        for ( unsigned i = 0; i < cpMax; i++ )
        {
            Glyph glyph = {};

            if ( gi[i].surface )
            {
                SDL_Rect dest = { ( int16_t )( gi[i].x + gi[i].ix ), ( int16_t )( gi[i].baseY + rowAscent - gi[i].iy ) };
                SDL_BlitSurface( gi[i].surface, 0, fontSurface, &dest );
                SDL_FreeSurface( gi[i].surface );

                glyph.defined = true;
                glyph.u[0] = ( float )( gi[i].x ) / totalWidth;
                glyph.v[0] = ( float )( gi[i].baseY ) / totalHeight;
                glyph.u[1] = ( float )( gi[i].x + gi[i].maxX ) / totalWidth;
                glyph.v[1] = ( float )( gi[i].baseY + lineSkip ) / totalHeight;
                glyph.width = ( float )( gi[i].maxX ) / lineSkip;
                glyph.advance = ( float )( gi[i].advance ) / lineSkip;
                glyph.offset = ( float ) gi[i].offset;
            }
            else
                glyph.defined = false;

            glyphs.add( glyph );
        }

//SDL_SaveBMP( fontSurface, "font_2.bmp" );

        Reference<Texture> texture = new Texture( driver, ( String ) name + ".glyphs", fontSurface, new FontLodFunction() );
        SDL_FreeSurface( fontSurface );

        MaterialProperties2 materialProperties;
        memset( &materialProperties, 0, sizeof( materialProperties ) );

        materialProperties.colour = Colour::white();
        materialProperties.numTextures = 1;
        materialProperties.textures[0] = texture.detach();

        material = new Material( driver, ( String ) name + ".material", &materialProperties, true );

        PlaneCreationInfo planeInfo( Vector2<>( 1.0f, 1.0f ), Vector<>(), Vector2<>(), Vector2<>( 1.0f, 1.0f ), false, true, material->reference() );
        plane = Mesh::createPlane( driver, &planeInfo, IModel::fullStatic );

        heightCorrection = ( float ) lineSkip / size;

        QueryPerformanceCounter( &end );

        //printf( "OpenGlDriver: generated texture for `%s`\n\tin %g ms (height correction factor is %g)\n\n", name,
        //        ( end.QuadPart - begin.QuadPart ) * 1000.0 / ( double )freq.QuadPart, heightCorrection );

        Common::logEvent( "OpenGlDriver.Font", "Generated texture for `" + this->name + "`@" + size + " in " + ( ( end.QuadPart - begin.QuadPart ) / ( freq.QuadPart / 1000.0 ) )
                + " ms. (height correction factor is " + heightCorrection + ")" );
                
        Resource::add( this );

        if ( driver->globalState.fontBatchingEnabled )
        {
            batch = new Batch;
            batch->size = driver->globalState.fontBatchSize;
            batch->used = 0;

            batch->vbo = 0;

            //batch->vertices = Allocator<float>::allocate( batch->size * 6 * 4 );
            batch->vertices = Allocator<float>::allocate( batch->size * 4 * 4 );

            SG_assert( batch->vertices != nullptr )

            if ( driver->features.gl3PlusOnly )
            {
                glApi.functions.glGenBuffers( 1, &batch->vbo );
                glApi.functions.glBindBuffer( GL_ARRAY_BUFFER, batch->vbo );
                //glApi.functions.glBufferData( GL_ARRAY_BUFFER, batch->size * 6 * 4 * sizeof( float ), nullptr, GL_STREAM_DRAW );
                glApi.functions.glBufferData( GL_ARRAY_BUFFER, batch->size * 4 * 4 * sizeof( float ), nullptr, GL_STREAM_DRAW );
            }
        }
    }

    Font::~Font()
    {
        if ( batch != nullptr )
        {
            batch->vertices = Allocator<float>::release( batch->vertices );

            if ( driver->features.gl3PlusOnly )
                glApi.functions.glDeleteBuffers( 1, &batch->vbo );

            batch.release();
        }

        //TTF_CloseFont( font );
        closeFont( font );

        Resource::remove( this );
    }

    void Font::batchFlush( const Colour& colour )
    {
        //printf( "batchFlush %u/%u glyphs\n", batch->used, batch->size );

        if ( batch->used == 0 )
            return;

        if ( driver->features.gl3PlusOnly )
        {
            glApi.functions.glBindBuffer( GL_ARRAY_BUFFER, batch->vbo );
            //glApi.functions.glBufferSubData( GL_ARRAY_BUFFER, 0, batch->used * 6 * 4 * sizeof( float ), batch->vertices );
            glApi.functions.glBufferSubData( GL_ARRAY_BUFFER, 0, batch->used * 4 * 4 * sizeof( float ), batch->vertices );
        }
        else
            glApi.functions.glBindBuffer( GL_ARRAY_BUFFER, 0 );

        material->apply( colour );

        glApi.functions.glClientActiveTexture( GL_TEXTURE0 );

        glEnableClientState( GL_VERTEX_ARRAY );
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );

        if ( driver->features.gl3PlusOnly )
        {
            if ( driver->globalState.currentCoordSource != &batch->vbo )
            {
                glVertexPointer( 2, GL_FLOAT, 4 * sizeof( float ), reinterpret_cast<const GLvoid*>( 0 ) );
                driver->globalState.currentCoordSource = &batch->vbo;
            }

            if ( driver->globalState.currentUvSource[0] != &batch->vbo )
            {
                glTexCoordPointer( 2, GL_FLOAT, 4 * sizeof( float ), reinterpret_cast<const GLvoid*>( 2 * sizeof( float ) ) );
                driver->globalState.currentUvSource[0] = &batch->vbo;
            }
        }
        else
        {
            if ( driver->globalState.currentCoordSource != batch->vertices )
            {
                glVertexPointer( 2, GL_FLOAT, 4 * sizeof( float ), batch->vertices );
                driver->globalState.currentCoordSource = batch->vertices;
            }

            if ( driver->globalState.currentUvSource[0] != batch->vertices )
            {
                glTexCoordPointer( 2, GL_FLOAT, 4 * sizeof( float ), batch->vertices + 2 );
                driver->globalState.currentUvSource[0] = batch->vertices;
            }
        }

        //glDrawArrays( GL_TRIANGLES, 0, batch->used * 6 );
        glDrawArrays( GL_QUADS, 0, batch->used * 4 );
        stats.numPolys += batch->used * 2;
        stats.numRenderCalls++;

        glDisableClientState( GL_VERTEX_ARRAY );
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );

        batch->used = 0;

        driver->renderState.currentMesh = nullptr;
    }

    float Font::batchGlyph( float x, float y, Unicode::Char c, const Colour& colour )
    {
        if ( !glyphs[c].defined )
            return 0.0f;

        if ( batch->used >= batch->size )
            batchFlush( colour );

        float correctedSize = size * heightCorrection;

        //size_t index = batch->used * 6 * 4;
        size_t index = batch->used * 4 * 4;

        Vector2<> pos0( x, y - glyphs[c].offset ), pos1( x + glyphs[c].width * correctedSize, y - glyphs[c].offset + correctedSize );
        Vector2<> uv0( glyphs[c].u[0], 1.0f - glyphs[c].v[0] ), uv1( glyphs[c].u[1], 1.0f - glyphs[c].v[1] );

        /*batch->vertices[index++] = pos0.x;
        batch->vertices[index++] = pos0.y;

        batch->vertices[index++] = uv0.x;
        batch->vertices[index++] = uv0.y;

        batch->vertices[index++] = pos0.x;
        batch->vertices[index++] = pos1.y;

        batch->vertices[index++] = uv0.x;
        batch->vertices[index++] = uv1.y;

        batch->vertices[index++] = pos1.x;
        batch->vertices[index++] = pos0.y;

        batch->vertices[index++] = uv1.x;
        batch->vertices[index++] = uv0.y;

        batch->vertices[index++] = pos1.x;
        batch->vertices[index++] = pos0.y;

        batch->vertices[index++] = uv1.x;
        batch->vertices[index++] = uv0.y;

        batch->vertices[index++] = pos0.x;
        batch->vertices[index++] = pos1.y;

        batch->vertices[index++] = uv0.x;
        batch->vertices[index++] = uv1.y;

        batch->vertices[index++] = pos1.x;
        batch->vertices[index++] = pos1.y;

        batch->vertices[index++] = uv1.x;
        batch->vertices[index++] = uv1.y;*/

        batch->vertices[index++] = pos0.x;
        batch->vertices[index++] = pos0.y;

        batch->vertices[index++] = uv0.x;
        batch->vertices[index++] = uv0.y;

        batch->vertices[index++] = pos0.x;
        batch->vertices[index++] = pos1.y;

        batch->vertices[index++] = uv0.x;
        batch->vertices[index++] = uv1.y;

        batch->vertices[index++] = pos1.x;
        batch->vertices[index++] = pos1.y;

        batch->vertices[index++] = uv1.x;
        batch->vertices[index++] = uv1.y;

        batch->vertices[index++] = pos1.x;
        batch->vertices[index++] = pos0.y;

        batch->vertices[index++] = uv1.x;
        batch->vertices[index++] = uv0.y;

        batch->used++;

        return glyphs[c].advance * correctedSize;
    }

    void Font::batchString( float x0, float y0, const char* text, intptr_t numBytes, Colour colour, bool shadow )
    {
        if ( text == nullptr )
            return;

        Utf8Char next;
        bool bold = false;

        static const float boldDist = 1.0f;

        if ( numBytes < 0 )
            numBytes = strlen( text );

        Escape escape = Escape_none;
        float x = round( x0 ), y = round( y0 );
        float width = x;

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
                        width = maximum( width, x );
                        x = x0;
                        y += lineSkip;
                    }
                    else if ( next == '\\' )
                    {
                        batchFlush( colour );
                        escape = Escape_unk;
                    }
                    else
                    {
                        if ( bold )
                            batchGlyph( x + boldDist, y, next, colour );

                        x += batchGlyph( x, y, next, colour );
                    }
                    break;

                case Escape_unk:
                    if ( next == '#' )
                        escape = Escape_colour;
                    else
                    {
                        if ( next == '\\' )
                        {
                            if ( bold )
                                batchGlyph( x + boldDist, y, next, colour );

                            x += renderChar( x, y, next, colour );
                        }
                        else if ( !shadow && next >= '0' && next <= '9' )
                            colour = Colour::grey( 0.1f * ( next - '0' ), colour.a );
                        else if ( !shadow && next == 'b' )
                            colour = Colour( 0.1f, 0.2f, 0.9f, colour.a );
                        else if ( !shadow && next == 'g' )
                            colour = Colour( 0.2f, 0.9f, 0.1f, colour.a );
                        else if ( !shadow && next == 'l' )
                            colour = Colour( 0.5f, 0.9f, 0.1f, colour.a );
                        else if ( !shadow && next == 'o' )
                            colour = Colour( 0.9f, 0.5f, 0.1f, colour.a );
                        else if ( !shadow && next == 'p' )
                            colour = Colour( 0.9f, 0.1f, 0.5f, colour.a );
                        else if ( !shadow && next == 'r' )
                            colour = Colour( 0.9f, 0.1f, 0.2f, colour.a );
                        else if ( !shadow && next == 's' )
                            colour = Colour( 0.1f, 0.5f, 0.9f, colour.a );
                        else if ( !shadow && next == 'w' )
                            colour = Colour( 1.0f, 1.0f, 1.0f, colour.a );
                        else if ( !shadow && next == 'y' )
                            colour = Colour( 0.9f, 0.9f, 0.1f, colour.a );
                        else if ( next == 'B' )
                            bold = true;

                        escape = Escape_none;
                    }
                    break;

                case Escape_colour:
                    if ( !shadow )
                        colour.r = ( next >= '0' && next <= '9' ) ? ( next - '0' ) / 9.0f : 0.0f;

                    escape = Escape_colour_r;
                    break;

                case Escape_colour_r:
                    if ( !shadow )
                        colour.g = ( next >= '0' && next <= '9' ) ? ( next - '0' ) / 9.0f : 0.0f;
                    
                    escape = Escape_colour_rg;
                    break;

                case Escape_colour_rg:
                    if ( !shadow )
                        colour.b = ( next >= '0' && next <= '9' ) ? ( next - '0' ) / 9.0f : 0.0f;
                    
                    escape = Escape_none;
                    break;
            }
        }

        batchFlush( colour );
    }

    void Font::drawString( const Vector2<>& pos, const String& string, const Colour& colour, unsigned short align )
    {
        static const float shadowDist = 1.0f;

        if ( align == ( left | top ) )
        {
            if ( driver->globalState.fontBatchingEnabled )
            {
                batchString( pos.x + shadowDist, pos.y + shadowDist, string, string.getNumBytes(), Colour( 0.0f, 0.0f, 0.0f, colour.a ), true );
                batchString( pos.x, pos.y, string, string.getNumBytes(), colour, false );
            }
            else
                layoutString( pos.x, pos.y, string, string.getNumBytes(), colour, true );
        }
        else
        {
            Vector2<> dimensions = layoutString( 0.0f, 0.0f, string, string.getNumBytes(), colour, false );
            Vector2<> pos2( pos );

            if ( align & centered )
                pos2.x -= dimensions.x / 2;
            else if ( align & right )
                pos2.x -= dimensions.x;

            if ( align & middle )
                pos2.y -= dimensions.y / 2;
            else if ( align & bottom )
                pos2.y -= dimensions.y;

            if ( driver->globalState.fontBatchingEnabled )
            {
                batchString( pos2.x + shadowDist, pos2.y + shadowDist, string, string.getNumBytes(), Colour( 0.0f, 0.0f, 0.0f, colour.a ), true );
                batchString( pos2.x, pos2.y, string, string.getNumBytes(), colour, false );
            }
            else
                layoutString( pos2.x, pos2.y, string, string.getNumBytes(), colour, true );
        }
    }

    void Font::drawText( const Vector2<>& pos, const Text* text, float alpha )
    {
        static const float shadowDist = 1.0f;

        auto layout = reinterpret_cast<const Layout*>( text );

        SG_assert( text != nullptr )

        if ( driver->globalState.fontBatchingEnabled )
        {
            batchString( pos.x + layout->x + shadowDist, pos.y + layout->y + shadowDist, layout->string, layout->string.getNumBytes(), Colour( 0.0f, 0.0f, 0.0f, alpha ), true );
            batchString( pos.x + layout->x, pos.y + layout->y, layout->string, layout->string.getNumBytes(), layout->colour * Colour::white( alpha ), false );
        }
        else
            layoutString( pos.x + layout->x, pos.y + layout->y, layout->string, layout->string.getNumBytes(), layout->colour * Colour::white( alpha ), true );
    }

    void Font::exitFreeType()
    {
        ::exitFreeType();
    }

    float Font::getCharWidth( Unicode::Char c )
    {
        if ( !glyphs[c].defined )
            return 0.0f;

        return glyphs[c].advance * size * heightCorrection;
    }

    unsigned Font::getSize()
    {
        return size;
    }

    unsigned Font::getStyle()
    {
        return style;
    }

    Vector2<float> Font::getTextDimensions( Text* text )
    {
        Layout* layout = ( Layout* ) text;

        return layout->dimensions;
    }

    Text* Font::layoutText( const String& text, const Colour& colour, unsigned short align )
    {
        Vector2<float> dimensions = layoutString( 0.0f, 0.0f, text, text.getNumBytes(), colour, false );

        Layout* layout = new Layout;
        layout->dimensions = dimensions;
        layout->colour = colour;
        layout->string = text;
        layout->x = 0.0f;
        layout->y = 0.0f;

        if ( align != 0 )
        {
            if ( align & centered )
                layout->x -= dimensions.x / 2;
            else if ( align & right )
                layout->x -= dimensions.x;

            if ( align & middle )
                layout->y -= dimensions.y / 2;
            else if ( align & bottom )
                layout->y -= dimensions.y;
        }

        return ( Text* ) layout;
    }

    Vector2<float> Font::layoutString( float x0, float y0, const char* text, intptr_t numBytes, Colour colour, bool render )
    {
        Utf8Char next;
        bool bold = false;

        static const float boldDist = 1.0f, shadowDist = 1.0f;
        Colour shadowColour = Colour( 0.0f, 0.0f, 0.0f, colour.a );

        if ( !text )
            return Vector2<float>();

        if ( numBytes < 0 )
            numBytes = strlen( text );

        Escape escape = Escape_none;
        float x = round( x0 ), y = round( y0 );
        float width = x;

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
                        width = maximum( width, x );
                        x = x0;
                        y += lineSkip;
                    }
                    else if ( next == '\\' )
                        escape = Escape_unk;
                    else
                    {
                        if ( render )
                        {
                            renderChar( x + shadowDist, y + shadowDist, next, shadowColour );

                            if ( bold )
                                renderChar( x + boldDist, y, next, colour );

                            x += renderChar( x, y, next, colour );
                        }
                        else
                            x += getCharWidth( next );
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
                                x += renderChar( x, y, next, colour );
                            else
                                x += getCharWidth( next );
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

        return Vector2<float>( maximum( width, x ) - x0, y + lineSkip - y0 );
    }

    void Font::releaseText( Text* text )
    {
        Layout* layout = ( Layout* ) text;

        delete layout;
    }

    float Font::renderChar( float x, float y, Unicode::Char c, const Colour& colour )
    {
        if ( !glyphs[c].defined )
            return 0.0f;

        float correctedSize = size * heightCorrection;

        glPushMatrix();
        glTranslatef( x, y - glyphs[c].offset, 0.0f );
        glScalef( glyphs[c].width * correctedSize, correctedSize, 1.0f );

        glApi.functions.glActiveTexture( GL_TEXTURE0 );
        glMatrixMode( GL_TEXTURE );
        glPushMatrix();
        glTranslatef( glyphs[c].u[0], 1.0f - glyphs[c].v[1], 0.0f );
        glScalef( glyphs[c].u[1] - glyphs[c].u[0], glyphs[c].v[1] - glyphs[c].v[0], 1.0f );

        plane->render( nullptr, colour );

        glApi.functions.glActiveTexture( GL_TEXTURE0 );
        glPopMatrix();

        glMatrixMode( GL_MODELVIEW );
        glPopMatrix();

        return glyphs[c].advance * correctedSize;
    }

    void Font::renderString( float x, float y, const String& string, const Colour& colour, unsigned short align )
    {
        if ( align != 0 )
        {
            Vector2<float> dimensions = layoutString( 0.0f, 0.0f, string, string.getNumBytes(), colour, false );

            if ( align & centered )
                x -= dimensions.x / 2;
            else if ( align & right )
                x -= dimensions.x;

            if ( align & middle )
                y -= dimensions.y / 2;
            else if ( align & bottom )
                y -= dimensions.y;
        }

        layoutString( x, y, string, string.getNumBytes(), colour, true );
    }

    void Font::renderText( float x, float y, const Text* text )
    {
        Layout* layout = ( Layout* ) text;

        SG_assert3( text != nullptr, "OpenGlDriver.Font.renderText" )

        layoutString( x + layout->x, y + layout->y, layout->string, layout->string.getNumBytes(), layout->colour, true );
    }
}
