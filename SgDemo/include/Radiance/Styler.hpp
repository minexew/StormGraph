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

#include <Radiance/Radiance.hpp>

namespace Radiance
{
    RadianceClass WidgetStyle : public Resource
    {
        public:
            enum Type
            {
                button, image, input, label, panel, ui, window
            };

            enum Properties
            {
                align, hasModal, inputType, origin, rotation, scale, selected, text, textureName, title
            };

            enum RenderStage
            {
                beforeModal, renderModal
            };

        public:
            WidgetStyle();
            virtual ~WidgetStyle();

            virtual Vector<float> beginRender( const Vector<float>& pos ) = 0;
            virtual void endRender();
            virtual void onMouseMove( const Vector<float>& mouse, const Vector<float>& pos );
            virtual void onUpdate( double delta );
            virtual void renderStage( RenderStage stage );
            virtual void setProperty( unsigned name, int value ) = 0;
            virtual void setProperty( unsigned name, float value ) = 0;
            virtual void setProperty( unsigned name, const String& value ) = 0;
            virtual void setProperty( unsigned name, const Vector<float>& value ) = 0;
    };

    RadianceClass Styler
    {
        public:
            virtual ~Styler();

            virtual WidgetStyle* createWidgetStyle( WidgetStyle::Type type, const Vector<float>& size ) = 0;
    };
}
