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
    RadianceClass EpicStyler : public Styler
    {
        friend class EpicWidgetStyle;
        friend class EpicWindowStyle;

        protected:
            IGraphicsDriver* driver;
            Reference<IResourceManager> resMgr;

            Reference<IFont> font;
            Reference<IMaterial> buttonMaterial, inputMaterial, panelMaterials[5];
            Reference<IModel> buttonModel, buttonHighlight, inputModel, inputHighlight, inputSaturate;

            IFont* getFont();

            IMaterial* getButtonMaterial();
            void getButtonModels( Reference<IModel>& button, Reference<IModel>& highlight, Reference<IMaterial>& material );

            IMaterial* getInputMaterial();
            void getInputModels( Reference<IModel>& model, Reference<IModel>& highlight,Reference<IModel>& saturate, Reference<IMaterial>& material );

            void getPanelMaterials( IMaterial** materials );
            ITexture* getTexture( const String& name );

        public:
            EpicStyler( IGraphicsDriver* driver, IResourceManager* resMgr );
            virtual ~EpicStyler();

            virtual WidgetStyle* createWidgetStyle( WidgetStyle::Type type, const Vector<float>& size );
    };

    RadianceClass EpicWindowStyle : public Animatable, public WidgetStyle
    {
        public:
            enum Properties { blur };

        protected:
            IGraphicsDriver* driver;
            bool selected = false;

            Reference<IRenderBuffer> renderBuffer;
            Reference<IMaterial> material;
            Reference<IModel> model[2];
            Reference<IShaderProgram> shader;
            Reference<IFont> font;
            Text* windowTitle;
            int blurProgress, blurRadius;

            Vector<float> size;

            List<Transform> modelTransforms;
            float blurLevel;

        public:
            EpicWindowStyle( EpicStyler* styler, const Vector<float>& size );
            virtual ~EpicWindowStyle();

            void animateProperty( unsigned name, float value );
            virtual Vector<float> beginRender( const Vector<float>& pos );
            virtual void endRender();
            //virtual void* getInterface( const char* name );
            virtual void onUpdate( double delta );
            virtual void setProperty( unsigned name, int value );
            virtual void setProperty( unsigned name, float value );
            virtual void setProperty( unsigned name, const String& value );
            virtual void setProperty( unsigned name, const Vector<float>& value );
    };

    RadianceClass EpicWidgetStyle : public WidgetStyle
    {
        protected:
            EpicStyler* styler;
            IGraphicsDriver* driver;
            WidgetStyle::Type type;

            Reference<IMaterial> material;
            Reference<IModel> model, highlight, saturate, modalShadow;
            Reference<IFont> font;
            Text* text = nullptr;

            Vector<float> size;
            List<Transform> modelTransforms;

            unsigned align;
            bool hasModal = false, mouseOver = false, selected = false;
            float highlightAlpha = 0.0f, satAlpha = 0.0f, cursorAlpha = 0.0f, modalShadowAlpha = 0.0f;
            Vector2<float> textDimensions;
            Input::Type inputType;

        public:
            EpicWidgetStyle( EpicStyler* styler, WidgetStyle::Type type, const Vector<float>& size );
            virtual ~EpicWidgetStyle();

            virtual Vector<float> beginRender( const Vector<float>& pos );
            virtual void onMouseMove( const Vector<float>& mouse, const Vector<float>& pos );
            virtual void onUpdate( double delta );
            virtual void renderStage( RenderStage stage );
            virtual void setProperty( unsigned name, int value );
            virtual void setProperty( unsigned name, float value );
            virtual void setProperty( unsigned name, const String& value );
            virtual void setProperty( unsigned name, const Vector<float>& value );
    };
}
