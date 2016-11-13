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
    static const Vector2<> scrollBarPadding( 4.0f, 4.0f );

    ScrollBar::ScrollBar( Gui* gui, const Vector<float>& pos, const Vector<float>& size, Orientation orientation )
            : Widget( pos.getXy(), size.getXy(), Vector2<>() ), orientation( orientation ), pos( 0.0f )
    {
        graphicsDriver = gui->getGraphicsDriver();
    }

    ScrollBar::~ScrollBar()
    {
    }

    Vector<float> ScrollBar::getMinSize()
    {
        return Widget::getMinSize().maximum( scrollBarPadding );
    }

    bool ScrollBar::onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse )
    {
        if ( mouse < realPos || mouse > realPos + realSize )
            return false;

        return true;
    }

    void ScrollBar::render()
    {
        if ( orientation == Orientation::horizontal )
        {
            graphicsDriver->drawRectangle( realPos + scrollBarPadding,
                    Vector2<>( factor * ( realSize.x - scrollBarPadding.x ), realSize.y - scrollBarPadding.y * 2 ), Colour::grey( 0.8f, 0.5f ), nullptr );
        }
        else
        {
            graphicsDriver->drawRectangle( realPos + scrollBarPadding,
                    Vector2<>( realSize.x - scrollBarPadding.x * 2, factor * ( realSize.y - scrollBarPadding.y ) ), Colour::grey( 0.8f, 0.5f ), nullptr );
        }
    }

    void ScrollBar::setRange( float visible, float content )
    {
        SG_assert( visible > 0.0f )
        SG_assert( content > 0.0f )

        this->visible = visible;
        this->content = content;

        factor = visible / content;
        pos = minimum( pos, maximum( content - visible, 0.0f ) );
    }
}