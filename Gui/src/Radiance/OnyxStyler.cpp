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

#include <Radiance/OnyxStyler.hpp>

namespace Radiance
{
    OnyxStyler::OnyxStyler( IGraphicsDriver* driver ) : driver( driver )
    {
    }

    OnyxStyler::~OnyxStyler()
    {
    }

    WidgetStyle* OnyxStyler::createWidgetStyle( WidgetStyle::Type type, const Vector<float>& size )
    {
        return new OnyxWidgetStyle( driver, type, size );
    }

    OnyxWidgetStyle::OnyxWidgetStyle( IGraphicsDriver* driver, WidgetStyle::Type type, const Vector<float>& size )
            : driver( driver ), type( type ), size( size )
    {
        switch ( type )
        {
            case WidgetStyle::button:
            {
                MaterialProperties materialProperties = { 0, Colour( 0.2f, 0.5f, 1.0f ), Colour( 0.05f, 0.15f, 0.25f ), Colour( 0.1f, 0.25f, 0.5f ), Colour(),
                        Colour( 1.0f, 1.0f, 1.0f, 0.45f ), 3.0f, 0 };
                material = driver->createTexturedMaterial( "onyx_btn_material", &materialProperties );

                CuboidCreationInfo panel( Vector<float>( 1.0f, 1.0f, 1.0f ), Vector<float>( 0.0f, 0.0f, 1.0f ), true, true, false, material->reference() );
                model = driver->createCuboid( "onyx_btn_model", &panel );

                modelTransforms.add( Transform( Transform::scale, Vector<float>( 1.0f, 1.0f, 1.0f ) ) );
                modelTransforms.add( Transform( Transform::translate, Vector<float>() ) );
                break;
            }

            case WidgetStyle::window:
            {
                //MaterialProperties materialProperties = { 0, Colour( 0.5f, 0.2f, 0.1f ), Colour( 0.25f, 0.1f, 0.05f ), Colour( 0.5f, 0.2f, 0.1f ), Colour(),
                //        Colour( 1.0f, 1.0f, 1.0f, 0.75f ), 3.0f, 0 };

                MaterialProperties materialProperties = { 0, Colour( 0.1f, 0.25f, 0.5f ), Colour( 0.05f, 0.15f, 0.25f ), Colour( 0.1f, 0.25f, 0.5f ), Colour(),
                        Colour( 1.0f, 1.0f, 1.0f, 0.75f ), 3.0f, 0 };
                material = driver->createTexturedMaterial( "onyx_wnd_material", &materialProperties );

                CuboidCreationInfo panel( Vector<float>( 1.0f, 1.0f, 1.0f ), Vector<float>( 0.0f, 0.0f, 1.0f ), true, true, false, material->reference() );
                model = driver->createCuboid( "onyx_wnd_model", &panel );

                modelTransforms.add( Transform( Transform::scale, Vector<float>( 1.0f, 1.0f, 1.0f ) ) );
                modelTransforms.add( Transform( Transform::translate, Vector<float>() ) );
                break;
            }

            default:
                ;
        }
    }

    OnyxWidgetStyle::~OnyxWidgetStyle()
    {
    }

    void OnyxWidgetStyle::animateProperty( unsigned name, float value )
    {
        switch ( name )
        {
            //case alpha: material->setAlpha( value ); break;

            case hue:
                MaterialProperties query;
                query.query = MaterialProperties::getAmbient | MaterialProperties::getDiffuse | MaterialProperties::getSpecular;
                material->query( &query );

                Vector<float> ambient = query.ambient.toHsl();
                ambient.x = value;
                query.ambient = Colour::fromHsl( ambient );

                Vector<float> diffuse = query.diffuse.toHsl();
                diffuse.x = value;
                query.diffuse = Colour::fromHsl( diffuse );

                Vector<float> specular = query.specular.toHsl();
                specular.x = value;
                query.specular = Colour::fromHsl( specular );

                query.query = MaterialProperties::setAmbient | MaterialProperties::setDiffuse | MaterialProperties::setSpecular;
                material->query( &query );
                break;
        }
    }

    Vector<float> OnyxWidgetStyle::beginRender( const Vector<float>& pos )
    {
        switch ( type )
        {
            case WidgetStyle::button:
            case WidgetStyle::window:
                modelTransforms[0].vector = size;
                modelTransforms[1].vector = pos;

                driver->disableCulling();
                driver->disableDepthTesting();
                model->render( modelTransforms );
                driver->enableDepthTesting();
                driver->enableCulling();
                break;

            default:
                ;
        }

        return Vector<float>();
    }

    void OnyxWidgetStyle::endRender()
    {
    }

    void* OnyxWidgetStyle::getInterface( const char* name )
    {
        if ( strcmp( name, "Radiance.OnyxWidgetStyle" ) == 0 )
            return this;
        else
            return Resource::getInterface( name );
    }

    void OnyxWidgetStyle::setProperty( unsigned name, int value )
    {
    }

    void OnyxWidgetStyle::setProperty( unsigned name, float value )
    {
    }

    void OnyxWidgetStyle::setProperty( unsigned name, const String& value )
    {
    }

    void OnyxWidgetStyle::setProperty( unsigned name, const Vector<float>& value )
    {
    }

    void OnyxWidgetStyle::setTexture( ITexture* texture )
    {
        MaterialProperties query;
        query.query = MaterialProperties::setTexture;
        query.texture = texture;

        material->query( &query );
    }
}
