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

#include "Internal.hpp"

#include <StormGraph/Engine.hpp>
#include <StormGraph/GraphicsDriver.hpp>
#include <StormGraph/OnScreenLog.hpp>
#include <StormGraph/ResourceManager.hpp>

#include <vector>

namespace StormGraph
{
    class OnScreenLog : public IEventListener, public IOnScreenLog
    {
        struct Line { Text* text; double alpha, time; };

        IEngine* engine;
        Reference<IFont> font;

        double disappearTime, displayTime;

        List<Line> lines;
        Vector2<> pos;
        unsigned numMaxLines, spacing;

        public:
            OnScreenLog( IEngine* engine, const ScreenRect& area, IFont* font )
                    : engine( engine ), font( font ), disappearTime( 1.5 ), displayTime( 5.0 ), numMaxLines( 0 )
            {
                if ( font == nullptr )
                {
                    this->font = engine->getSharedResourceManager()->getFont( "Common/Fonts/DejaVuSans.ttf", 14, IFont::bold );

                    SG_assert( this->font != nullptr )
                }

                pos = Vector2<>( area.pos.x, area.pos.y + area.size.y );
                spacing = this->font->getLineSkip();
                numMaxLines = area.size.y / spacing;

                if ( numMaxLines <= 0 )
                    numMaxLines = 1;

                engine->registerEventListener( this );
            }

            virtual ~OnScreenLog()
            {
                reverse_iterate2 ( i, lines )
                    remove( i.getIndex() );

                engine->unregisterEventListener( this );
            }

            virtual void addLine( const char* text ) override
            {
                if ( lines.getLength() + 1 > numMaxLines )
                    remove( 0 );

                Line line = { font->layoutText( text, Colour::white(), IFont::left | IFont::bottom ), 1.0, 0.0 };
                lines.add( line );
            }

            virtual void onRender() override
            {
                Vector2<> pos = this->pos;

                reverse_iterate2 ( i, lines )
                {
                    Line& line = i;

                    font->drawText( pos, line.text, line.alpha );

                    pos.y -= spacing;
                }
            }

            virtual void onUpdate( double delta ) override
            {
                reverse_iterate2 ( i, lines )
                {
                    Line& line = i;

                    if ( line.time < displayTime )
                        line.time += delta;
                    else
                    {
                        line.alpha -= delta / disappearTime;

                        if ( line.alpha < 0.0 )
                            remove( i.getIndex() );
                    }
                }
            }

            void remove( size_t i )
            {
                font->releaseText( lines[i].text );
                lines.remove( i );
            }
    };

    IOnScreenLog* createOnScreenLog( IEngine* engine, const ScreenRect& area, IFont* font )
    {
        return new OnScreenLog( engine, area, font );
    }
}
