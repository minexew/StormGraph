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

#include <Radiance/EpicStyler.hpp>

#include <StormGraph/ResourceManager.hpp>

#include "Models.hpp"

namespace Radiance
{
    EpicStyler::EpicStyler( GraphicsDriver* driver, ResourceManager* resMgr )
            : driver( driver ), resMgr( resMgr )
    {
    }

    EpicStyler::~EpicStyler()
    {
    }

    WidgetStyle* EpicStyler::createWidgetStyle( WidgetStyle::Type type, const Vector<float>& size )
    {
        if ( type == WidgetStyle::window )
            return new EpicWindowStyle( this, size );

        return new EpicWidgetStyle( this, type, size );
    }

    IFont* EpicStyler::getFont()
    {
        if ( !font )
            font = resMgr->getFont( "Radiance.EpicStyler.Assets/DefaultFont.ttf", 16, IFont::normal );

        return font->reference();
    }

    IMaterial* EpicStyler::getButtonMaterial()
    {
        if ( !buttonMaterial )
            buttonMaterial = driver->createSolidMaterial( "epic_button_mat", Colour::white(), resMgr->getTexture( "Radiance.EpicStyler.Assets/Button112x36.png" ) );

        return buttonMaterial->reference();
    }

    void EpicStyler::getButtonModels( Reference<IModel>& button, Reference<IModel>& highlight, Reference<IMaterial>& material )
    {
        if ( !buttonModel )
        {
            buttonModel = model_button( driver, getButtonMaterial(), 112.0f, 36.0f );
            buttonHighlight = model_button_highlight( driver, getButtonMaterial(), 112.0f, 36.0f );
        }

        button = buttonModel->reference();
        highlight = buttonHighlight->reference();
        material = getButtonMaterial();
    }

    IMaterial* EpicStyler::getInputMaterial()
    {
        if ( !inputMaterial )
            inputMaterial = driver->createSolidMaterial( "epic_input_mat", Colour::white(), resMgr->getTexture( "Radiance.EpicStyler.Assets/Input240x36.png" ) );

        return inputMaterial->reference();
    }

    void EpicStyler::getInputModels( Reference<IModel>& model, Reference<IModel>& highlight,Reference<IModel>& saturate, Reference<IMaterial>& material )
    {
        if ( !inputModel )
        {
            inputModel = model_input( driver, getInputMaterial(), 240.0f, 36.0f );
            inputHighlight = model_input_highlight( driver, getInputMaterial(), 240.0f, 36.0f );
            inputSaturate = model_input_saturate( driver, getInputMaterial(), 240.0f, 36.0f );
        }

        model = inputModel->reference();
        highlight = inputHighlight->reference();
        saturate = inputSaturate->reference();
        material = getInputMaterial();
    }

    void EpicStyler::getPanelMaterials( IMaterial** materials )
    {
        if ( !panelMaterials[0] )
        {
            Colour blend( 0.45f, 0.48f, 0.42f, 0.9f );

            panelMaterials[0] = driver->createSolidMaterial( "epic_panel_mat0", blend, resMgr->getTexture( "Radiance.EpicStyler.Assets/Panel.png" ) );
            panelMaterials[1] = driver->createSolidMaterial( "epic_panel_mat1", blend, resMgr->getTexture( "Radiance.EpicStyler.Assets/Panel_Corners.png" ) );
            panelMaterials[2] = driver->createSolidMaterial( "epic_panel_mat2", blend, resMgr->getTexture( "Radiance.EpicStyler.Assets/Panel_TB.png" ) );
            panelMaterials[3] = driver->createSolidMaterial( "epic_panel_mat3", blend, resMgr->getTexture( "Radiance.EpicStyler.Assets/Panel_LR.png" ) );
        }

        for ( unsigned i = 0; i < 4; i++ )
            materials[i] = panelMaterials[i];
    }

    ITexture* EpicStyler::getTexture( const String& name )
    {
        return resMgr->getTexture( name );
    }

    EpicWindowStyle::EpicWindowStyle( EpicStyler* styler, const Vector<float>& size )
            : driver( styler->driver ), windowTitle( 0 ), size( size ), blurLevel( 0.0f )
    {
        IMaterial* materials[4];
        styler->getPanelMaterials( materials );

        model[0] = model_window( driver, materials, size.x, size.y );

        modelTransforms.add( Transform( Transform::translate, Vector<float>() ) );

        Vector2<> renderDim = size.getXy() + Vector2<>( 48.0f, 48.0f );
        renderBuffer = driver->createRenderBuffer( renderDim.ceil().convert<unsigned>(), false );

        if ( renderBuffer )
        {
            shader = driver->createCustomShader( "Solid", "Radiance.EpicStyler.Assets/Blur" );

            if ( !shader )
                // we need both shaders and renderBuffers to do the job
                renderBuffer.release();
            else
            {
                blurProgress = shader->getParamId( "blurProgress" );
                blurRadius = shader->getParamId( "blurRadius" );

                material = driver->createCustomMaterial( "EpicStyler/shaderMat", shader->reference() );

                PlaneCreationInfo plane( renderDim, Vector<>(), Vector2<>(), Vector2<>( 1.0f, 1.0f ), false, true, material->reference() );
                model[1] = driver->createPlane( "epic_panel_render_plane", &plane );
            }
        }

        font = styler->getFont();
    }

    EpicWindowStyle::~EpicWindowStyle()
    {
        if ( windowTitle )
            font->releaseText( windowTitle );
    }

    void EpicWindowStyle::animateProperty( unsigned name, float value )
    {
        switch ( name )
        {
            case blur:
                blurLevel = value;
                break;
        }
    }

    Vector<float> EpicWindowStyle::beginRender( const Vector<float>& pos )
    {
        driver->disableDepthTesting();

        if ( windowTitle )
            font->renderText( pos.x, pos.y - 12.0f, windowTitle );

        if ( renderBuffer )
        {
            driver->pushProjection();
            driver->pushRenderBuffer( renderBuffer );
            driver->set2dMode( -10.0f, 10.0f );
            driver->clear();

            modelTransforms[0].vector = Vector<float>( 24.0f, 24.0f );
            model[0]->render( modelTransforms );
            modelTransforms[0].vector = pos - Vector<float>( 24.0f, 24.0f );

            return -modelTransforms[0].vector;
        }
        else
        {
            modelTransforms[0].vector = pos;
            model[0]->render( modelTransforms );

            return Vector<float>();
        }
    }

    void EpicWindowStyle::endRender()
    {
        if ( renderBuffer )
        {
            driver->popRenderBuffer();
            driver->popProjection();

            shader->select();
            shader->setFloatParam( blurProgress, blurLevel );
            shader->setVector2Param( blurRadius, blurLevel / size.x, blurLevel / size.y );
            shader->setTexture( renderBuffer->getTexture() );

            model[1]->render( modelTransforms );
        }
    }

    void* EpicWindowStyle::getInterface( const char* name )
    {
        if ( strcmp( name, "Radiance.EpicWindowStyle" ) == 0 )
            return this;
        else
            return Resource::getInterface( name );
    }

    void EpicWindowStyle::onUpdate( double delta )
    {
        if ( renderBuffer )
        {
            if ( selected && blurLevel > 0.0f )
                blurLevel = maximum<float>( blurLevel - delta / 0.25f, 0.0f );
            else if ( !selected && blurLevel < 1.0f )
                blurLevel = minimum<float>( blurLevel + delta / 0.25f, 1.0f );
        }
    }

    void EpicWindowStyle::setProperty( unsigned name, int value )
    {
        switch ( name )
        {
            case WidgetStyle::selected: selected = value; break;
        }
    }

    void EpicWindowStyle::setProperty( unsigned name, float value )
    {
    }

    void EpicWindowStyle::setProperty( unsigned name, const String& value )
    {
        switch ( name )
        {
            case WidgetStyle::title:
                if ( windowTitle )
                    font->releaseText( windowTitle );

                windowTitle = font->layoutText( value, Colour::white(), IFont::left | IFont::bottom );
                break;
        }
    }

    void EpicWindowStyle::setProperty( unsigned name, const Vector<float>& value )
    {
    }

    EpicWidgetStyle::EpicWidgetStyle( EpicStyler* styler, WidgetStyle::Type type, const Vector<float>& size )
            : styler( styler ), driver( styler->driver ), type( type ), size( size )
    {
        switch ( type )
        {
            case WidgetStyle::button:
                styler->getButtonModels( model, highlight, material );

                font = styler->getFont();
                align = IFont::centered | IFont::middle;

                modelTransforms.add( Transform( Transform::translate, Vector<float>() ) );
                break;

            case WidgetStyle::input:
                styler->getInputModels( model, highlight, saturate, material );

                font = styler->getFont();
                align = IFont::left | IFont::middle;

                modelTransforms.add( Transform( Transform::translate, Vector<float>() ) );
                break;

            case WidgetStyle::label:
                font = styler->getFont();
                break;

            default:
                ;
        }
    }

    EpicWidgetStyle::~EpicWidgetStyle()
    {
        if ( text )
            font->releaseText( text );
    }

    void EpicWidgetStyle::onMouseMove( const Vector<float>& mouse, const Vector<float>& pos )
    {
        mouseOver = ( mouse.x > pos.x && mouse.y > pos.y && mouse.x < pos.x + size.x && mouse.y < pos.y + size.y );
    }

    void EpicWidgetStyle::onUpdate( double delta )
    {
        static const float maxAlpha = 0.25f, maxModalShadowAlpha = 0.75f;

        if ( mouseOver && highlightAlpha < maxAlpha )
            highlightAlpha = minimum<float>( highlightAlpha + delta / 0.5f, maxAlpha );
        else if ( !mouseOver && highlightAlpha > 0.0f )
            highlightAlpha = maximum<float>( highlightAlpha - delta / 2.0f, 0.0f );

        if ( selected && satAlpha < 1.0f )
            satAlpha = minimum<float>( satAlpha + delta / 0.25f, 1.0f );
        else if ( !selected && satAlpha > 0.0f )
            satAlpha = maximum<float>( satAlpha - delta / 0.25f, 0.0f );

        if ( hasModal && modalShadowAlpha < maxModalShadowAlpha )
            modalShadowAlpha = minimum<float>( modalShadowAlpha + delta / 0.2f, maxModalShadowAlpha );
        else if ( !hasModal && modalShadowAlpha > 0.0f )
            modalShadowAlpha = maximum<float>( modalShadowAlpha - delta / 0.2f, 0.0f );

        if ( type == WidgetStyle::input )
        {
            cursorAlpha += delta / 0.2f;

            while ( cursorAlpha > 1.0f )
                cursorAlpha -= 2.0f;
        }
    }

    Vector<float> EpicWidgetStyle::beginRender( const Vector<float>& pos )
    {
        MaterialProperties query;

        switch ( type )
        {
            case WidgetStyle::button:
                modelTransforms[0].vector = pos;

                // base model
                query.query = MaterialProperties::setColour;
                query.colour = Colour::white();
                material->query( &query );

                model->render( modelTransforms );

                // highlight
                query.query = MaterialProperties::setColour;
                query.colour = Colour( 0.0f, 0.0f, 0.0f, highlightAlpha );
                material->query( &query );

                highlight->render( modelTransforms );

                if ( text )
                    font->renderText( pos.x + 56.0f, pos.y + 18.0f, text );
                break;

            case WidgetStyle::image:
                modelTransforms[3].vector = pos;

                model->render( modelTransforms );
                break;

            case WidgetStyle::input:
                modelTransforms[0].vector = pos;

                // base model
                query.query = MaterialProperties::setColour;
                query.colour = Colour::white();
                material->query( &query );

                model->render( modelTransforms );

                // highlight
                query.query = MaterialProperties::setColour;
                query.colour = Colour( 0.0f, 0.0f, 0.0f, highlightAlpha );
                material->query( &query );

                highlight->render( modelTransforms );

                // saturate
                query.query = MaterialProperties::setColour;
                query.colour = Colour( 1.0f, 1.0f, 1.0f, satAlpha );
                material->query( &query );

                saturate->render( modelTransforms );

                if ( text )
                    font->renderText( pos.x + 18.0f, pos.y + 18.0f, text );

                if ( selected )
                    font->renderString( pos.x + 18.0f + textDimensions.x, pos.y + 18.0f, "+", Colour( 1.0f, 1.0f, 1.0f, sin( fabs( cursorAlpha ) ) ), IFont::left | IFont::middle );
                break;

            case WidgetStyle::label:
                if ( text )
                    font->renderText( pos.x, pos.y, text );
                break;

            default:
                ;
        }

        return Vector<float>();
    }

    void EpicWidgetStyle::renderStage( RenderStage stage )
    {
        if ( stage == beforeModal && modalShadowAlpha > 0.0f )
        {
            MaterialProperties query;

            query.query = MaterialProperties::setColour;
            query.colour = Colour( 0.0f, 0.0f, 0.0f, modalShadowAlpha );
            material->query( &query );

            modalShadow->render( modelTransforms );
        }
    }

    void EpicWidgetStyle::setProperty( unsigned name, int value )
    {
        switch ( name )
        {
            case WidgetStyle::align: align = value; break;
            case WidgetStyle::hasModal: hasModal = value; break;
            case WidgetStyle::inputType: inputType = ( Input::Type ) value; break;
            case WidgetStyle::selected: selected = value; break;
            default: ;
        }

        if ( hasModal && !modalShadow )
        {
            material = driver->createSolidMaterial( "EpicStyler/modalShadowMat", Colour(), 0 );
            modalShadow = model_ui_modal_shadow( driver, material->reference(), size.x, size.y );
        }
    }

    void EpicWidgetStyle::setProperty( unsigned name, float value )
    {
        switch ( name )
        {
            case WidgetStyle::rotation:
                modelTransforms[2].angle = value;
                break;
        }
    }

    void EpicWidgetStyle::setProperty( unsigned name, const String& value )
    {
        switch ( name )
        {
            case WidgetStyle::text:
                if ( text )
                    font->releaseText( text );

                if ( type == WidgetStyle::input && inputType == Input::password )
                {
                    String stars;

                    for ( unsigned i = 0; i < value.getNumCharsUncached(); i++ )
                        stars += Unicode::Character( '*' );

                    text = font->layoutText( stars, Colour::white(), align );
                }
                else
                    text = font->layoutText( value, Colour::white(), align );

                if ( text )
                    textDimensions = font->getTextDimensions( text );
                else
                    textDimensions = Vector2<float>();
                break;

            case WidgetStyle::textureName:
            {
                Reference<ITexture> texture = styler->getTexture( value );
                material = driver->createSolidMaterial( value + "_material", Colour::white(), texture->reference() );

                PlaneCreationInfo plane( texture->getDimensions().getXy(), Vector<>(), Vector2<>(), Vector2<>( 1.0f, 1.0f ), false, true, material->reference() );
                model = driver->createPlane( "RadianceUIImage", &plane );

                modelTransforms.clear();
                modelTransforms.add( Transform( Transform::translate, Vector<float>() ) );
                modelTransforms.add( Transform( Transform::scale, Vector<float>( 1.0f, 1.0f ) ) );
                modelTransforms.add( Transform( Transform::rotate, Vector<float>( 0.0f, 0.0f, 1.0f ), 0.0f ) );
                modelTransforms.add( Transform( Transform::translate, Vector<float>() ) );
                break;
            }
        }
    }

    void EpicWidgetStyle::setProperty( unsigned name, const Vector<float>& value )
    {
        switch ( name )
        {
            case WidgetStyle::origin:
                modelTransforms[0].vector = -value;
                break;

            case WidgetStyle::scale:
                modelTransforms[1].vector = value;
                break;
        }
    }
}
