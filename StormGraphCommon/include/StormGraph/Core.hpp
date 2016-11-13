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

#include <StormGraph/Common.hpp>

namespace StormGraph
{
    class IEngine;

    namespace Key
    {
        enum State { released, pressed };
    }

    class ICommandListener
    {
        public:
            virtual ~ICommandListener() {}

            virtual bool onCommand( const List<String>& tokens ) = 0;
    };

    class ICore
    {
        public:
            virtual ~ICore() {}

            virtual IEngine* createEngine( const char* app, int argc, char** argv ) = 0;
    };

    class IEventListener
    {
        public:
            virtual ~IEventListener() {}

            virtual bool isRunning() { return false; }

            virtual void onCloseButton() {}
            virtual void onFrameBegin() {}
            virtual void onFrameEnd() {}

            virtual void onKeyState( int16_t key, Key::State state, Unicode::Char character ) {}
            virtual void onMouseMoveTo( const Vector2<int>& mouse ) {}
            virtual void onRender() {}
            virtual void onUpdate( double delta ) {}
            virtual void onViewportResize( const Vector2<unsigned>& dimensions ) {}
    };

    class IEventSource
    {
        public:
            virtual ~IEventSource() {}

            virtual void processEvents( IEventListener* listener ) = 0;
    };

    class ILineOutput : public ReferencedClass
    {
        public:
            virtual void writeLine( const char* text ) = 0;
    };

    class IResource : public ReferencedClass
    {
        public:
            virtual ~IResource() {}

            virtual const char* getClassName() const = 0;
            virtual const char* getName() const = 0;
    };

#ifdef StormGraph_Static_Core
    ICore* createCore( const char* apiVersion );
#else
    typedef ICore* ( *CoreProvider )( const char* apiVersion );
#endif
}
