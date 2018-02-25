
#include <StormGraph/StormGraph.hpp>

namespace StormGraph
{
    Vector<float> Camera::currentCamera, Camera::currentUp;

    void Camera::look( const Vector<float>& eye, const Vector<float>& center, const Vector<float>& up )
    {
        currentCamera = eye;
        currentUp = up;

        glLoadIdentity();
        gluLookAt( -eye.x, eye.y, eye.z, -center.x, center.y, center.z, -up.x, up.y, up.z );
        glScalef( -1.f, 1.f, 1.f );

        Engine::getInstance()->frustum.setView( eye, center, up );
    }

    void Camera::look( const Vector<float>& center, float dist, float angle, float angle2 )
    {
        float radius = cos( angle2 ) * dist;
        float upRadius = radius - sin( angle2 );

        Vector<float> cam( cos( angle + ( float )( M_PI * 1.5f ) ) * radius, -sin( angle + ( float )( M_PI * 1.5f ) ) * radius, sin( angle2 ) * dist );
        Vector<float> up( cos( angle + ( float )( M_PI * 1.5f ) ) * upRadius - cam.x, -sin( angle + ( float )( M_PI * 1.5f ) ) * upRadius - cam.y, cos( angle2 ) );

        look( center + cam, center, up );
    }

    void Camera::moveEye( const Vector<float>& vector, bool relative )
    {
        if ( relative )
            eye += vector;
        else
            eye = vector;
    }

    void FpsCamera::select()
    {
        look( eye, center, Vector<float>( 0.f, 0.f, 1.f ) );
    }

    void FpsCamera::moveCenter( const Vector<float>& vector, bool relative )
    {
        if ( relative )
            center += vector;
        else
            center = vector;
    }

    void TopDownCamera::select()
    {
        look( eye, Vector<float>( eye.x, eye.y, 0.f ), Vector<float>( 0.f, -1.f, 0.f ) );
    }
}
