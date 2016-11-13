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

#include <StormGraph/Abstract.hpp>

namespace StormGraph
{
    Camera::Camera()
    {
    }

    Camera::Camera( const Vector<>& eye, const Vector<>& center, const Vector<>& up )
            : eye( eye ), center( center ), up( up )
    {
    }

    Camera::Camera( const Vector<>& center, float dist, float angle, float angle2 )
            : center( center )
    {
        convert( dist, angle, angle2, eye, this->center, up );
    }

    void Camera::convert( float dist, float angle, float angle2, Vector<>& eye, Vector<>& center, Vector<>& up )
    {
        float ca = cos( angle/* + ( float )( M_PI * 1.5f )*/ );
        float sa = sin( angle/* + ( float )( M_PI * 1.5f )*/ );

        float ca2 = cos( angle2 );
        float sa2 = sin( angle2 );

        float radius = ca2 * dist;
        float upRadius = radius - sa2;

        Vector<> cam( ca * radius, -sa * radius, sa2 * dist );

        eye = center + cam;
        up = Vector<>( ca * upRadius - cam.x, -sa * upRadius - cam.y, ca2 );
    }

    void Camera::convert( const Vector<>& eye, const Vector<>& center, const Vector<>& up, float& dist, float& angle, float& angle2 )
    {
        Vector<> cam = eye - center;

        dist = cam.getLength();
        angle = atan2( -cam.y, cam.x );
        angle2 = atan2( cam.z, cam.getXy().getLength() );
    }

    float Camera::getDistance() const
    {
        return ( eye - center ).getLength();
    }

    /*void Camera::look( GraphicsDriver* driver, Vector<> center, float dist, float angle, float angle2 )
    {
        Vector<> eye, up;

        convert( dist, angle, angle2, eye, center, up );

        //Engine::getInstance()->getGraphicsDriver()
        driver->setCamera( eye, center, up );
    }*/

    void Camera::move( const Vector<>& vec )
    {
        eye += vec;
        center += vec;
    }

    void Camera::rotateXY( float alpha, bool absolute )
    {
        float dist, angle, angle2;

        convert( eye, center, up, dist, angle, angle2 );

        if ( absolute )
            angle2 = alpha;
        else
            angle2 += alpha;

        convert( dist, angle, angle2, eye, center, up );
    }

    void Camera::rotateZ( float alpha, bool absolute )
    {
        float dist, angle, angle2;

        convert( eye, center, up, dist, angle, angle2 );

        if ( absolute )
            angle = alpha;
        else
            angle += alpha;

        convert( dist, angle, angle2, eye, center, up );
    }

    void Camera::zoom( float amount, bool absolute )
    {
        float dist, angle, angle2;

        convert( eye, center, up, dist, angle, angle2 );

        if ( absolute )
            dist = amount;
        else if ( dist + amount > 0.0f )
            dist += amount;

        convert( dist, angle, angle2, eye, center, up );
    }
}
