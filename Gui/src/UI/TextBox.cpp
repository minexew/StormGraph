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

#include "GuiDriver.hpp"

namespace GuiDriver
{
    static const Vector2<float> textBoxMinSize( 16.0f, 16.0f );

    TextBox::TextBox( Gui* gui, const Vector<float>& pos, const Vector<float>& size )
            : Widget( pos.getXy(), size.getXy(), textBoxMinSize ), layout(nullptr)
    {
        graphicsDriver = gui->getGraphicsDriver();
        font = gui->getTextFont();

        backspace = graphicsDriver->getKey( "Backspace" );

        clear();
    }

    TextBox::~TextBox()
    {
        font->releaseText( layout );
    }

    void TextBox::clear()
    {
        setValue( "" );
    }

    void TextBox::onKeyState( int16_t key, Key::State state, Unicode::Char character )
    {
        if ( state == Key::pressed )
        {
            if ( key == backspace )
                setValue( value.dropRightPart( 1 ) );
            else if ( character >= ' ' )
                setValue( value + Unicode::Character( character ) );
        }
    }

    bool TextBox::onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse )
    {
        if ( mouse < realPos || mouse > realPos + realSize )
            return false;

        return true;
    }

    void TextBox::render()
    {
#ifdef li_GCC4
        graphicsDriver->pushClippingRect( ScreenRect { realPos, realSize } );
#else
        ScreenRect sr = { realPos, realSize };
        graphicsDriver->pushClippingRect( sr );
#endif

        graphicsDriver->drawRectangle( realPos, realSize, Colour::grey( 0.2f, 0.9f ), nullptr );

        if ( layoutSize.x < realSize.x )
            font->drawText( realPos + Vector2<>( 4.0f, realSize.y / 2 ), layout );
        else
            font->drawText( realPos + Vector2<>( realSize.x - 4.0f - layoutSize.x, realSize.y / 2 ), layout );

        graphicsDriver->popClippingRect();
    }

    void TextBox::setMinSize( const Vector<float>& minSize )
    {
        this->minSize = textBoxMinSize.maximum( minSize.getXy() );

        realign();
    }

    void TextBox::setValue( const char* value )
    {
        font->releaseText( layout );
        layout = font->layoutText( ( String ) value + "\\#579+", Colour::white(), IFont::left | IFont::middle );
        layoutSize = font->getTextSize( layout );

        this->value = value;
    }
}
