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

#include "OpenGlDriver.hpp"

namespace OpenGlDriver
{
    Light::Light( OpenGlDriver* driver, const DirectionalLightProperties* properties )
            : driver( driver ), type( ILight::directional )
    {
        directional = new DirectionalLightProperties;
        memcpy( directional, properties, sizeof( DirectionalLightProperties ) );
    }

    Light::Light( OpenGlDriver* driver, const PointLightProperties* properties )
            : driver( driver ), type( ILight::point )
    {
        point = new PointLightProperties;
        memcpy( point, properties, sizeof( PointLightProperties ) );
    }

    Light::~Light()
    {
    }

    /*void Light::render( const List<Transform>& transforms )
    {
        render( transforms.getPtr(), transforms.getLength() );
    }*/

    void Light::render( const Transform* transforms, size_t count, bool inWorldSpace )
    {
        glPushMatrix();

        OpenGlDriver::applyTransforms( transforms, count );

        if ( type == ILight::directional )
            driver->addDirectionalLight( *directional, inWorldSpace );
        else
            driver->addPointLight( *point, inWorldSpace );

        glPopMatrix();

        /*else
        {
            glEnable( GL_LIGHT0 + id );

            GLfloat ambientRGBA[] = { ambient.r, ambient.g, ambient.b, ambient.a };
            GLfloat diffuseRGBA[] = { diffuse.r, diffuse.g, diffuse.b, diffuse.a };

            glLightfv( GL_LIGHT0 + id, GL_AMBIENT, ambientRGBA );
            glLightfv( GL_LIGHT0 + id, GL_DIFFUSE, diffuseRGBA );
            glLightfv( GL_LIGHT0 + id, GL_SPECULAR, diffuseRGBA );

            if ( type == ILight::positional )
            {
                // 1.f == positional

                GLfloat position[] = { 0.0f, 0.0f, 0.0f, 1.0f };
                glLightfv( GL_LIGHT0 + id, GL_POSITION, position );
            }
            /*else if ( type == line )
            {
                // notling like a line light in OpenGL
                // emulate it with point

                GLfloat position[] = { direction.x / 2, direction.y / 2, direction.z / 2, 1.0f };
                glLightfv( GL_LIGHT0 + id, GL_POSITION, position );
            }*/
            /*else
            {
                // dir[] actually specifies a relative position in the space
                // included is also a HACK because of BUGS (in OpenGL)

                GLfloat dir[] = { -direction.x - 0.0001f, -direction.y - 0.0001f, -direction.z - 0.0001f, 0.f };
                glLightfv( GL_LIGHT0 + id, GL_POSITION, dir );
            }

            glLightf( GL_LIGHT0 + id, GL_CONSTANT_ATTENUATION, 0.f );
            glLightf( GL_LIGHT0 + id, GL_LINEAR_ATTENUATION, 2.f / range );
            glLightf( GL_LIGHT0 + id, GL_QUADRATIC_ATTENUATION, 0.f );*/

            /*glLightf( GL_LIGHT0 + id, GL_SPOT_CUTOFF, cutoffAngle / 2 );

            GLfloat dir[] = { direction.x, direction.y, direction.z };
            glLightfv( GL_LIGHT0 + id, GL_SPOT_DIRECTION, dir );*/
        /*}

        glPopMatrix();*/
    }

    /*void Light::setCutoff( float angle )
    {
        cutoffAngle = angle;
    }*/
}
