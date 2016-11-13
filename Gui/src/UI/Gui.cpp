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

#include <StormGraph/Engine.hpp>

namespace GuiDriver
{
    Gui::Gui( GuiDriver* driver, const Vector<float>& pos, const Vector<float>& size )
            : driver( driver ), pos( pos.getXy() ), size( size.getXy() )
    {
        lmb = getGraphicsDriver()->getKey( "Left Mouse Button" );

        driver->engine->registerEventListener( this );
    }

    Gui::~Gui()
    {
        driver->engine->unregisterEventListener( this );
    }

    void Gui::add( IWidget* widget )
    {
        widgets.add( widget );
    }

    IBoxSizer* Gui::createBoxSizer( Orientation orientation )
    {
        return new BoxSizer( this, orientation );
    }

    IButton* Gui::createButton( const char* text )
    {
        return new Button( this, Vector<>(), Vector<>(), text );
    }

    IButton* Gui::createButton( const Vector<float>& pos, const Vector<float>& size, const char* text )
    {
        return new Button( this, pos, size, text );
    }

    IPanel* Gui::createPanel( const Vector<float>& pos, const Vector<float>& size )
    {
        return new Panel( this, pos, size );
    }

    IPopupMenu* Gui::createPopupMenu()
    {
        return new PopupMenu( this );
    }

    ITableLayout* Gui::createTableLayout( size_t numColumns )
    {
        return new TableLayout( this, numColumns );
    }

    ITextBox* Gui::createTextBox( const Vector<float>& pos, const Vector<float>& size )
    {
        return new TextBox( this, pos, size );
    }

    IStaticText* Gui::createStaticText( const Vector<float>& pos, const Vector<float>& size, const char* text )
    {
        return new StaticText( this, pos, size, text );
    }

    IWindow* Gui::createWindow( const Vector<float>& pos, const Vector<float>& size, const char* title )
    {
        return new Window( this, pos, size, title );
    }

    IGraphicsDriver* Gui::getGraphicsDriver()
    {
        return driver->engine->getGraphicsDriver();
    }

    IFont* Gui::getTextFont()
    {
        if ( font == nullptr )
            font = driver->uiResMgr->getFont( "Common/Fonts/DejaVuSans.ttf", 10, IFont::normal );

        return font->reference();
    }

    void Gui::onKeyState( int16_t key, Key::State state, Unicode::Char character )
    {
        if ( key == lmb )
            onMouseButton( MouseButton::left, state == Key::pressed, mousePos );

        if ( !modalWindows.isEmpty() )
            modalWindows.getFromEnd()->onKeyState( key, state, character );
        else
            Container::onKeyState( key, state, character );
    }

    bool Gui::onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse )
    {
        if ( !modalWindows.isEmpty() )
            return modalWindows.getFromEnd()->onMouseButton( button, pressed, mouse );
        else
            return Container::onMouseButton( button, pressed, mouse );
    }

    void Gui::onMouseMoveTo( const Vector2<int>& mouse )
    {
        mousePos = mouse;

        iterate ( widgets )
            widgets.current()->onMouseMoveTo( mouse );

        iterate ( modalWindows )
            modalWindows.current()->onMouseMoveTo( mouse );
    }

    void Gui::onRender()
    {
        getGraphicsDriver()->set2dMode( 1.0f, -1.0f );

        iterate ( widgets )
            widgets.current()->render();

        iterate ( modalWindows )
            modalWindows.current()->render();
    }

    void Gui::onUpdate( double delta )
    {
        iterate ( widgets )
            widgets.current()->update( delta );
    }

    void Gui::pushModal( IWidget* window )
    {
        modalWindows.add( window );
    }

    void Gui::remove( IWidget* widget )
    {
        if ( modalWindows.removeItem( widget ) )
            return;

        widgets.removeItem( widget );
    }
}
