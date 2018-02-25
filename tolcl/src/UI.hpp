#pragma once

#include "GameClient.hpp"

namespace GameUI
{
    class Widget;

    static const StormGraph::Colour defaultPanelColour( 0.0f, 0.0f, 0.0f, 0.75f );

    class EventListener
    {
        public:
            virtual ~EventListener();

            virtual void uiEvent( Widget* widget, const li::String& event ) = 0;
    };

    class InputFilter
    {
        public:
            virtual ~InputFilter();

            virtual bool isValid( Widget* widget, const li::String& text ) = 0;
    };

    class Widget
    {
        protected:
            int x, y;
            li::String name;
            Widget* modalParent;

        public:
            Widget( int x, int y );
            virtual ~Widget();

            virtual Widget* findWidget( const char* name );
            Widget* getModalParent() { return modalParent; }
            const li::String& getName() const { return name; }
            virtual bool keyDown( li::Utf8Char key );
            virtual bool mouseDown( int x, int y );
            virtual bool mouseMove( int x, int y );
            virtual bool mouseUp( int x, int y );
            virtual void move( int x, int y );
            virtual void render();
            void setModalParent( Widget* parent ) { modalParent = parent; }
            void setName( const char* name ) { this->name = name; }
    };

    class StaticImage : public Widget
    {
        StormGraph::Texture* texture;
        int w, h;

        public:
            StaticImage( int x, int y, StormGraph::Texture* texture = 0, int w = 0, int h = 0 );
            virtual ~StaticImage();

            virtual void render();
            StaticImage* setTexture( StormGraph::Texture* texture );
    };

    class Label : public Widget
    {
        li::String fontName, text;
        StormGraph::Colour colour;
        StormGraph::Font* font;

        public:
            Label( int x, int y, const char* text, const char* fontName = "ui", StormGraph::Colour colour = StormGraph::Colour( 1.0f, 1.0f, 1.0f ) );
            virtual ~Label();

            virtual void render();
            void setText( const char* text );
    };

    class Button : public Widget
    {
        int w, h;
        //li::String text;
        StormGraph::Texture* buttonTexture, * labelTexture;

        //StormGraph::Colour colour;
        //StormGraph::Font* font;

        EventListener* onPush;

        public:
            Button( int x, int y, int w, int h, const char* text );
            virtual ~Button();

            virtual bool mouseDown( int x, int y );
            virtual void render();
            void setOnPush( EventListener* listener );
    };

    class GraphicalButton : public Widget
    {
        int w, h;
        StormGraph::Texture* texture;

        EventListener* onPush;

        public:
            GraphicalButton( int x, int y, int w, int h, StormGraph::Texture* texture );
            virtual ~GraphicalButton();

            virtual bool mouseDown( int x, int y );
            virtual void render();
            void setOnPush( EventListener* listener );
    };

    class Input : public Widget
    {
        protected:
            int w, h;

            StormGraph::Font* font;
            InputFilter* filter;

            bool active;
            li::String text;

        public:
            Input( int x, int y, int w, int h );
            virtual ~Input();

            const li::String& getText() const { return text; }
            virtual bool keyDown( li::Utf8Char key );
            virtual bool mouseDown( int x, int y );
            virtual bool mouseUp( int x, int y );
            virtual void render();
            void setFilter( InputFilter* filt );
            void setText( const li::String& text );
    };

    class List : public Widget
    {
        li::List<li::String> items;
        int w, h;

        StormGraph::Font* font;

        EventListener* onSelect;

        public:
            List( int x, int y, int w, int h );
            virtual ~List();

            unsigned addItem( const li::String& item );
            virtual bool mouseDown( int x, int y );
            virtual void render();
            void setOnSelect( EventListener* listener );
    };

    class Panel : public Widget
    {
        protected:
            li::List<Widget*> widgets;
            Widget* modal;

            int w, h;
            bool enabled, visible;
            StormGraph::Colour colour;

            EventListener* onClick;

        public:
            Panel( int x, int y, int w, int h, bool visible = false, StormGraph::Colour colour = defaultPanelColour );
            virtual ~Panel();

            void addWidget( Widget* widget );
            void add( Widget* widget ) { addWidget( widget ); }
            void disable();
            void doModal( Widget* widget );
            void enable();
            virtual Widget* findWidget( const char* name );
            virtual bool keyDown( li::Utf8Char key );
            virtual bool mouseDown( int x, int y );
            virtual bool mouseMove( int x, int y );
            virtual bool mouseUp( int x, int y );
            virtual void move( int x, int y );
            bool removeWidget( Widget* widget );
            void remove( Widget* widget ) { removeWidget( widget ); }
            virtual void render();
            void setOnClick( EventListener* listener );
    };

    class UI : public Panel
    {
        int tooltipX, tooltipY;
        li::String tooltip;

        StormGraph::Text tooltipText;

        li::List<Widget*> focusQueue, overlays, removalQueue;

        public:
            UI( int w, int h );
            virtual ~UI();

            void destroyWidget( Widget* widget );
            void focus( Widget* widget );
            void hideTooltip();
            void overlay( Widget* widget );
            virtual void render();
            void showTooltip( int x, int y, const char* text );
            void update();
    };

    class Window : public Panel
    {
        UI* ui;

        int dragX, dragY;
        bool dragging;

        StormGraph::Font* font;
        StormGraph::Text windowTitle;

        public:
            Window( UI* ui, const char* title, int x, int y, int w, int h, bool visible = true, StormGraph::Colour colour = defaultPanelColour );
            virtual ~Window();

            void hide();
            virtual bool mouseDown( int x, int y );
            virtual bool mouseMove( int x, int y );
            virtual bool mouseUp( int x, int y );
            virtual void render();
            void show();
    };

    class MessageBox : public Window, EventListener
    {
        EventListener* onClose;

        public:
            MessageBox( const char* text, const char* title, int x, int y, int w, int h );
            virtual ~MessageBox();

            void setOnClose( EventListener* listener );
            void uiEvent( Widget* widget, const li::String& event );
    };
};
