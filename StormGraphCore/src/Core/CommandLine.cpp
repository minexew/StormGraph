
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

#include <StormGraph/CommandLine.hpp>
#include <StormGraph/Engine.hpp>
#include <StormGraph/GraphicsDriver.hpp>
#include <StormGraph/GuiDriver.hpp>

namespace StormGraph
{
    class CommandLine : public ICommandLine
    {
        protected:
            IEngine* engine;

            int16_t down, enter, toggle, up;

            bool active;
            Reference<ITextBox> input;

            List<String> history;
            intptr_t historyIndex;

        public:
            CommandLine( IEngine* engine, IGui* gui );
            virtual ~CommandLine();

            virtual void onKeyState( int16_t key, Key::State state, Unicode::Char character ) override;
            virtual void onRender() override;
    };

    CommandLine::CommandLine( IEngine* engine, IGui* gui )
            : engine( engine ), active( false ), historyIndex( -1 )
    {
        down = engine->getGraphicsDriver()->getKey( "Down" );
        enter = engine->getGraphicsDriver()->getKey( "Enter" );
        toggle = engine->getGraphicsDriver()->getKey( "`" );
        up = engine->getGraphicsDriver()->getKey( "Up" );

        input = gui->createTextBox( Vector<>( 8.0f, 8.0f ), Vector<>( engine->getGraphicsDriver()->getViewportSize().x - 16.0f, -1.0f ) );

        engine->registerEventListener( this );
    }

    CommandLine::~CommandLine()
    {
        engine->unregisterEventListener( this );
    }

    void CommandLine::onKeyState( int16_t key, Key::State state, Unicode::Char character )
    {
        if ( key == toggle && state == Key::pressed )
        {
            active = !active;
            return;
        }

        if ( !active )
            return;

        if ( key == down && state == Key::pressed )
        {
            if ( historyIndex >= 0 && ( size_t ) historyIndex < history.getLength() )
            {
                historyIndex++;
                input->setValue( history[historyIndex] );
            }
            else
                input->clear();
        }
        else if ( key == enter && state == Key::pressed )
        {
            String command( input->getValue() );

            if ( command.isEmpty() )
                return;

            if ( history.isEmpty() || history.getFromEnd() != command )
                history.add( command );

            historyIndex = -1;

            engine->command( command );

            input->clear();
        }
        else if ( key == up && state == Key::pressed )
        {
            if ( historyIndex < 0 )
                historyIndex = history.getLength();

            if ( historyIndex > 0 )
            {
                historyIndex--;
                input->setValue( history[historyIndex] );
            }
        }
        else
            input->onKeyState( key, state, character );
    }

    void CommandLine::onRender()
    {
        if ( !active )
            return;

        input->render();
    }

    ICommandLine* createCommandLine( IEngine* engine, IGui* gui )
    {
        return new CommandLine( engine, gui );
    }
}
