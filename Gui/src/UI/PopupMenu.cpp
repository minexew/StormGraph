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
    static const Vector2<float> checkBoxSize( 10.0f, 10.0f ), checkBoxSize2( 4.0f, 4.0f );

    PopupMenu::PopupMenu( Gui* gui )
            : gui( gui ), selectedItem( -1 ), eventListener( nullptr ), padding( 4.0f, 2.0f )
    {
        graphicsDriver = gui->getGraphicsDriver();
        font = gui->getTextFont();
    }

    PopupMenu::~PopupMenu()
    {
        iterate ( items )
            if ( items.current().labelLayout != nullptr )
                font->releaseText( items.current().labelLayout );
    }

    unsigned PopupMenu::addCommand( const char* text )
    {
        Item item;

        item.type = Item::command;
        //item.text = text;
        item.labelLayout = font->layoutText( text, Colour::white(), IFont::left | IFont::middle );

        unsigned id = items.add( item );

        realign();

        return id;
    }

    unsigned PopupMenu::addSpacer()
    {
        Item item;

        item.type = Item::spacer;
        item.labelLayout = nullptr;

        unsigned id = items.add( item );

        realign();

        return id;
    }

    unsigned PopupMenu::addToggle( const char* text, bool value )
    {
        Item item;

        item.type = Item::boolean;
        //item.text = text;
        item.checked = value;
        item.labelLayout = font->layoutText( text, Colour::white(), IFont::left | IFont::middle );

        unsigned id = items.add( item );

        realign();

        return id;
    }

    bool PopupMenu::getToggleValue( unsigned id )
    {
        if ( id >= items.getLength() || items[id].type != Item::boolean )
            throw Exception( "GuiDriver.PopupMenu.getToggleValue", "InvalidId", "Invalid item id (not a toggle/outside bounds)" );

        return items[id].checked;
    }

    void PopupMenu::onKeyState( int16_t key, Key::State state, Unicode::Char character )
    {
    }

    bool PopupMenu::onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse )
    {
        if ( mouse < realPos || mouse > realPos + realSize )
        {
            if ( eventListener != nullptr )
            {
                IGuiEventListener::MenuCancelEvent event = { this };
                eventListener->onGuiMenuCancel( event );
            }

            gui->remove( this );

            return false;
        }

        if ( selectedItem < 0 || button != MouseButton::left )
            return true;

        if ( items[selectedItem].type == Item::boolean && pressed )
        {
            items[selectedItem].checked = !items[selectedItem].checked;

            if ( eventListener != nullptr )
            {
                IGuiEventListener::MenuToggleEvent event = { this, ( unsigned ) selectedItem, items[selectedItem].checked };
                eventListener->onGuiMenuToggle( event );
            }
        }
        else if ( items[selectedItem].type == Item::command && !pressed )
        {
            if ( eventListener != nullptr )
            {
                IGuiEventListener::MenuCommandEvent event = { this, ( unsigned ) selectedItem };
                eventListener->onGuiMenuCommand( event );
            }

            gui->remove( this );
        }

        return true;
    }

    void PopupMenu::onMouseMoveTo( const Vector2<int>& mouse )
    {
        if ( !( mouse < realPos || mouse > realPos + realSize ) )
        {
            float y = realPos.y;

            iterate ( items )
            {
                if ( mouse.y >= y && mouse.y < y + items.current().height )
                {
                    selectedItem = ( items.current().type != Item::spacer ) ? items.iter() : -1;
                    break;
                }

                y += items.current().height;
            }
        }
    }

    void PopupMenu::realign()
    {
        realSize = Vector2<>();

        iterate ( items )
        {
            Vector<float> size;

            if ( items.current().labelLayout != nullptr )
                size = font->getTextSize( items.current().labelLayout );

            if ( items.current().type == Item::boolean )
            {
                items.current().labelX = padding.x + checkBoxSize.x;
                size.x += items.current().labelX;
                items.current().height = maximum( size.y, checkBoxSize.y ) + padding.y * 2;
            }
            else if ( items.current().type == Item::command )
            {
                items.current().labelX = 0.0f;
                items.current().height = size.y + padding.y * 2;
            }
            else
                items.current().height = padding.y * 2;

            realSize.x = maximum( realSize.x, size.x + padding.x * 2 );
            realSize.y += items.current().height;
        }

        realSize = realSize.maximum( minSize );
    }

    void PopupMenu::render()
    {
        graphicsDriver->drawRectangle( realPos, realSize, Colour::grey( 0.3f, 0.9f ), nullptr );

        float y = realPos.y;

        iterate ( items )
        {
            if ( items.iter() == selectedItem )
                graphicsDriver->drawRectangle( Vector2<>( realPos.x, y ), Vector2<>( realSize.x, items.current().height ), Colour::grey( 0.0f, 0.3f ), nullptr );

            if ( items.current().labelLayout != nullptr )
                font->renderText( realPos.x + padding.x + items.current().labelX, y + items.current().height / 2, items.current().labelLayout );

            if ( items.current().type == Item::boolean )
            {
                graphicsDriver->drawRectangle( Vector2<>( realPos.x + padding.x, y + ( items.current().height - checkBoxSize.y ) / 2 ),
                        checkBoxSize, Colour::grey( 0.0f, 0.6f ), nullptr );

                if ( items.current().checked )
                    graphicsDriver->drawRectangle( Vector2<>( realPos.x + padding.x + ( checkBoxSize.x - checkBoxSize2.x ) / 2, y + ( items.current().height - checkBoxSize2.y ) / 2 ),
                            checkBoxSize2, Colour( 0.0f, 1.0f, 0.0f, 0.8f ), nullptr );
            }
            else if ( items.current().type == Item::spacer )
                graphicsDriver->drawLine( Vector<>( realPos.x + padding.x, y + padding.y ), Vector<>( realPos.x + realSize.x - padding.x, y + padding.y ), Colour::white( 0.6f ) );

            y += items.current().height;
        }
    }

    void PopupMenu::setEventListener( IGuiEventListener* listener )
    {
        eventListener = listener;
    }

    void PopupMenu::setMinSize( const Vector<float>& minSize )
    {
        this->minSize = minSize.getXy();
    }

    void PopupMenu::show( const Vector<>& pos )
    {
        realPos = pos.getXy();
        selectedItem = -1;

        gui->pushModal( reference() );
    }

    void PopupMenu::update( double delta )
    {
    }
}
