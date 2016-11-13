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
    Panel::Panel( Gui* gui, const Vector<float>& pos, const Vector<float>& size )
            : Widget( pos.getXy(), size.getXy(), Vector2<>() ), padding( 4.0f, 4.0f ), gui( gui )
    {
        graphicsDriver = gui->getGraphicsDriver();
    }

    Panel::~Panel()
    {
    }

    void Panel::add( IChildWidget* widget )
    {
        widget->setBounds( pos + padding, size - padding * 2 );

        Container::add( widget );
    }

    /*bool Panel::fitsInArea( const Vector<float>& size )
    {
        return Widget::fitsInArea( size );
    }*/

    Vector<float> Panel::getMinSize( bool ofContentArea )
    {
        Vector<float> minSize = Widget::getMinSize();

        if ( ofContentArea || !scrollable )
            iterate ( widgets )
                minSize = minSize.maximum( static_cast<IChildWidget*>( ( IWidget* ) widgets.current() )->getMinSize() );

        return minSize + padding * 2;
    }

    void Panel::layout()
    {
        Widget::realign();

        if ( scrollable && getMinSize( true ) > realSize )
        {
            // TODO: install a scrollbar


        }

        iterate( widgets )
            static_cast<IChildWidget*>( ( IWidget* ) widgets.current() )->setBounds( realPos + padding, realSize - padding * 2 );
    }

    void Panel::move( const Vector<float>& vec )
    {
        pos += vec.getXy();

        //Container::move( vec );

        layout();
    }

    void Panel::onKeyState( int16_t key, Key::State state, Unicode::Char character )
    {
        Container::onKeyState( key, state, character );
    }

    bool Panel::onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse )
    {
        return Container::onMouseButton( button, pressed, mouse );
    }

    void Panel::setAlign( unsigned align )
    {
        Widget::setAlign( align );
    }

    void Panel::setBounds( const Vector<float>& pos, const Vector<float>& size )
    {
        Widget::setBounds( pos, size );

        iterate ( widgets )
            static_cast<IChildWidget*>( ( IWidget* ) widgets.current() )->setBounds( pos + padding, size - padding * 2 );
    }

    void Panel::setPadding( const Vector<>& padding )
    {
        this->padding = padding.getXy();
    }

    void Panel::setScrollable( bool scrollable )
    {
        this->scrollable = scrollable;

        if ( scrollable )
        {
            horScrollBar = new ScrollBar( gui, realPos + Vector2<>( 0.0f, realSize.y - 16.0f ), Vector<>( realSize.x, 16.0f ), Orientation::horizontal );
            vertScrollBar = new ScrollBar( gui, realPos + Vector2<>( realSize.x - 16.0f, 0.0f ), Vector<>( 16.0f, realSize.y ), Orientation::vertical );
        }
    }

    void Panel::render()
    {
        graphicsDriver->drawRectangle( realPos, realSize, Colour::grey( 0.2f, 0.8f ), nullptr );

#ifdef li_GCC4
        graphicsDriver->pushClippingRect( ScreenRect { realPos, realSize } );
#else
        ScreenRect sr = { realPos, realSize };
        graphicsDriver->pushClippingRect( sr );
#endif
        Container::render();

        if ( horScrollBar != nullptr )
            horScrollBar->render();

        if ( vertScrollBar != nullptr )
            vertScrollBar->render();

        graphicsDriver->popClippingRect();
    }

    void Panel::update( double delta )
    {
        Container::update( delta );
    }
}
