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

#pragma once

#include <StormGraph/GraphicsDriver.hpp>

#include <littl/cfx2.hpp>

#if defined( __li_MSW )
#if defined( Radiance_Build_DLL )
#define RadianceClass class __declspec( dllexport )
#elif defined( Radiance_DLL )
#define RadianceClass class __declspec( dllimport )
#endif
#endif

#ifndef RadianceClass
#define RadianceClass class
#endif

namespace Radiance
{
    using namespace StormGraph;

    class Styler;
    class UI;
    class Widget;
    class WidgetStyle;

    RadianceClass Animatable : public ReferencedClass
    {
        public:
            virtual ~Animatable();

            virtual void animateProperty( unsigned name, float value ) = 0;
    };

    RadianceClass EventListener
    {
        public:
            virtual ~EventListener();

            virtual void onRadianceEvent( Widget* widget, const String& eventName, void* eventProperties ) = 0;
    };

    RadianceClass Widget
    {
        public:
            UI* uiPtr = nullptr;
            String name;
            Reference<WidgetStyle> style;

            bool visible;
            Vector<> pos, size;

        public:
            Widget();
            Widget( WidgetStyle* style, const Vector<float>& pos, const Vector<float>& size );
            virtual ~Widget();

            virtual Widget* findWidget( const char* name );
            const String& getName() const { return name; }
            const Vector<>& getSize() const { return size; }
            WidgetStyle* getStyle();
            void hide();
            virtual bool mouseDown( const Vector<float>& mouse );
            virtual bool mouseMove( const Vector<float>& mouse );
            virtual bool mouseUp( const Vector<float>& mouse );
            virtual void move( const Vector<float>& vec );
            virtual void moveTo( const Vector<float>& vec );
            virtual bool onKeyState( Unicode::Char character, Key::State state );
            virtual void render( const Vector<float>& offset );
            void setName( const char* name ) { this->name = name; }
            virtual void setTop( bool top );
            void setUI( UI* ui ) { uiPtr = ui; }
            virtual void show();
            virtual void update( double delta );
    };

    RadianceClass LinearAnimator : public Widget
    {
        Animatable* animatable;
        unsigned propertyName;

        bool running;
        float begin, end, speed, value;

        EventListener* onAnimationEnd;

        public:
            LinearAnimator( Animatable* animatable, unsigned propertyName );

            void animate( float begin, float end, float speed );
            bool isRunning() const { return running; }
            void update( double delta );
            void setOnAnimationEnd( EventListener* listener );
    };

    RadianceClass Button : public Widget
    {
        protected:
            EventListener* onPush = nullptr;

        public:
            Button( Styler* styler, const Vector<float>& pos, const Vector<float>& size, const String& label );
            virtual ~Button();

            virtual bool mouseUp( const Vector<float>& mouse );
            void setOnPush( EventListener* listener );
    };

    RadianceClass Input : public Widget
    {
        public:
            enum Type { plain, password };

        protected:
            bool selected, down;

            String text;
            Unicode::Char backspace;

            void setText( const String& text );
            bool updateActive( const Vector<float>& mouse );

        public:
            Input( Styler* styler, const Vector<float>& pos, const Vector<float>& size, Type type = plain );
            virtual ~Input();

            const String& getText();
            virtual bool mouseDown( const Vector<float>& mouse );
            virtual bool mouseMove( const Vector<float>& mouse );
            virtual bool mouseUp( const Vector<float>& mouse );
            virtual bool onKeyState( Unicode::Char character, Key::State state );
    };

    RadianceClass Image : public Widget
    {
        float rotation = 0.0f;

        public:
            Image( Styler* styler, const Vector<float>& pos, cfx2::Node& description );
            virtual ~Image();

            float getRotation() const;
            void setRotation( float angle );
    };

    RadianceClass Label : public Widget
    {
        public:
            Label( Styler* styler, const Vector<float>& pos, const String& text, uint16_t align );
            virtual ~Label();

            void setText( const String& text );
    };

    RadianceClass Panel : public Animatable, public Widget
    {
        public:
            enum Properties { x, y, sizeX, sizeY, alpha };

        protected:
            List<Widget*> widgets;

            EventListener* onClick;

            Panel( WidgetStyle* style, const Vector<float>& pos, const Vector<float>& size );

        public:
            Panel( Styler* styler, const Vector<float>& pos, const Vector<float>& size );
            virtual ~Panel();

            virtual void animateProperty( unsigned name, float value );

            /*void addWidget( Widget* widget );*/
            void add( Widget* widget );
            /*void disable();
            void enable();*/
            virtual Widget* findWidget( const char* name );
            virtual bool mouseDown( const Vector<float>& mouse );
            virtual bool mouseMove( const Vector<float>& mouse );
            virtual bool mouseUp( const Vector<float>& mouse );
            virtual bool onKeyState( Unicode::Char character, Key::State state );
            /*bool removeWidget( Widget* widget );
            void remove( Widget* widget ) { removeWidget( widget ); }*/
            virtual void render( const Vector<float>& offset );
            void setOnClick( EventListener* listener );
            virtual void update( double delta );
    };

    RadianceClass Window : public Panel
    {
        Vector<float> drag;
        bool dragging = false, selected = false, down = false;

        bool updateSelected( const Vector<float>& mouse );

        public:
            Window( Styler* styler, const Vector<float>& pos, const Vector<float>& size, const String& title );
            virtual ~Window();

            virtual bool mouseDown( const Vector<float>& mouse );
            virtual bool mouseMove( const Vector<float>& mouse );
            virtual bool mouseUp( const Vector<float>& mouse );
            virtual void render( const Vector<float>& offset );
            virtual void setTop( bool top );
            virtual void show();
    };

    RadianceClass MessageBox : public Window, EventListener
    {
        EventListener* onClose = nullptr;

        public:
            MessageBox( Styler* styler, const Vector<float>& pos, const Vector<float>& size, const String& title, const String& text );
            virtual ~MessageBox();

            virtual void onRadianceEvent( Widget* widget, const String& eventName, void* eventProperties );
            void setOnClose( EventListener* listener );
    };

    RadianceClass UI : public Panel
    {
        /*int tooltipX, tooltipY;
        li::String tooltip;

        StormGraph::Text tooltipText;*/

        li::List<Widget*> focusQueue;
        //, overlays, removalQueue;

        Styler* styler = nullptr;
        Object<Window> modal;
        //Var<Widget*> topLevelWindow;

        Widget* currentTop = nullptr;

        Widget* load( const char* fileName, EventListener* listener, cfx2::Node node );
        void setTop( Widget* widget );

        public:
            UI( Styler* styler, const Vector<float>& pos, const Vector<float>& size );
            virtual ~UI();

            void add( Widget* widget );
            ///*void destroyWidget( Widget* widget );
            void doModal( Window* window );
            void focus( Widget* widget );
            //void hideTooltip();*/
            void load( const char* fileName, EventListener* listener );
            virtual bool mouseDown( const Vector<float>& mouse );
            virtual bool mouseMove( const Vector<float>& mouse );
            virtual bool mouseUp( const Vector<float>& mouse );
            //void overlay( Widget* widget );
            bool remove( Widget* widget );
            void render();
            //void showTooltip( int x, int y, const char* text );
            void update( double delta );
    };
}
