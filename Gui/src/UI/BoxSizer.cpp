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
    BoxSizer::BoxSizer( Gui* gui, Orientation orientation )
            : orientation( orientation ), spacing( 4.0f )
    {
    }

    BoxSizer::~BoxSizer()
    {
    }

    void BoxSizer::add( IChildWidget* widget )
    {
        ISizableWidget* sizable = dynamic_cast<ISizableWidget*>( widget );

        if ( sizable != nullptr )
            sizable->setFreeFloat( false );

        Container::add( widget );

        layout();
    }

    Vector<float> BoxSizer::getMinSize()
    {
        Vector<float> minSize;

        iterate ( widgets )
            minSize = minSize.maximum( static_cast<IChildWidget*>( ( IWidget* ) widgets.current() )->getMinSize() );

        if ( orientation == Orientation::horizontal )
        {
            minSize.x *= widgets.getLength();
            minSize.x += ( widgets.getLength() - 1 ) * spacing;
        }
        else
        {
            minSize.y *= widgets.getLength();
            minSize.y += ( widgets.getLength() - 1 ) * spacing;
        }

        return minSize;
    }

    void BoxSizer::layout()
    {
        if ( widgets.isEmpty() )
            return;

        if ( orientation == Orientation::horizontal )
        {
            float cellWidth = ceil( ( areaSize.x - ( widgets.getLength() - 1 ) * spacing ) / widgets.getLength() );

            for each_in_list( widgets, i )
                static_cast<IChildWidget*>( ( IWidget* ) widgets[i] )->setBounds( areaPos + Vector2<>( ( cellWidth + spacing ) * i, 0.0f ), Vector<>( cellWidth, areaSize.y ) );
        }
        else
        {
            float cellHeight = ceil( ( areaSize.y - ( widgets.getLength() - 1 ) * spacing ) / widgets.getLength() );

            for each_in_list( widgets, i )
                static_cast<IChildWidget*>( ( IWidget* ) widgets[i] )->setBounds( areaPos + Vector2<>( 0.0f, ( cellHeight + spacing ) * i ), Vector<>( areaSize.x, cellHeight ) );
        }
    }

    void BoxSizer::onKeyState( int16_t key, Key::State state, Unicode::Char character )
    {
        Container::onKeyState( key, state, character );
    }

    bool BoxSizer::onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse )
    {
        return Container::onMouseButton( button, pressed, mouse );
    }

    void BoxSizer::onMouseMoveTo( const Vector2<int>& mouse )
    {
        Container::onMouseMoveTo( mouse );
    }

    void BoxSizer::render()
    {
        Container::render();
    }

    void BoxSizer::setBounds( const Vector<float>& pos, const Vector<float>& size )
    {
        areaPos = pos.getXy();
        areaSize = size.getXy();

        layout();
    }

    void BoxSizer::update( double delta )
    {
        Container::update( delta );
    }
}
