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
#include <StormGraph/GuiDriver.hpp>
#include <StormGraph/ResourceManager.hpp>

namespace GuiDriver
{
    using namespace StormGraph;

    class Gui;
    class GuiDriver;
    class Panel;
    class ScrollBar;

    class Widget
    {
        protected:
            String name;
            Vector2<float> pos, size, minSize;

            Vector2<float> areaPos, areaSize;
            unsigned align;
            bool expand, freeFloat;

            Vector2<float> realPos, realSize;

            //bool fitsInArea( const Vector<float>& size ) const;
            void realign();

        public:
            Widget( const Vector2<float>& pos, const Vector2<float>& size, const Vector2<float>& minSize );
            virtual ~Widget();

            virtual Vector<float> getMinSize();
            //const char* getName() { return name; }
            void setAlign( unsigned align );
            void setBounds( const Vector<float>& pos, const Vector<float>& size );
            void setExpand( bool expand );
            void setFreeFloat( bool freeFloat );
            void setName( const char* name );
    };

    class Container
    {
        protected:
            ReferenceList<IWidget> widgets;

        public:
            Container();
            ~Container();

            void add( IWidget* widget );
            virtual void onKeyState( int16_t key, Key::State state, Unicode::Char character );
            virtual bool onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse );
            virtual void onMouseMoveTo( const Vector2<int>& mouse );
            virtual void render();
            virtual void update( double delta );
    };

    class BoxSizer : public IBoxSizer, public Container
    {
        protected:
            String name;
            Orientation orientation;
            float spacing;

            Vector2<float> areaPos, areaSize;

            void layout();

        public:
            BoxSizer( Gui* gui, Orientation orientation );
            ~BoxSizer();

            virtual void add( IChildWidget* widget );
            virtual Vector<float> getMinSize();
            virtual const char* getName() { return name; }
            virtual void onKeyState( int16_t key, Key::State state, Unicode::Char character );
            virtual bool onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse ) override;
            virtual void onMouseMoveTo( const Vector2<int>& mouse );
            virtual void render() override;
            virtual void setBounds( const Vector<float>& pos, const Vector<float>& size ) override;
            virtual void setName( const char* name ) override { this->name = name; }
            virtual void update( double delta ) override;
    };

    class Button : public IButton, public Widget
    {
        protected:
            IGuiEventListener* eventListener;
            Vector2<float> padding;
            String text;

            IGraphicsDriver* graphicsDriver;
            Reference<IFont> font;

            Text* layout;

        public:
            Button( Gui* gui, const Vector<float>& pos, const Vector<float>& size, const char* text );
            virtual ~Button();

            virtual Vector<float> getMinSize();
            virtual const char* getName() { return name; }
            virtual void onKeyState( int16_t key, Key::State state, Unicode::Char character );
            virtual bool onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse ) override;
            virtual void onMouseMoveTo( const Vector2<int>& mouse );
            virtual void render();
            virtual void setAlign( unsigned align );
            virtual void setBounds( const Vector<float>& pos, const Vector<float>& size );
            virtual void setEventListener( IGuiEventListener* listener ) { eventListener = listener; }
            virtual void setExpand( bool expand ) override { Widget::setExpand( expand ); }
            virtual void setFreeFloat( bool freeFloat ) override { Widget::setFreeFloat( freeFloat ); }
            virtual void setName( const char* name ) override { Widget::setName( name ); }
            virtual void setText( const char* text );
            virtual void update( double delta );
    };

    class Panel : public IPanel, public Widget, public Container
    {
        protected:
            Vector2<float> padding;
            bool scrollable;

            Object<ScrollBar> horScrollBar, vertScrollBar;

            Gui* gui;
            IGraphicsDriver* graphicsDriver;

            Vector<float> getMinSize( bool ofContentArea );
            void layout();

        public:
            Panel( Gui* gui, const Vector<float>& pos, const Vector<float>& size );
            virtual ~Panel();

            virtual void add( IChildWidget* widget ) override;
            virtual Vector<float> getMinSize() override { return getMinSize( false ); }
            virtual const char* getName() { return name; }
            virtual void move( const Vector<float>& vec );
            virtual void onKeyState( int16_t key, Key::State state, Unicode::Char character );
            virtual bool onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse ) override;
            virtual void onMouseMoveTo( const Vector2<int>& mouse ) override { Container::onMouseMoveTo( mouse ); }
            virtual void setAlign( unsigned align );
            virtual void setBounds( const Vector<float>& pos, const Vector<float>& size );
            virtual void setExpand( bool expand ) override { Widget::setExpand( expand ); }
            virtual void setFreeFloat( bool freeFloat ) override { Widget::setFreeFloat( freeFloat ); }
            virtual void setName( const char* name ) override { Widget::setName( name ); }
            virtual void setPadding( const Vector<>& padding ) override;
            virtual void setScrollable( bool scrollable ) override;
            virtual void render();
            virtual void update( double delta );
    };

    class PopupMenu : public IPopupMenu
    {
        public:
            struct Item
            {
                enum { boolean, command, spacer } type;

                //String text;
                bool checked;

                Text* labelLayout;
                float labelX, height;
            };

        protected:
            String name;

            Gui* gui;
            IGuiEventListener* eventListener;

            IGraphicsDriver* graphicsDriver;
            Reference<IFont> font;

            List<Item> items;
            int selectedItem;

            Vector2<float> minSize, padding, realPos, realSize;

            void realign();

        public:
            PopupMenu( Gui* gui );
            virtual ~PopupMenu();

            virtual unsigned addCommand( const char* text );
            virtual unsigned addSpacer() override;
            virtual unsigned addToggle( const char* text, bool value );
            virtual const char* getName() { return name; }
            virtual bool getToggleValue( unsigned id );
            virtual void onKeyState( int16_t key, Key::State state, Unicode::Char character );
            virtual bool onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse ) override;
            virtual void onMouseMoveTo( const Vector2<int>& mouse );
            virtual void render();
            virtual void update( double delta );
            virtual void setEventListener( IGuiEventListener* listener );
            virtual void setMinSize( const Vector<float>& minSize );
            virtual void setName( const char* name ) override { this->name = name; }
            virtual void show( const Vector<>& pos );
    };

    class ProgressBar : public IProgressBar, public Widget
    {
        protected:
            float progress;
            Vector2<> padding;

            IGraphicsDriver* graphicsDriver;

        public:
            ProgressBar( Gui* gui, const Vector<float>& pos, const Vector<float>& size );

            virtual Vector<> getMinSize() override;
            virtual const char* getName() override { return name; }
            virtual bool onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse ) override;
            virtual void render() override;
            virtual void setAlign( unsigned align ) override { Widget::setAlign( align ); }
            virtual void setBounds( const Vector<float>& pos, const Vector<float>& size ) override { Widget::setBounds( pos, size ); }
            virtual void setExpand( bool expand ) override { Widget::setExpand( expand ); }
            virtual void setFreeFloat( bool freeFloat ) override { Widget::setFreeFloat( freeFloat ); }
            virtual void setMinSize( const Vector<float>& minSize ) override;
            virtual void setName( const char* name ) override { Widget::setName( name ); }
            virtual void setPadding( const Vector<>& padding ) override;
            virtual void setProgress( float progress ) override;
    };

    class ScrollBar : public IScrollBar, public Widget
    {
        protected:
            Orientation orientation;
            float visible, content, factor, pos;

            IGraphicsDriver* graphicsDriver;

        public:
            /**
             *  setRange must be called to finish initialization (may throw)
             */
            ScrollBar( Gui* gui, const Vector<float>& pos, const Vector<float>& size, Orientation orientation );
            virtual ~ScrollBar();

            virtual Vector<float> getMinSize() override;
            virtual const char* getName() override { return name; }
            virtual bool onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse ) override;
            virtual void render() override;
            virtual void setAlign( unsigned align ) override { Widget::setAlign( align ); }
            virtual void setBounds( const Vector<float>& pos, const Vector<float>& size ) override { Widget::setBounds( pos, size ); }
            //virtual void setEventListener( IGuiEventListener* listener ) override { eventListener = listener; }
            virtual void setExpand( bool expand ) override { Widget::setExpand( expand ); }
            virtual void setFreeFloat( bool freeFloat ) override { Widget::setFreeFloat( freeFloat ); }
            virtual void setName( const char* name ) override { Widget::setName( name ); }
            virtual void setRange( float visible, float content ) override;
    };

    class StaticText : public IStaticText, public Widget
    {
        protected:
            IGuiEventListener* eventListener;
            String text;

            IGraphicsDriver* graphicsDriver;
            Reference<IFont> font;

            Text* layout;

        public:
            StaticText( Gui* gui, const Vector<float>& pos, const Vector<float>& size, const char* text );
            virtual ~StaticText();

            virtual Vector<float> getMinSize() override;
            virtual const char* getName() override { return name; }
            virtual bool onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse ) override;
            virtual void render() override;
            virtual void setAlign( unsigned align ) override { Widget::setAlign( align ); }
            virtual void setBounds( const Vector<float>& pos, const Vector<float>& size ) override { Widget::setBounds( pos, size ); }
            virtual void setEventListener( IGuiEventListener* listener ) override { eventListener = listener; }
            virtual void setExpand( bool expand ) override { Widget::setExpand( expand ); }
            virtual void setFreeFloat( bool freeFloat ) override { Widget::setFreeFloat( freeFloat ); }
            virtual void setFont( IFont* font ) override { this->font = font; setText( text ); }
            virtual void setName( const char* name ) override { Widget::setName( name ); }
            virtual void setText( const char* text ) override;
    };

    class TableLayout : public ITableLayout, public Container
    {
        protected:
            String name;
            size_t numColumns;
            Vector2<float> spacing;

            Array<bool> columnsGrowable, rowsGrowable;

            Vector2<float> areaPos, areaSize;

            void layout();

        public:
            TableLayout( Gui* gui, size_t numColumns );
            ~TableLayout();

            virtual void add( IChildWidget* widget );
            virtual Vector<float> getMinSize();
            virtual const char* getName() { return name; }
            virtual void onKeyState( int16_t key, Key::State state, Unicode::Char character );
            virtual bool onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse ) override;
            virtual void onMouseMoveTo( const Vector2<int>& mouse );
            virtual void render();
            virtual void setBounds( const Vector<float>& pos, const Vector<float>& size );
            virtual void setColumnGrowable( size_t column, bool growable );
            virtual void setName( const char* name ) override { this->name = name; }
            virtual void setRowGrowable( size_t row, bool growable );
            virtual void update( double delta );
    };

    class TextBox : public ITextBox, public Widget
    {
        protected:
            String value;

            IGraphicsDriver* graphicsDriver;
            Reference<IFont> font;

            int16_t backspace;

            Text* layout;
            Vector2<float> layoutSize;

        public:
            TextBox( Gui* gui, const Vector<float>& pos, const Vector<float>& size );
            virtual ~TextBox();

            virtual void clear();
            virtual Vector<float> getMinSize() { return Widget::getMinSize(); }
            virtual const char* getName() { return name; }
            virtual const char* getValue() { return value; }
            virtual void onKeyState( int16_t key, Key::State state, Unicode::Char character );
            virtual bool onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse ) override;
            virtual void render();
            virtual void setAlign( unsigned align ) override { Widget::setAlign( align ); }
            virtual void setBounds( const Vector<float>& pos, const Vector<float>& size ) override { Widget::setBounds( pos, size ); }
            virtual void setExpand( bool expand ) override { Widget::setExpand( expand ); }
            virtual void setFreeFloat( bool freeFloat ) override { Widget::setFreeFloat( freeFloat ); }
            virtual void setMinSize( const Vector<float>& minSize );
            virtual void setName( const char* name ) override { Widget::setName( name ); }
            virtual void setValue( const char* value );
    };

    class Window : public IWindow, public Panel
    {
        protected:
            String title;
            bool closeButton, resizable;

            bool drag, resize;
            Vector2<int> dragFrom;

            Gui* gui;
            Reference<IFont> font;

            Text* titleLayout;

        public:
            Window( Gui* gui, const Vector<float>& pos, const Vector<float>& size, const char* title );
            virtual ~Window();

            virtual void add( IChildWidget* widget );
            virtual Vector<float> getMinSize();
            virtual const char* getName() { return name; }
            virtual const char* getTitle();
            virtual void move( const Vector<float>& vec );
            virtual void onKeyState( int16_t key, Key::State state, Unicode::Char character );
            virtual bool onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse ) override;
            virtual void onMouseMoveTo( const Vector2<int>& mouse );
            virtual void setAlign( unsigned align );
            virtual void setBounds( const Vector<float>& pos, const Vector<float>& size );
            virtual void setExpand( bool expand );
            virtual void setFreeFloat( bool freeFloat ) { Panel::setFreeFloat( freeFloat ); }
            virtual void setName( const char* name ) override { Widget::setName( name ); }
            virtual void setPadding( const Vector<>& padding ) override { Panel::setPadding( padding ); }
            virtual void setScrollable( bool scrollable ) override { Panel::setScrollable( scrollable ); }
            virtual void setTitle( const char* title );
            virtual void showModal();
            virtual void render();
            virtual void update( double delta );
    };

    class Gui : public IGui, public Container
    {
        protected:
            GuiDriver* driver;

            Vector2<float> pos, size;

            Vector2<int> mousePos;
            int16_t lmb;

            List<IWidget*> modalWindows;

            Reference<IFont> font;

        public:
            Gui( GuiDriver* driver, const Vector<float>& pos, const Vector<float>& size );
            virtual ~Gui();

            IGraphicsDriver* getGraphicsDriver();
            IFont* getTextFont();
            void pushModal( IWidget* window );

            // StormGraph::IGui
            virtual void add( IWidget* widget );
            virtual IBoxSizer* createBoxSizer( Orientation orientation );

            virtual IButton* createButton( const char* text );
            virtual IButton* createButton( const Vector<float>& pos, const Vector<float>& size, const char* text );

            virtual IPanel* createPanel( const Vector<float>& pos, const Vector<float>& size );
            virtual IPopupMenu* createPopupMenu();
            virtual IProgressBar* createProgressBar() override { return new ProgressBar( this, Vector2<>(), Vector2<>() ); }
            virtual ITableLayout* createTableLayout( size_t numColumns );
            virtual ITextBox* createTextBox( const Vector<float>& pos, const Vector<float>& size );
            virtual IStaticText* createStaticText( const Vector<float>& pos, const Vector<float>& size, const char* text );
            virtual IWindow* createWindow( const Vector<float>& pos, const Vector<float>& size, const char* title );
            virtual void onKeyState( int16_t key, Key::State state, Unicode::Char character );
            virtual bool onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse ) override;
            virtual void onMouseMoveTo( const Vector2<int>& mouse );
            virtual void onRender();
            virtual void onUpdate( double delta );
            virtual void remove( IWidget* widget ) override;
    };

    class GuiDriver : public IGuiDriver
    {
        friend class Gui;

        protected:
            IEngine* engine;

            Reference<IResourceManager> uiResMgr;

        public:
            GuiDriver( IEngine* engine );
            virtual ~GuiDriver();

            virtual IGui* createGui( const Vector<float>& pos, const Vector<float>& size );
    };
}
