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
    struct TextBoxProperties;
    struct WindowProperties;

    class UIStyler
    {
        public:
            class TextBoxStyle
            {
                public:
                    virtual ~TextBoxStyle() {}

                    virtual void render( const Vector<float>& pos ) = 0;
                    virtual void setSize( const Vector<float>& size ) = 0;
                    virtual void setValue( const char* value ) = 0;
            }

            class WindowStyle
            {
                public:
                    virtual ~WindowStyle() {}

                    virtual void isResizable() = 0;
                    virtual void render( const Vector<float>& pos ) = 0;
                    virtual void setSize( const Vector<float>& size ) = 0;
                    virtual void setTitle( const char* title ) = 0;
            };

            virtual ~UIStyler() {}

            virtual TextBoxStyle* createTextBoxStyle( const TextBoxProperties& properties ) = 0;
            virtual WindowStyle* createWindowStyle( const WindowProperties& properties ) = 0;
    };
}
