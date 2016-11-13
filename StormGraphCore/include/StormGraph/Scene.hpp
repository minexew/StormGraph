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
    class IScene : public IEventListener, public ReferencedClass
    {
        public:
            li_ReferencedClass_override( IScene )

            virtual ~IScene() {}

            //virtual bool closeButtonAction();
            //virtual void keyStateChange( unsigned short key, bool pressed, Utf8Char character );
            //virtual void mouseButton( int x, int y, bool right, bool down );
            //virtual void mouseMove( int x, int y );
            //virtual void mouseWheel( bool down );

            /*virtual void onKeyState( int16_t key, Key::State state, Unicode::Char character ) {}
            virtual void onMouseMoveTo( const Vector2<int>& mouse ) {}

            virtual void render() {}
            virtual void update( double delta ) {}*/

            virtual void init() = 0;
            virtual void uninit() = 0;
    };
}
