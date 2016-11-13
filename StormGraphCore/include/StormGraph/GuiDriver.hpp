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

#include <StormGraph/Core.hpp>

namespace StormGraph
{
    class IEngine;
    class IPopupMenu;

    li_enum_class( MouseButton ) { left, right };
    li_enum_class( Orientation ) { horizontal, vertical };

    class ISizableWidget
    {
        public:
            enum Align { left = 0, top = 0, centered = 1, right = 2, middle = 4, bottom = 8 };

            virtual void setAlign( unsigned align ) = 0;
            virtual void setExpand( bool expand ) = 0;
            virtual void setFreeFloat( bool freeFloat ) = 0;
    };

    class IWidget : public ReferencedClass
    {
        protected:
            virtual ~IWidget() {}

        public:
            li_ReferencedClass_override( IWidget )

            virtual const char* getName() = 0;
            virtual void onKeyState( int16_t key, Key::State state, Unicode::Char character ) {}
            virtual bool onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse ) = 0;
            virtual void onMouseMoveTo( const Vector2<int>& mouse ) {}
            virtual void render() = 0;
            virtual void setName( const char* name ) = 0;
            virtual void update( double delta ) {}
    };

    class IChildWidget : public IWidget
    {
        protected:
            virtual ~IChildWidget() {}

        public:
            virtual Vector<float> getMinSize() = 0;
            virtual void setBounds( const Vector<float>& pos, const Vector<float>& size ) = 0;
    };

    class IGuiEventListener
    {
        public:
            struct CommandEvent
            {
                IWidget* widget;

                Vector2<int> mousePos;
            };

            struct MenuCancelEvent
            {
                IPopupMenu* menu;
            };

            struct MenuCommandEvent
            {
                IPopupMenu* menu;
                unsigned itemIndex;
            };

            struct MenuToggleEvent
            {
                IPopupMenu* menu;
                unsigned itemIndex;
                bool value;
            };

            virtual void onGuiCommand( const CommandEvent& event ) {}
            virtual void onGuiMenuCancel( const MenuCancelEvent& event ) {}
            virtual void onGuiMenuCommand( const MenuCommandEvent& event ) {}
            virtual void onGuiMenuToggle( const MenuToggleEvent& event ) {}
    };

    class IBoxSizer : public IChildWidget
    {
        protected:
            virtual ~IBoxSizer() {}

        public:
            virtual void add( IChildWidget* widget ) = 0;
    };

    class IButton : public IChildWidget, public ISizableWidget
    {
        protected:
            virtual ~IButton() {}

        public:
            virtual void setEventListener( IGuiEventListener* listener ) = 0;
            virtual void setName( const char* name ) = 0;
    };

    class IPanel : public IChildWidget, public ISizableWidget
    {
        protected:
            virtual ~IPanel() {}

        public:
            virtual void add( IChildWidget* widget ) = 0;
            virtual void setPadding( const Vector<>& padding ) = 0;
            virtual void setScrollable( bool scrollable ) = 0;
    };

    /**
     *  @brief A modal popup menu.
     */
    class IPopupMenu : public IWidget
    {
        protected:
            virtual ~IPopupMenu() {}

        public:
            /**
             *  @brief Add a text command item.
             *
             *  @param text label of the command
             *
             *  @return new item index
             */
            virtual unsigned addCommand( const char* text ) = 0;

            /**
             *  @brief Add a spacer item.
             *
             *  @return new item index
             */
            virtual unsigned addSpacer() = 0;

            /**
             *  @brief Add a toggle item.
             *
             *  @param text label for the toggle
             *  @param initial value (true for checked)
             *
             *  @return new item index
             *  @see getToggleValue
             */
            virtual unsigned addToggle( const char* text, bool value ) = 0;

            /**
             *  @brief Get the current value of a toggle item.
             *
             *  @param id item index previously returned by addToggle
             *
             *  @return the toggle value (true for checked)
             *  @see addToggle
             */
            virtual bool getToggleValue( unsigned id ) = 0;

            /**
             *  @brief Set the event listener to receive events from this menu.
             *
             *  @param listener the listener to connect
             */
            virtual void setEventListener( IGuiEventListener* listener ) = 0;

            virtual void setMinSize( const Vector<float>& minSize ) = 0;

            /**
             *  @brief Show the menu at the specified position.
             *
             *  The menu will gain focus and become the top modal window.
             *
             *  @param pos the position in GUI space
             */
            virtual void show( const Vector<>& pos ) = 0;
    };

    class IProgressBar : public IChildWidget, public ISizableWidget
    {
        protected:
            virtual ~IProgressBar() {}

        public:
            virtual void setMinSize( const Vector<float>& minSize ) = 0;
            virtual void setPadding( const Vector<>& padding ) = 0;
            virtual void setProgress( float value ) = 0;
    };

    class IScrollBar : public IChildWidget, public ISizableWidget
    {
        protected:
            virtual ~IScrollBar() {}

        public:
            //virtual void setPos( float pos ) = 0;
            virtual void setRange( float visible, float content ) = 0;
    };

    class IStaticText : public IChildWidget, public ISizableWidget
    {
        protected:
            virtual ~IStaticText() {}

        public:
            virtual void setEventListener( IGuiEventListener* listener ) = 0;
            virtual void setFont( IFont* font ) = 0;
            virtual void setText( const char* text ) = 0;
    };

    class ITableLayout : public IChildWidget
    {
        protected:
            virtual ~ITableLayout() {}

        public:
            virtual void add( IChildWidget* widget ) = 0;
            virtual void setColumnGrowable( size_t column, bool growable ) = 0;
            virtual void setRowGrowable( size_t row, bool growable ) = 0;
    };

    class ITextBox : public IChildWidget, public ISizableWidget
    {
        protected:
            virtual ~ITextBox() {}

        public:
            virtual void clear() = 0;
            virtual const char* getValue() = 0;
            virtual void setMinSize( const Vector<float>& minSize ) = 0;
            virtual void setValue( const char* value ) = 0;
    };

    class IWindow : public IPanel
    {
        protected:
            virtual ~IWindow() {}

        public:
            virtual const char* getTitle() = 0;
            virtual void setTitle( const char* title ) = 0;
            virtual void showModal() = 0;
    };

    class IGui : public IEventListener
    {
        public:
            virtual ~IGui() {}

            virtual IBoxSizer* createBoxSizer( Orientation orientation ) = 0;

            virtual IButton* createButton( const char* text ) = 0;
            virtual IButton* createButton( const Vector<float>& pos, const Vector<float>& size, const char* text ) = 0;
            virtual IPanel* createPanel( const Vector<float>& pos, const Vector<float>& size ) = 0;

            /**
             *  Create an empty popup menu.
             */
            virtual IPopupMenu* createPopupMenu() = 0;

            virtual IProgressBar* createProgressBar() = 0;
            virtual ITableLayout* createTableLayout( size_t numColumns ) = 0;
            virtual ITextBox* createTextBox( const Vector<float>& pos, const Vector<float>& size ) = 0;
            virtual IStaticText* createStaticText( const Vector<float>& pos, const Vector<float>& size, const char* text ) = 0;
            virtual IWindow* createWindow( const Vector<float>& pos, const Vector<float>& size, const char* title ) = 0;

            virtual void add( IWidget* widget ) = 0;
            virtual bool onMouseButton( MouseButton button, bool pressed, const Vector2<int>& mouse ) = 0;
            virtual void remove( IWidget* widget ) = 0;
    };

    class IGuiDriver
    {
        public:
            virtual ~IGuiDriver() {}

            virtual IGui* createGui( const Vector<float>& pos, const Vector<float>& size ) = 0;
            //virtual UI* createUI( UIStyler* styler ) = 0;
    };

    typedef IGuiDriver* ( *GuiDriverProvider )( IEngine* engine, const char* driverName );
}
