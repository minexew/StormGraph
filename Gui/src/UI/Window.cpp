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
    static const float titleHeight = 14.0f;

    static const Vector2<> closeButtonSize( 8.0f, 8.0f );
    static const Vector2<> closeButtonPadding( ( titleHeight - closeButtonSize.x ) / 2, ( titleHeight - closeButtonSize.y ) / 2 );

    Window::Window( Gui* gui, const Vector<float>& pos, const Vector<float>& size, const char* title )
            : Panel( gui, pos, size ), closeButton( true ), resizable( true ), drag( false ), resize( false ), gui( gui ), titleLayout( nullptr )
    {
        font = gui->getTextFont();

        setTitle( title );
    }

    Window::~Window()
    {
        font->releaseText( titleLayout );
    }

    void Window::add( IChildWidget* widget )
    {
        Panel::add( widget );
    }

    Vector<float> Window::getMinSize()
    {
        return Panel::getMinSize().maximum( Vector2<>( titleHeight, titleHeight ) );
    }

    const char* Window::getTitle()
    {
        return title;
    }

    void Window::move( const Vector<float>& vec )
    {
        Panel::move( vec );
    }

    void Window::onKeyState( int16_t key, Key::State state, Unicode::Char character )
    {
        Panel::onKeyState( key, state, character );
    }

    bool Window::onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse )
    {
        if ( pressed )
        {
            if ( !( mouse < Vector2<int>( realPos.x, realPos.y - titleHeight ) || mouse > Vector2<int>( realPos.x + realSize.x, realPos.y ) ) )
            {
                if ( button == MouseButton::left )
                {
                    dragFrom = mouse;
                    drag = true;
                }

                return true;
            }
            else if ( !( mouse < realPos + realSize - Vector2<float>( 12.0f, 12.0f ) || mouse > realPos + realSize - Vector2<float>( 4.0f, 4.0f ) ) )
            {
                if ( button == MouseButton::left )
                {
                    dragFrom = mouse;
                    resize = true;
                }

                return true;
            }
        }
        else
        {
            if ( button == MouseButton::left )
            {
                drag = false;
                resize = false;
            }

            return false;
        }

        if ( mouse < realPos || mouse > realPos + realSize )
            return false;

        Panel::onMouseButton( button, pressed, mouse );
        return true;
    }

    void Window::onMouseMoveTo( const Vector2<int>& mouse )
    {
        if ( drag )
        {
            move( mouse - dragFrom );
            dragFrom = mouse;
        }
        else if ( resize )
        {
            size += mouse - dragFrom;
            dragFrom = mouse;

            layout();
        }

        Panel::onMouseMoveTo( mouse );
    }

    void Window::setAlign( unsigned align )
    {
        Panel::setAlign( align );
    }

    void Window::setBounds( const Vector<float>& pos, const Vector<float>& size )
    {
        Panel::setBounds( pos, size );
    }

    void Window::setExpand( bool expand )
    {
        Panel::setExpand( expand );
    }

    void Window::setTitle( const char* title )
    {
        font->releaseText( titleLayout );
        titleLayout = font->layoutText( title, Colour::white(), IFont::centered | IFont::middle );

        this->title = title;
    }

    void Window::showModal()
    {
        gui->pushModal( ( IWindow* ) this );
    }

    void Window::render()
    {
        Panel::render();

        if ( closeButton )
            graphicsDriver->drawRectangle( realPos + Vector2<float>( realSize.x - closeButtonSize.x - closeButtonPadding.x, - closeButtonSize.y - closeButtonPadding.y ),
                    closeButtonSize, Colour( 0.9f, 0.0f, 0.0f, 0.9f ), nullptr );

        if ( resizable )
            graphicsDriver->drawRectangle( realPos + realSize - Vector2<float>( 12.0f, 12.0f ), Vector2<float>( 8.0f, 8.0f ), Colour::grey( 0.8f, 0.2f ), nullptr );

        graphicsDriver->drawRectangle( Vector<float>( realPos.x, realPos.y - titleHeight ), Vector2<float>( realSize.x, titleHeight ), Colour::grey( 0.2f, 0.6f ), nullptr );

#ifdef li_GCC4
        graphicsDriver->pushClippingRect( ScreenRect { Vector2<uint16_t>( realPos.x, realPos.y - titleHeight ), Vector2<uint16_t>( realSize.x, titleHeight ) } );
#else
        ScreenRect sr = { Vector2<uint16_t>( realPos.x, realPos.y - titleHeight ), Vector2<uint16_t>( realSize.x, titleHeight ) };
        graphicsDriver->pushClippingRect( sr );
#endif

        font->renderText( realPos.x + realSize.x / 2, realPos.y - titleHeight / 2, titleLayout );
        graphicsDriver->popClippingRect();
    }

    void Window::update( double delta )
    {
        Panel::update( delta );
    }
}

