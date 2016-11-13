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
    ProgressBar::ProgressBar( Gui* gui, const Vector<float>& pos, const Vector<float>& size )
            : Widget( pos.getXy(), size.getXy(), Vector2<>() ), padding( 4.0f, 4.0f )
    {
        graphicsDriver = gui->getGraphicsDriver();
    }

    Vector<> ProgressBar::getMinSize()
    {
        return Widget::getMinSize().maximum( padding * 2 );
    }

    bool ProgressBar::onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse )
    {
        if ( mouse < realPos || mouse > realPos + realSize )
            return false;

        return true;
    }

    void ProgressBar::render()
    {
        graphicsDriver->drawRectangle( realPos, realSize, Colour::grey( 0.4f, 0.9f ), nullptr );
        graphicsDriver->drawRectangle( realPos + padding, Vector2<>( ( realSize.x - 2 * padding.x ) * progress, realSize.y - 2 * padding.y ), Colour::grey( 0.8f, 0.9f ), nullptr );
    }

    void ProgressBar::setMinSize( const Vector<float>& minSize )
    {
        this->minSize = getMinSize().getXy().maximum( minSize.getXy() );

        realign();
    }

    void ProgressBar::setPadding( const Vector<>& padding )
    {
        this->padding = padding.getXy();
    }

    void ProgressBar::setProgress( float progress )
    {
        if ( progress < 0.0f )
            this->progress = 0.0f;
        else if ( progress > 1.0f )
            this->progress = 1.0f;
        else
            this->progress = progress;
    }
}
