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

#include <StormGraph/Engine.hpp>

#include <Radiance/Radiance.hpp>
#include <Radiance/Styler.hpp>

namespace Radiance
{
    Widget::Widget()
            : visible( false )
    {
    }

    Widget::Widget( WidgetStyle* style, const Vector<float>& pos, const Vector<float>& size )
            : style( style ), visible( true ), pos( pos ), size( size )
    {
    }

    Widget::~Widget()
    {
    }

    Widget* Widget::findWidget( const char* name )
    {
        return 0;
    }

    WidgetStyle* Widget::getStyle()
    {
        return style;
    }

    void Widget::hide()
    {
        visible = false;
    }

    bool Widget::mouseDown( const Vector<float>& mouse )
    {
        return false;
    }

    bool Widget::mouseMove( const Vector<float>& mouse )
    {
        if ( style )
            style->onMouseMove( mouse, pos );

        return false;
    }

    bool Widget::mouseUp( const Vector<float>& mouse )
    {
        return false;
    }

    void Widget::move( const Vector<float>& vec )
    {
        pos += vec;
    }

    void Widget::moveTo( const Vector<float>& vec )
    {
        pos = vec;
    }

    bool Widget::onKeyState( Unicode::Char character, Key::State state )
    {
        return false;
    }

    void Widget::render( const Vector<float>& offset )
    {
        if ( visible )
        {
            style->beginRender( pos + offset );
            style->endRender();
        }
    }

    void Widget::setTop( bool top )
    {
    }

    void Widget::show()
    {
        visible = true;
    }

    void Widget::update( double delta )
    {
        if ( style )
            style->onUpdate( delta );
    }

    LinearAnimator::LinearAnimator( Animatable* animatable, unsigned propertyName )
            : animatable( animatable ), propertyName( propertyName ), running( false ), onAnimationEnd( 0 )
    {
    }

    void LinearAnimator::animate( float begin, float end, float speed )
    {
        running = true;

        if ( end > begin )
        {
            this->begin = begin;
            this->end = end;
            this->speed = speed;
            value = begin;
        }
        else
        {
            this->begin = end;
            this->end = begin;
            this->speed = -speed;
            value = begin;
        }
    }

    void LinearAnimator::update( double delta )
    {
        if ( !running )
            return;

        value += speed * delta;

        if ( value >= end )
        {
            value = end;
            running = false;
        }
        else if ( value <= begin )
        {
            value = begin;
            running = false;
        }

        animatable->animateProperty( propertyName, value );

        if ( !running && onAnimationEnd )
            onAnimationEnd->onRadianceEvent( this, "animationEnd", 0 );
    }

    void LinearAnimator::setOnAnimationEnd( EventListener* listener )
    {
        onAnimationEnd = listener;
    }

    Button::Button( Styler* styler, const Vector<float>& pos, const Vector<float>& size, const String& label )
            : Widget( styler->createWidgetStyle( WidgetStyle::button, size ), pos, size )
    {
        style->setProperty( WidgetStyle::text, label );
    }

    Button::~Button()
    {
    }

    bool Button::mouseUp( const Vector<float>& mouse )
    {
        Widget::mouseUp( mouse );

        if ( onPush && mouse.x >= pos.x && mouse.y >= pos.y && mouse.x < pos.x + size.x && mouse.y < pos.y + size.y )
        {
            onPush->onRadianceEvent( this, "push", 0 );
            return true;
        }

        return false;
    }

    void Button::setOnPush( EventListener* listener )
    {
        onPush = listener;
    }

    Image::Image( Styler* styler, const Vector<float>& pos, cfx2::Node& description )
            : Widget( styler->createWidgetStyle( WidgetStyle::image, Vector<>() ), pos, Vector<>() )
    {
        cfx2::Node image = description.findChild( "RadianceImage" );

        if ( !image )
            throw StormGraph::Exception( "Radiance.Image.Image", "ImageDescriptionError", "Missing required node 'RadianceImage' in image description." );

        style->setProperty( WidgetStyle::textureName, image.getAttrib( "texture" ) );
        style->setProperty( WidgetStyle::origin, Vector<>( image.getAttrib( "origin" ) ) );

        if ( image.getAttrib( "scale" ) )
            style->setProperty( WidgetStyle::scale, Vector<>( image.getAttrib( "scale" ) ) );
    }

    Image::~Image()
    {
    }

    float Image::getRotation() const
    {
        return rotation;
    }

    void Image::setRotation( float angle )
    {
        rotation = angle;
        style->setProperty( WidgetStyle::rotation, rotation );
    }

    Input::Input( Styler* styler, const Vector<float>& pos, const Vector<float>& size, Type type )
            : Widget( styler->createWidgetStyle( WidgetStyle::input, size ), pos, size ), selected( false ), down( false )
    {
        style->setProperty( WidgetStyle::inputType, type );

        backspace = 0x08;
    }

    Input::~Input()
    {
    }

    const String& Input::getText()
    {
        return text;
    }

    bool Input::mouseDown( const Vector<float>& mouse )
    {
        Widget::mouseDown( mouse );

        down = true;
        updateActive( mouse );
        return false;
    }

    bool Input::mouseMove( const Vector<float>& mouse )
    {
        Widget::mouseMove( mouse );

        if ( down )
            updateActive( mouse );

        return false;
    }

    bool Input::mouseUp( const Vector<float>& mouse )
    {
        Widget::mouseUp( mouse );

        down = false;
        updateActive( mouse );
        return false;
    }

    bool Input::onKeyState( Unicode::Char character, Key::State state )
    {
        if ( selected && state == Key::pressed )
        {
            if ( character == backspace )
                setText( text.dropRightPart( 1 ) );
            else if ( character >= ' ' )
                setText( text + Unicode::Character( character ) );
            else
                return false;

            return true;
        }

        return false;
    }

    void Input::setText( const String& text )
    {
        this->text = text;
        style->setProperty( WidgetStyle::text, text );
    }

    bool Input::updateActive( const Vector<float>& mouse )
    {
        selected = ( mouse.x >= pos.x && mouse.y >= pos.y && mouse.x < pos.x + size.x && mouse.y < pos.y + size.y );
        style->setProperty( WidgetStyle::selected, selected );
        return selected;
    }

    Label::Label( Styler* styler, const Vector<float>& pos, const String& text, uint16_t align )
            : Widget( styler->createWidgetStyle( WidgetStyle::label, Vector<>() ), pos, Vector<>() )
    {
        style->setProperty( WidgetStyle::align, align );
        setText( text );
    }

    Label::~Label()
    {
    }

    void Label::setText( const String& text )
    {
        style->setProperty( WidgetStyle::text, text );
    }

    Panel::Panel( Styler* styler, const Vector<float>& pos, const Vector<float>& size )
            : Widget( styler->createWidgetStyle( WidgetStyle::panel, size ), pos, size ), onClick( 0 )
    {
    }

    Panel::Panel( WidgetStyle* style, const Vector<float>& pos, const Vector<float>& size )
            : Widget( style, pos, size ), onClick( 0 )
    {
    }

    Panel::~Panel()
    {
        iterate ( widgets )
            delete widgets.current();
    }

    void Panel::add( Widget* widget )
    {
        widgets.add( widget );
    }

    void Panel::animateProperty( unsigned name, float value )
    {
        switch ( name )
        {
            case x: pos.x = value; break;
            case y: pos.y = value; break;
            case sizeX: size.x = value; break;
            case sizeY: size.y = value; break;
        }
    }

    Widget* Panel::findWidget( const char* name )
    {
        iterate ( widgets )
            if ( widgets.current()->getName() == name )
                return widgets.current();

        return 0;
    }

    bool Panel::mouseDown( const Vector<float>& mouse )
    {
        if ( mouse.x >= pos.x && mouse.y >= pos.y && mouse.x < pos.x + size.x && mouse.y < pos.y + size.y )
        {
            reverse_iterate ( widgets )
            {
                if ( widgets.current()->mouseDown( mouse - pos ) )
                    return true;
            }

            if ( visible )
            {
                if ( onClick )
                    onClick->onRadianceEvent( this, "click", 0 );

                return true;
            }
        }

        return false;
    }

    bool Panel::mouseMove( const Vector<float>& mouse )
    {
        if ( !visible )
            return false;

        reverse_iterate ( widgets )
            if ( widgets.current()->mouseMove( mouse - pos ) )
                return true;

        return false;
    }

    bool Panel::mouseUp( const Vector<float>& mouse )
    {
        /*if ( modal )
        {
            modal->mouseDown( x, y );
            return true;
        }*/

        if ( mouse.x >= pos.x && mouse.y >= pos.y && mouse.x < pos.x + size.x && mouse.y < pos.y + size.y )
        {
            reverse_iterate ( widgets )
            {
                if ( widgets.current()->mouseUp( mouse - pos ) )
                    return true;
            }
        }

        return false;
    }

    bool Panel::onKeyState( Unicode::Char character, Key::State state )
    {
        reverse_iterate ( widgets )
        {
            if ( widgets.current()->onKeyState( character, state ) )
                return true;
        }

        return false;
    }

    void Panel::render( const Vector<float>& offset )
    {
        if ( !visible )
            return;

        Vector<float> subOffset = pos + offset + style->beginRender( pos + offset );

        iterate ( widgets )
            widgets.current()->render( subOffset );

        style->endRender();
    }

    void Panel::update( double delta )
    {
        Widget::update( delta );

        reverse_iterate ( widgets )
            widgets.current()->update( delta );
    }

    Window::Window( Styler* styler, const Vector<float>& pos, const Vector<float>& size, const String& title )
            : Panel( styler->createWidgetStyle( WidgetStyle::window, size ), pos, size ), dragging( false )
    {
        style->setProperty( WidgetStyle::title, title );
    }

    Window::~Window()
    {
    }

    bool Window::mouseDown( const Vector<float>& mouse )
    {
        if ( !visible )
            return false;

        down = true;

        if ( uiPtr && mouse.x >= pos.x - 8.0f && mouse.y >= pos.y - 32.0f && mouse.x < pos.x + size.x + 8.0f && mouse.y < pos.y + size.y )
            uiPtr->focus( this );

        if ( mouse.x >= pos.x - 8.0f && mouse.y >= pos.y - 32.0f && mouse.x < pos.x + size.x + 8.0f && mouse.y < pos.y )
        {
            drag = mouse;
            dragging = true;
            return true;
        }

        return Panel::mouseDown( mouse );
    }

    bool Window::mouseMove( const Vector<float>& mouse )
    {
        if ( !visible )
            return false;

        if ( dragging )
        {
            move( mouse - drag );
            drag = mouse;
        }

        return Panel::mouseMove( mouse );
    }

    bool Window::mouseUp( const Vector<float>& mouse )
    {
        if ( !visible )
            return false;

        if ( dragging )
            dragging = false;

        down = false;

        return Panel::mouseUp( mouse );
    }

    void Window::render( const Vector<float>& offset )
    {
        return Panel::render( offset );
    }

    void Window::show()
    {
        Widget::show();

        uiPtr->focus( this );
        selected = true;
    }

    void Window::setTop( bool top )
    {
        style->setProperty( WidgetStyle::selected, top );
    }

    MessageBox::MessageBox( Styler* styler, const Vector<float>& pos, const Vector<float>& size, const String& title, const String& text )
            : Window( styler, pos, size, title )
    {
        add( new Label( styler, Vector<>( 20.0f, 20.0f ), text, IFont::left | IFont::top ) );

        Object<Button> closeBtn = new Button( styler, Vector<>( size.x - 112.0f - 20.0f, size.y - 36.0f - 20.0f ), Vector<>( 112.0f, 36.0f ), "Well then" );
        closeBtn->setName( "closeBtn" );
        closeBtn->setOnPush( this );
        add( closeBtn.detach() );
    }

    MessageBox::~MessageBox()
    {
    }

    void MessageBox::onRadianceEvent( Widget* widget, const String& eventName, void* eventProperties )
    {
        if ( widget->getName() == "closeBtn" && eventName == "push" && onClose )
            onClose->onRadianceEvent( this, "close", eventProperties );
    }

    void MessageBox::setOnClose( EventListener* listener )
    {
        onClose = listener;
    }

    UI::UI( Styler* styler, const Vector<float>& pos, const Vector<float>& size )
            : Panel( styler->createWidgetStyle( WidgetStyle::ui, size ), pos, size ), styler( styler )
    {
    }

    UI::~UI()
    {
    }

    void UI::add( Widget* widget )
    {
        widget->setUI( this );

        Panel::add( widget );
    }

    void UI::doModal( Window* window )
    {
        modal = window;

        if ( modal )
        {
            modal->setUI( this );
            modal->show();

            style->setProperty( WidgetStyle::hasModal, true );
        }
        else
            style->setProperty( WidgetStyle::hasModal, false );
    }

    void UI::focus( Widget* widget )
    {
        focusQueue.add( widget );
    }

    Widget* UI::load( const char* fileName, EventListener* listener, cfx2::Node node )
    {
        String type = node.getName();
        Widget* widget = 0;
        Panel* panel = 0;

        if ( type == "Button" )
        {
            Button* button = new Button( styler, Vector<>( node.getAttrib( "pos" ) ), Vector<>( node.getAttrib( "size" ) ), node.getAttrib( "text" ) );
            button->setOnPush( listener );

            widget = button;
        }
        else if ( type == "Image" )
        {
            cfx2::Document description = Engine::getInstance()->loadCfx2Asset( node.getAttrib( "description" ) );

            widget = new Image( styler, Vector<>( node.getAttrib( "pos" ) ), description );
        }
        else if ( type == "Input" )
        {
            Input::Type inputType = Input::plain;

            if ( ( String ) node.getAttrib( "type" ) == "password" )
                inputType = Input::password;

            widget = new Input( styler, Vector<>( node.getAttrib( "pos" ) ), Vector<>( node.getAttrib( "size" ) ), inputType );
        }
        else if ( type == "Label" )
            widget = new Label( styler, Vector<>( node.getAttrib( "pos" ) ), node.getAttrib( "text" ), strtoul( node.getAttrib( "align" ), 0, 0 ) );
        else if ( type == "Panel" )
        {
            panel = new Panel( styler, Vector<>( node.getAttrib( "pos" ) ), Vector<>( node.getAttrib( "size" ) ) );

            widget = panel;
        }
        else if ( type == "Window" )
        {
            panel = new Window( styler, Vector<>( node.getAttrib( "pos" ) ), Vector<>( node.getAttrib( "size" ) ), node.getAttrib( "title" ) );

            widget = panel;
        }

        if ( !widget )
            throw StormGraph::Exception( "Radiance.UI.load", "InvalidWidgetType", "Unrecognized widget class `" + type + "` in `" + fileName + "`" );

        if ( panel )
        {
            Object<cfx2::List> children = node.getChildren();

            for each_in_list_ptr ( children, i )
                panel->add( load( fileName, listener, children->get( i ) ) );
        }

        const char* name = node.getText();

        if ( name )
            widget->setName( name );

        const char* visible = node.getAttrib( "visible" );

        if ( visible && !strtoul( visible, 0, 0 ) )
            widget->hide();

        return widget;
    }

    void UI::load( const char* fileName, EventListener* listener )
    {
        cfx2::Document doc = Engine::getInstance()->loadCfx2Asset( fileName );

        iterate ( doc )
            add( load( fileName, listener, doc.current() ) );
    }

    bool UI::mouseDown( const Vector<float>& mouse )
    {
        if ( modal )
        {
            modal->mouseDown( mouse - pos );
            return true;
        }

        return Panel::mouseDown( mouse );
    }

    bool UI::mouseMove( const Vector<float>& mouse )
    {
        if ( modal )
        {
            modal->mouseMove( mouse - pos );
            return true;
        }

        return Panel::mouseMove( mouse );
    }

    bool UI::mouseUp( const Vector<float>& mouse )
    {
        if ( modal )
        {
            modal->mouseUp( mouse - pos );
            return true;
        }

        return Panel::mouseUp( mouse );
    }

    bool UI::remove( Widget* widget )
    {
        if ( widget == modal )
        {
            modal.release();
            doModal( 0 );
            return true;
        }
        else
            return widgets.removeItem( widget );
    }

    void UI::render()
    {
        iterate ( widgets )
            widgets.current()->render( Vector<>() );

        style->renderStage( WidgetStyle::beforeModal );

        if ( modal )
        {
            style->renderStage( WidgetStyle::renderModal );
            modal->render( Vector<>() );
        }
    }

    void UI::setTop( Widget* widget )
    {
        if ( widget != currentTop )
        {
            if ( currentTop )
                currentTop->setTop( false );

            currentTop = widget;

            if ( currentTop )
                currentTop->setTop( true );
        }
    }

    void UI::update( double delta )
    {
        iterate ( focusQueue )
        {
            if ( widgets.removeItem( focusQueue.current() ) )
                add( focusQueue.current() );
        }

        focusQueue.clear();

        if ( modal )
        {
            setTop( modal );
            modal->update( delta );
        }
        else if ( !widgets.isEmpty() )
            setTop( widgets.getFromEnd() );
        else
            setTop( 0 );

        Panel::update( delta );
    }
}
