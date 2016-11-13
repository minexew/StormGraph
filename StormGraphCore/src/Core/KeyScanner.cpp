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
#include <StormGraph/GraphicsDriver.hpp>
#include <StormGraph/KeyScanner.hpp>

namespace StormGraph
{
    class KeyScanner : public IEventListener, public IKeyScanner
    {
        IEngine* engine;

        List<int16_t> registeredKeys;
        Array<Key::State> states;

        public:
            KeyScanner( IEngine* engine );
            virtual ~KeyScanner();

            virtual size_t registerKey( int16_t key ) override;
            virtual size_t registerKey( const char* name ) override;

            virtual Key::State getKeyState( size_t index ) override;

            virtual void onKeyState( int16_t key, Key::State state, Unicode::Char character ) override;
    };

    KeyScanner::KeyScanner( IEngine* engine )
            : engine( engine )
    {
        engine->registerEventListener( this );
    }

    KeyScanner::~KeyScanner()
    {
        engine->unregisterEventListener( this );
    }

    size_t KeyScanner::registerKey( int16_t key )
    {
        return registeredKeys.add( key );
    }

    size_t KeyScanner::registerKey( const char* name )
    {
        return registeredKeys.add( engine->getGraphicsDriver()->getKey( name ) );
    }

    Key::State KeyScanner::getKeyState( size_t index )
    {
        return states[index];
    }

    void KeyScanner::onKeyState( int16_t key, Key::State state, Unicode::Char character )
    {
        iterate ( registeredKeys )
            if ( registeredKeys.current() == key )
            {
                states[registeredKeys.iter()] = state;
                break;
            }
    }

    IKeyScanner* createKeyScanner( IEngine* engine )
    {
        return new KeyScanner( engine );
    }
}
