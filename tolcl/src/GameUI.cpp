
#include "UI.hpp"

namespace GameUI
{
    using namespace GameClient;

    static const Colour buttonTextColour( 0.1f, 0.2f, 0.5f );
    static const Colour inputTextColour( 1.0f, 1.0f, 1.0f );
    static const Colour inputBgColour( 0.0f, 0.0f, 0.0f, 0.5f );
    static const Colour inputActiveBgColour( 0.0f, 0.0f, 0.0f, 0.75f );
    static const Colour windowTitleColour( 1.0f, 1.0f, 1.0f );

    EventListener::~EventListener()
    {
    }

    InputFilter::~InputFilter()
    {
    }

    Widget::Widget( int x, int y ) : x( x ), y( y ), modalParent( 0 )
    {
    }

    Widget::~Widget()
    {
    }

    void Widget::move( int x, int y )
    {
        this->x += x;
        this->y += y;
    }

    Widget* Widget::findWidget( const char* name )
    {
        if ( getName() == name )
            return this;

        return 0;
    }

    bool Widget::keyDown( li::Utf8Char key )
    {
        return false;
    }

    bool Widget::mouseDown( int x, int y )
    {
        return false;
    }

    bool Widget::mouseMove( int x, int y )
    {
        return false;
    }

    bool Widget::mouseUp( int x, int y )
    {
        return false;
    }

    void Widget::render()
    {
    }

    StaticImage::StaticImage( int x, int y, StormGraph::Texture* texture, int w, int h )
            : Widget( x, y ), texture( texture ), w( w ), h( h )
    {
    }

    StaticImage::~StaticImage()
    {
        if ( texture )
            texture->release();
    }

    void StaticImage::render()
    {
        if ( texture )
        {
            if ( w > 0 && h > 0 )
                texture->render2D( x, y, w, h );
            else
                texture->render2D( x, y );
        }
    }

    StaticImage* StaticImage::setTexture( Texture* texture )
    {
        if ( this->texture )
            this->texture->release();

        this->texture = texture;
        return this;
    }

    Label::Label( int x, int y, const char* text, const char* fontName, StormGraph::Colour colour )
            : Widget( x, y ), text( text ), colour( colour )
    {
        font = globalResMgr->getNamedFont( fontName );
    }

    Label::~Label()
    {
    }

    void Label::setText( const char* text )
    {
        this->text = text;
    }

    void Label::render()
    {
        font->render( x, y, text, colour );
    }

    Button::Button( int x, int y, int w, int h, const char* text )
            : Widget( x, y ), w( w ), h( h ), onPush( 0 )
    {
        buttonTexture = globalResMgr->getTexture( "tolcl/gfx/button-2.0.png" );

        labelTexture = globalResMgr->getNamedFont( "ui" )->render( text, buttonTextColour );
        labelTexture->centerOrigin();
    }

    Button::~Button()
    {
        buttonTexture->release();

        if ( labelTexture )
            labelTexture->release();
    }

    bool Button::mouseDown( int x, int y )
    {
        if ( x >= this->x && y >= this->y && x < this->x + this->w && y < this->y + this->h )
        {
            if ( onPush )
                onPush->uiEvent( this, "push" );

            return true;
        }

        return false;
    }

    void Button::render()
    {
        buttonTexture->render2D( x, y, w, h );

        if ( labelTexture )
            labelTexture->render2D( x + w / 2, y + h / 2 );
    }

    void Button::setOnPush( EventListener* listener )
    {
        onPush = listener;
    }

    GraphicalButton::GraphicalButton( int x, int y, int w, int h, Texture* texture )
            : Widget( x, y ), w( w ), h( h ), texture( texture ), onPush( 0 )
    {
    }

    GraphicalButton::~GraphicalButton()
    {
        texture->release();
    }

    bool GraphicalButton::mouseDown( int x, int y )
    {
        if ( x >= this->x && y >= this->y && x < this->x + this->w && y < this->y + this->h )
        {
            if ( onPush )
                onPush->uiEvent( this, "push" );

            return true;
        }

        return false;
    }

    void GraphicalButton::render()
    {
        texture->render2D( x, y, w, h );
    }

    void GraphicalButton::setOnPush( EventListener* listener )
    {
        onPush = listener;
    }

    Input::Input( int x, int y, int w, int h )
            : Widget( x, y ), w( w ), h( h ), filter( 0 ), active( false )
    {
        font = globalResMgr->getNamedFont( "ui" );
    }

    Input::~Input()
    {
    }

    bool Input::keyDown( li::Utf8Char key )
    {
        if ( active )
        {
            if ( key == 0x08 )
                setText( text.dropRightPart( 1 ) );
            else if ( key >= 0x20 )
                setText( text + Utf8Character( key ) );

            return true;
        }

        return false;
    }

    bool Input::mouseDown( int x, int y )
    {
        if ( x >= this->x && y >= this->y && x < this->x + this->w && y < this->y + this->h )
        {
            active = true;
            return true;
        }

        return false;
    }

    bool Input::mouseUp( int x, int y )
    {
        if ( !( x >= this->x && y >= this->y && x < this->x + this->w && y < this->y + this->h ) )
            active = false;

        return false;
    }

    void Input::render()
    {
        sg->drawRect( Vector<float>( x, y, 0 ), Vector<float>( x + w, y + h, 0 ), active ? inputActiveBgColour : inputBgColour );

        font->render( x + 4, y + h / 2, text, inputTextColour, Align::middle );
    }

    void Input::setText( const String& text )
    {
        if ( !filter || filter->isValid( this, text ) )
            this->text = text;
    }

    List::List( int x, int y, int w, int h )
            : Widget( x, y ), w( w ), h( h )
    {
        font = globalResMgr->getNamedFont( "ui" );
    }

    List::~List()
    {
    }

    unsigned List::addItem( const String& item )
    {
        return items.add( item );
    }

    bool List::mouseDown( int x, int y )
    {
        /*TO DO
        if ( x >= this->x && y >= this->y && x < this->x + this->w && y < this->y + this->h )
        {
            active = true;
            return true;
        }*/

        return false;
    }

    void List::render()
    {
        sg->drawRect( Vector<float>( x, y, 0 ), Vector<float>( x + w, y + h, 0 ), defaultPanelColour );

        iterate ( items )
            font->render( x + 4, y + 4 + items.iter() * font->getLineSkip(), items.current() );
    }

    Panel::Panel( int x, int y, int w, int h, bool visible, StormGraph::Colour colour )
            : Widget( x, y ), modal( 0 ), w( w ), h( h ), enabled( true ), visible( visible ), colour( colour ), onClick( 0 )
    {
    }

    Panel::~Panel()
    {
        iterate ( widgets )
            delete widgets.current();
    }

    void Panel::addWidget( Widget* widget )
    {
        widget->move( x, y );
        widgets.add( widget );
    }

    void Panel::disable()
    {
        enabled = false;
    }

    void Panel::doModal( Widget* widget )
    {
        modal = widget;
        modal->setModalParent( this );
    }

    void Panel::enable()
    {
        enabled = true;
    }

    Widget* Panel::findWidget( const char* name )
    {
        if ( getName() == name )
            return this;

        if ( modal && modal->getName() == name )
            return modal;

        reverse_iterate ( widgets )
        {
            Widget* widget = widgets.current()->findWidget( name );

            if ( widget )
                return widget;
        }

        return 0;
    }

    bool Panel::keyDown( li::Utf8Char key )
    {
        if ( !enabled )
            return false;

        if ( modal )
        {
            modal->keyDown( key );
            return true;
        }

        reverse_iterate ( widgets )
        {
            if ( widgets.current()->keyDown( key ) )
                return true;
        }

        return false;
    }

    bool Panel::mouseDown( int x, int y )
    {
        if ( !enabled )
            return false;

        if ( modal )
        {
            modal->mouseDown( x, y );
            return true;
        }

        if ( x >= this->x && y >= this->y && x < this->x + this->w && y < this->y + this->h )
        {
            reverse_iterate ( widgets )
            {
                if ( widgets.current()->mouseDown( x, y ) )
                    return true;
            }

            if ( visible )
            {
                if ( onClick )
                    onClick->uiEvent( this, "click" );

                return true;
            }
        }

        return false;
    }

    bool Panel::mouseMove( int x, int y )
    {
        if ( !enabled )
            return false;

        if ( modal )
        {
            modal->mouseMove( x, y );
            return true;
        }

        reverse_iterate ( widgets )
        {
            if ( widgets.current()->mouseMove( x, y ) )
                return true;
        }

        return false;
    }

    bool Panel::mouseUp( int x, int y )
    {
        if ( !enabled )
            return false;

        if ( modal )
        {
            modal->mouseUp( x, y );
            return true;
        }

        reverse_iterate ( widgets )
        {
            if ( widgets.current()->mouseUp( x, y ) )
                return true;
        }

        return false;
    }

    void Panel::move( int x, int y )
    {
        Widget::move( x, y );

        iterate ( widgets )
            widgets.current()->move( x, y );
    }

    bool Panel::removeWidget( Widget* widget )
    {
        if ( widget == modal )
        {
            modal->setModalParent( 0 );
            modal = 0;
            return true;
        }
        else
            return widgets.removeItem( widget );
    }

    void Panel::render()
    {
        if ( !enabled )
            return;

        if ( visible )
            sg->drawRect( Vector<float>( x, y ), Vector<float>( x + w, y + h ), colour );

        iterate ( widgets )
            widgets.current()->render();

        if ( modal )
        {
            sg->drawRect( Vector<float>( x, y ), Vector<float>( x + w, y + h ), StormGraph::Colour( 1.0f, 1.0f, 1.0f, 0.3f ) );
            modal->render();
        }
    }

    void Panel::setOnClick( EventListener* listener )
    {
        onClick = listener;
    }

    UI::UI( int w, int h )
            : Panel( 0, 0, w, h )
    {
    }

    UI::~UI()
    {
        iterate ( removalQueue )
            delete removalQueue.current();
    }

    void UI::destroyWidget( Widget* widget )
    {
        removeWidget( widget );
        removalQueue.add( widget );
    }

    void UI::focus( Widget* widget )
    {
        focusQueue.add( widget );
    }

    void UI::hideTooltip()
    {
        tooltip.clear();
    }

    void UI::overlay( Widget* widget )
    {
        overlays.add( widget );
    }

    void UI::render()
    {
        Panel::render();

        if ( !tooltip.isEmpty() )
        {
            sg->drawRect( Vector<float>( tooltipX, tooltipY ), Vector<float>( tooltipX + tooltipText.width + 20, tooltipY + tooltipText.height + 20 ), defaultPanelColour );
            globalResMgr->getNamedFont( "ui" )->render( tooltipX + 10, tooltipY + 10, tooltipText );
        }
    }

    void UI::showTooltip( int x, int y, const char* text )
    {
        tooltipX = x;
        tooltipY = y;
        tooltip = text;

        tooltipText = globalResMgr->getNamedFont( "ui" )->layout( "\\l" + tooltip );
    }

    void UI::update()
    {
        iterate ( focusQueue )
        {
            if ( removeWidget( focusQueue.current() ) )
                addWidget( focusQueue.current() );
        }

        focusQueue.clear();

        iterate ( overlays )
        {
            removeWidget( overlays.current() );
            addWidget( overlays.current() );
        }

        reverse_iterate ( removalQueue )
        {
            Widget* widget = removalQueue.current();
            removalQueue.remove( removalQueue.iter() );
            delete widget;
        }
    }

    Window::Window( UI* ui, const char* title, int x, int y, int w, int h, bool visible, StormGraph::Colour colour )
            : Panel( x, y + 20, w, h, visible, colour ), ui( ui ), dragging( false )
    {
        font = globalResMgr->getNamedFont( "ui" );
        windowTitle = font->layout( title, windowTitleColour, Align::left | Align::bottom );
    }

    Window::~Window()
    {
    }

    void Window::hide()
    {
        disable();
    }

    bool Window::mouseDown( int x, int y )
    {
        if ( !enabled )
            return false;

        if ( ui && x >= this->x && y >= this->y - 20 && x < this->x + this->w && y < this->y + this->h )
            ui->focus( this );

        if ( x >= this->x && y >= this->y - 20 && x < this->x + this->w && y < this->y )
        {
            dragX = x;
            dragY = y;
            dragging = true;
            return true;
        }

        return Panel::mouseDown( x, y );
    }

    bool Window::mouseMove( int x, int y )
    {
        if ( !enabled )
            return false;

        if ( dragging )
        {
            move( x - dragX, y - dragY );
            dragX = x;
            dragY = y;
        }

        return Panel::mouseMove( x, y );
    }

    bool Window::mouseUp( int x, int y )
    {
        if ( !enabled )
            return false;

        if ( dragging )
            dragging = false;

        return Panel::mouseUp( x, y );
    }

    void Window::render()
    {
        if ( !enabled )
            return;

        Panel::render();

        font->render( x + 4, y - 2, windowTitle );
    }

    void Window::show()
    {
        enable();
    }

    MessageBox::MessageBox( const char* text, const char* title, int x, int y, int w, int h )
            : Window( 0, title, x, y, w, h ), onClose( 0 )
    {
        addWidget( new Label( 20, 20, text ) );

        Button* okButton = new Button( w - 80, h - 40, 64, 24, "close" );
        okButton->setName( "ok_btn" );
        okButton->setOnPush( this );
        addWidget( okButton );
    }

    MessageBox::~MessageBox()
    {
    }

    void MessageBox::setOnClose( EventListener* listener )
    {
        onClose = listener;
    }

    void MessageBox::uiEvent( Widget* widget, const li::String& event )
    {
        if ( widget->getName() == "ok_btn" && event == "push" && onClose )
            onClose->uiEvent( this, "close" );
    }
}
