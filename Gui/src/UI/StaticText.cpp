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
    StaticText::StaticText( Gui* gui, const Vector<float>& pos, const Vector<float>& size, const char* text )
            : Widget( pos.getXy(), size.getXy(), Vector2<>() ), eventListener( nullptr ), text( (const char*) nullptr ), layout( nullptr )
    {
        graphicsDriver = gui->getGraphicsDriver();
        font = gui->getTextFont();

        setText( text );
    }

    StaticText::~StaticText()
    {
        font->releaseText( layout );
    }

    Vector<float> StaticText::getMinSize()
    {
        return Widget::getMinSize().maximum( font->getTextDimensions( layout ).ceil() );
    }

    bool StaticText::onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse )
    {
        if ( mouse < realPos || mouse > realPos + realSize )
            return false;

        if ( button == MouseButton::left && pressed && eventListener != nullptr )
        {
            IGuiEventListener::CommandEvent event = { this, mouse };

            eventListener->onGuiCommand( event );
        }

        return true;
    }

    void StaticText::render()
    {
        graphicsDriver->drawRectangle( realPos, realSize, Colour::grey( 0.8f, 0.1f ), nullptr );

        font->renderText( ceil( realPos.x + realSize.x / 2 ), ceil( realPos.y + realSize.y / 2 ), layout );
    }

    void StaticText::setText( const char* text )
    {
        font->releaseText( layout );
        layout = font->layoutText( text, Colour::white(), IFont::centered | IFont::middle );

        this->text = text;

        realign();
    }
}
