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
    static float hue2rgb( float p, float q, float t )
    {
        while ( t < 0.0f )
            t += 1.0f;

        while ( t > 1.0f )
            t -= 1.0f;

        if ( t < 1.0f / 6.0f )
            return p + ( q - p ) * 6.0f * t;

        if ( t < 1.0f / 2.0f )
            return q;

        if ( t < 2.0f / 3.0f )
            return p + ( q - p ) * ( 2.0f / 3.0f - t ) * 6.0f;

        return p;
    }

    Colour::Colour( String text ) : r( 0.0f ), g( 0.0f ), b( 0.0f ), a( 1.0f )
    {
        List<String> components;

        text.parse( components, ',' );

        if ( components.getLength() >= 1 )
            r = components[0].toFloat();

        if ( components.getLength() >= 2 )
            g = components[1].toFloat();

        if ( components.getLength() >= 3 )
            b = components[2].toFloat();

        if ( components.getLength() >= 4 )
            a = components[3].toFloat();
    }

    bool Colour::equals( const Colour& other, float tolerance ) const
    {
        return fabs( r - other.r ) <= tolerance && fabs( g - other.g ) <= tolerance && fabs( b - other.b ) <= tolerance && fabs( a - other.a ) <= tolerance;
    }

    Colour Colour::fromHsl( const Vector<float>& hsl )
    {
        const float h = hsl.x, s = hsl.y, l = hsl.z, a = hsl.w;

        if ( s == 0 )
            return Colour( l, l, l, a );
        else
        {
            float q = l < 0.5 ? l * ( 1 + s ) : l + s - l * s;
            float p = 2 * l - q;

            return Colour( hue2rgb( p, q, h + 1.0f / 3.0f ), hue2rgb( p, q, h ), hue2rgb( p, q, h - 1.0f / 3.0f ) );
        }
    }

    Vector<float> Colour::toHsl() const
    {
        const float max = maximum( r, g, b ), min = minimum( r, g, b );
        const float l = ( max + min ) / 2;

        if ( max == min )
            return Vector<float>( 0.0f, 0.0f, l, a );
        else
        {
            const float d = max - min;
            const float s = l > 0.5 ? d / ( 2 - max - min ) : d / ( max + min );

            float h;

            if ( max == r )
                h = ( g - b ) / d + (g < b ? 6 : 0);
            else if ( max == g )
                h = ( b - r ) / d + 2;
            else
                h = ( r - g ) / d + 4;

            h /= 6;

            return Vector<float>( h, s, l, a );
        }
    }

    Plane::Plane( const Vector<float>& v1, const Vector<float>& v2, const Vector<float>& v3 )
    {
    	set3Points( v1, v2, v3 );
    }

    float Plane::pointDistance( const Vector<float>& point ) const
    {
    	return d + normal.dotProduct( point );
    }

    void Plane::set3Points( const Vector<float>& v1, const Vector<float>& v2, const Vector<float>& v3 )
    {
    	Vector<float> aux1, aux2;

    	aux1 = v1 - v2;
    	aux2 = v3 - v2;

    	normal = aux2.crossProduct( aux1 ).normalize();

    	point = v2;
    	d = -normal.dotProduct( point );
    }

    void Plane::setNormalAndPoint( const Vector<float>& newNormal, const Vector<float>& point )
    {
    	normal = newNormal.normalize();
    	d = -normal.dotProduct( point );
    }

    void Plane::setCoefficients( float a, float b, float c, float d )
    {
    	float length = Vector<float>( a, b, c ).getLength();
    	normal = Vector<float>( a / length, b / length, c / length );
    	this->d = d / length;
    }

    void Plane::print()
    {
    	printf( "Plane( %f, %f, %f # %f )\n", normal.x, normal.y, normal.z, d );
    }

    unsigned Polygon::breakIntoTriangles( List<unsigned>& indices )
    {
        if ( numVertices < 3 )
            return 0;

        indices.add( 0 );
        indices.add( 1 );
        indices.add( numVertices - 1 );

        bool even = false;

        for ( unsigned i = 0; i < numVertices - 3; i++ )
        {
            if ( !even )
            {
                indices.add( numVertices - 1 - i / 2 );
                indices.add( 1 + i / 2 );
                indices.add( 2 + i / 2 );
            }
            else
            {
                indices.add( numVertices - 1 - i / 2 );
                indices.add( 2 + i / 2 );
                indices.add( numVertices - 2 - i / 2 );
            }

            even = !even;
        }

        return numVertices - 2;
    }
}
