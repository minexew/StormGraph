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

#include <Radiance/Styler.hpp>

namespace Radiance
{
    RadianceClass OnyxStyler : public Styler
    {
        protected:
            IGraphicsDriver* driver;

        public:
            OnyxStyler( IGraphicsDriver* driver );
            virtual ~OnyxStyler();

            virtual WidgetStyle* createWidgetStyle( WidgetStyle::Type type, const Vector<float>& size );
    };

    RadianceClass OnyxWidgetStyle : public Animatable, public WidgetStyle
    {
        public:
            enum Properties { alpha, hue };

        protected:
            IGraphicsDriver* driver;
            WidgetStyle::Type type;

            Reference<IModel> model;
            Reference<IMaterial> material;

            Vector<float> size;
            List<Transform> modelTransforms;

        public:
            OnyxWidgetStyle( IGraphicsDriver* driver, WidgetStyle::Type type, const Vector<float>& size );
            virtual ~OnyxWidgetStyle();

            void animateProperty( unsigned name, float value );
            virtual Vector<float> beginRender( const Vector<float>& pos );
            virtual void endRender();
            //virtual void* getInterface( const char* name );
            virtual void setProperty( unsigned name, int value );
            virtual void setProperty( unsigned name, float value );
            virtual void setProperty( unsigned name, const String& value );
            virtual void setProperty( unsigned name, const Vector<float>& value );
            void setTexture( ITexture* texture );
    };
}
