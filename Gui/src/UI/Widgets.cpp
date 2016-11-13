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
    Widget::Widget( const Vector2<float>& pos, const Vector2<float>& size, const Vector2<float>& minSize )
            : pos( pos ), size( size ), minSize( minSize ), align( ISizableWidget::centered | ISizableWidget::middle ), expand( false ), freeFloat( true )
    {
        areaPos = pos;
        areaSize = size;

        realign();
    }

    Widget::~Widget()
    {
    }

    /*bool Widget::fitsInArea( const Vector<float>& size ) const
    {
        return !( minSize > size.getXy() );
    }*/

    Vector<float> Widget::getMinSize()
    {
        return minSize;
    }

    /*void Widget::move( const Vector<float>& vec )
    {
        pos += vec;
    }*/

    void Widget::realign()
    {
        if ( freeFloat )
        {
            realPos = pos;
            realSize = size.maximum( getMinSize().getXy() );
        }
        else
        {
            if ( expand )
                realSize = areaSize/*.maximum( getMinSize().getXy() )*/;
            else
                realSize = size.maximum( getMinSize().getXy() ).minimum( areaSize );

            if ( align & ISizableWidget::centered )
                realPos.x = areaPos.x + ( areaSize.x - realSize.x ) / 2.0f;
            else if ( align & ISizableWidget::right )
                realPos.x = areaPos.x + areaSize.x - realSize.x;
            else
                realPos.x = areaPos.x;

            if ( align & ISizableWidget::middle )
                realPos.y = areaPos.y + ( areaSize.y - realSize.y ) / 2.0f;
            else if ( align & ISizableWidget::bottom )
                realPos.y = areaPos.y + areaSize.y - realSize.y;
            else
                realPos.y = areaPos.y;
        }
    }

    void Widget::setAlign( unsigned align )
    {
        this->align = align;

        realign();
    }

    void Widget::setBounds( const Vector<float>& pos, const Vector<float>& size )
    {
        areaPos = pos.getXy();
        areaSize = size.getXy();

        realign();
    }

    void Widget::setExpand( bool expand )
    {
        this->expand = expand;

        realign();
    }

    void Widget::setFreeFloat( bool freeFloat )
    {
        this->freeFloat = false;

        realign();
    }

    void Widget::setName( const char* name )
    {
        this->name = name;
    }

    Container::Container()
    {
    }

    Container::~Container()
    {
    }

    void Container::add( IWidget* widget )
    {
        widgets.add( widget );
    }

    /*void Container::move( const Vector<float>& vec )
    {
        //iterate ( widgets )
        //    widgets.current()->move( vec );
    }*/

    void Container::onKeyState( int16_t key, Key::State state, Unicode::Char character )
    {
        iterate ( widgets )
            widgets.current()->onKeyState( key, state, character );
    }

    bool Container::onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse )
    {
        iterate ( widgets )
            if ( widgets.current()->onMouseButton( button, pressed, mouse ) )
                return true;

        return false;
    }

    void Container::onMouseMoveTo( const Vector2<int>& mouse )
    {
        iterate ( widgets )
            widgets.current()->onMouseMoveTo( mouse );
    }

    void Container::render()
    {
        iterate ( widgets )
            widgets.current()->render();
    }

    void Container::update( double delta )
    {
        iterate ( widgets )
            widgets.current()->update( delta );
    }
}
