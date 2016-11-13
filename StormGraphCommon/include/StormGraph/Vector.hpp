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

#include <littl/String.hpp>

#include <cmath>
#include <glm/glm.hpp>

namespace StormGraph
{
    using namespace li;

    template <typename Unit = float> struct Vector2
    {
        Unit x, y;

        public:
            Vector2( Unit x = 0, Unit y = 0 ) : x( x ), y( y )
            {
            }

            Vector2( String text ) : x( 0 ), y( 0 )
            {
                List<String> components;

                text.parse( components, ',' );

                if ( components.getLength() >= 1 )
                    x = strtol(components[0], 0, 0);

                if ( components.getLength() >= 2 )
                    y = strtol(components[1], 0, 0);
            }

            template <typename OtherUnit>
            Vector2( const Vector2<OtherUnit>& other ) : x( ( Unit ) other.x ), y( ( Unit ) other.y )
            {
            }

            Vector2<Unit> operator - () const
            {
                return Vector2<Unit>( -x, -y );
            }

            void operator += ( const Vector2<Unit>& other )
            {
                x += other.x;
                y += other.y;
            }

            void operator -= ( const Vector2<Unit>& other )
            {
                x -= other.x;
                y -= other.y;
            }

            template <typename OtherUnit>
            void operator *= ( OtherUnit factor )
            {
                x *= factor;
                y *= factor;
            }

            template <typename OtherUnit>
            void operator *= ( const Vector2<OtherUnit>& other )
            {
                x *= other.x;
                y *= other.y;
            }

            template <typename OtherUnit>
            void operator /= ( OtherUnit factor )
            {
                x /= factor;
                y /= factor;
            }

            bool operator == ( const Vector2<Unit>& other ) const
            {
                return x == other.x && y == other.y;
            }

            bool operator > ( const Vector2<Unit>& other ) const
            {
                return x > other.x || y > other.y;
            }

            Vector2<Unit> operator - ( const Vector2<Unit>& other ) const
            {
                return Vector2<Unit>( x - other.x, y - other.y );
            }

            Vector2<Unit> operator - ( Unit value ) const
            {
                return Vector2<Unit>( x - value, y - value );
            }

            Vector2<Unit> operator + ( const Vector2<Unit>& other ) const
            {
                return Vector2<Unit>( x + other.x, y + other.y );
            }

            Vector2<Unit> operator + ( Unit value ) const
            {
                return Vector2<Unit>( x + value, y + value );
            }

            Vector2<Unit> operator * ( Unit factor ) const
            {
                return Vector2<Unit>( x * factor, y * factor );
            }

            Vector2<Unit> operator * ( const Vector2<Unit>& other ) const
            {
                return Vector2<Unit>( x * other.x, y * other.y );
            }

            template <typename OtherUnit>
            Vector2<Unit> operator / ( const Vector2<OtherUnit>& other ) const
            {
                return Vector2<Unit>( x / other.x, y / other.y );
            }

            template <typename OtherUnit>
            Vector2<Unit> operator / ( OtherUnit factor ) const
            {
                return Vector2<Unit>( x / factor, y / factor );
            }

            template <typename OtherUnit>
            Vector2<Unit> operator % ( const Vector2<OtherUnit>& other ) const
            {
                return Vector2<Unit>( fmod( x, other.x ), fmod( y, other.y ) );
            }

            bool operator < ( const Vector2<Unit>& other ) const { return x < other.x || y < other.y; }

            Unit& operator [] ( size_t index ) { return (&x)[index]; }
            const Unit& operator [] ( size_t index ) const { return (&x)[index]; }

            Vector2<Unit> ceil() const { return Vector2<Unit>( ::ceil( x ), ::ceil( y ) ); }
            template <typename NewUnit> Vector2<NewUnit> convert() { return Vector2<NewUnit>( ( NewUnit ) x, ( NewUnit ) y ); }
            Unit dot( const Vector2<Unit>& other ) const { return x * other.x + y * other.y; }
            bool equals( const Vector2<float>& other, float tolerance ) const { return ::fabs( x - other.x ) <= tolerance && ::fabs( y - other.y ) <= tolerance; }
            Vector2<Unit> fabs() const { return Vector2<Unit>( ::fabs( x ), ::fabs( y ) ); }
            Vector2<Unit> floor() const { return Vector2<Unit>( ::floor( x ), ::floor( y ) ); }
            Unit getLength() const { return ( Unit )( sqrt( x * x + y * y ) ); }
            Vector2<Unit> maximum( Unit b ) const { return Vector2<Unit>( li::maximum( x, b ), li::maximum( y, b ) ); }
            Vector2<Unit> maximum( const Vector2<Unit>& other ) const { return Vector2<Unit>( li::maximum( x, other.x ), li::maximum( y, other.y ) ); }
            Vector2<Unit> minimum( Unit b ) const { return Vector2<Unit>( li::minimum( x, b ), li::minimum( y, b ) ); }
            Vector2<Unit> minimum( const Vector2<Unit>& other ) const { return Vector2<Unit>( li::minimum( x, other.x ), li::minimum( y, other.y ) ); }

            Vector2<Unit> normalize() const
            {
                Unit length = getLength();

                if ( length != 0 )
                    return Vector2<Unit>( x / length, y / length );
                else
                    return Vector2<Unit>();
            }

            String toString() const
            {
                return ( String ) x + ", " + y;
            }
    };

    template <typename Unit = float> struct Vector
    {
        Unit x, y, z, w;

        public:
            Vector( String text ) : x( 0 ), y( 0 ), z( 0 ), w( 0 )
            {
                int comma = text.findChar( ',' );

                if ( comma < 0 )
                    x = text;
                else
                {
                    x = text.leftPart( comma );
                    text = text.dropLeftPart( comma + 1 );

                    comma = text.findChar( ',' );

                    if ( comma < 0 )
                        y = text;
                    else
                    {
                        y = text.leftPart( comma );
                        text = text.dropLeftPart( comma + 1 );

                        comma = text.findChar( ',' );

                        if ( comma < 0 )
                            z = text;
                        else
                        {
                            z = text.leftPart( comma );
                            w = text.dropLeftPart( comma + 1 );
                        }
                    }
                }
            }

            Vector( Unit x = 0, Unit y = 0, Unit z = 0, Unit w = 0 ) : x( x ), y( y ), z( z ), w( w )
            {
            }

            template <typename OtherUnit>
            Vector( const Vector<OtherUnit>& other ) : x( ( Unit ) other.x ), y( ( Unit ) other.y ), z( ( Unit ) other.z ), w( ( Unit ) other.w )
            {
            }

            template <typename OtherUnit>
            Vector( const Vector2<OtherUnit>& v2 ) : x( ( Unit ) v2.x ), y( ( Unit ) v2.y ), z( ( Unit ) 0 ), w( ( Unit ) 0 )
            {
            }

#ifdef StormGraph_Helium
            Vector( const Helium::VectorFixedObject* vfo ) : x( ( Unit )vfo->x ), y( ( Unit )vfo->y ), z( ( Unit )vfo->z )
            {
            }
#endif

            //static Vector<Unit> fromDirUint16( uint16_t vec );

            Vector<Unit>& operator = ( const Vector<Unit>& other )
            {
                x = other.x;
                y = other.y;
                z = other.z;
                w = other.w;

                return *this;
            }

            void operator += ( const Vector<Unit>& other )
            {
                x += other.x;
                y += other.y;
                z += other.z;
                w += other.w;
            }

            void operator -= ( const Vector<Unit>& other )
            {
                x -= other.x;
                y -= other.y;
                z -= other.z;
                w -= other.w;
            }

            template <typename OtherUnit>
            void operator *= ( OtherUnit factor )
            {
                x *= factor;
                y *= factor;
                z *= factor;
                w *= factor;
            }

            template <typename OtherUnit>
            void operator /= ( OtherUnit factor )
            {
                x /= factor;
                y /= factor;
                z /= factor;
                w /= factor;
            }

            Vector<Unit> operator - ( const Vector<Unit>& other ) const
            {
                return Vector<Unit>( x - other.x, y - other.y, z - other.z, w - other.w );
            }

            Vector<Unit> operator + ( const Vector<Unit>& other ) const
            {
                return Vector<Unit>( x + other.x, y + other.y, z + other.z, w + other.w );
            }

            Vector<Unit> operator * ( Unit factor ) const
            {
                return Vector<Unit>( x * factor, y * factor, z * factor, w * factor );
            }

            Vector<Unit> operator * ( const Vector<Unit>& other ) const
            {
                return Vector<Unit>( x * other.x, y * other.y, z * other.z, w * other.w );
            }

            Vector<Unit> operator / ( Unit factor ) const
            {
                return Vector<Unit>( x / factor, y / factor, z / factor, w / factor );
            }

            Vector<Unit> operator / ( const Vector<Unit>& other ) const
            {
                return Vector<Unit>( x / other.x, y / other.y, z / other.z, w / other.w );
            }

            bool operator < ( const Vector<Unit>& other ) const
            {
                return x < other.x || y < other.y || z < other.z || w < other.w;
            }

            bool operator > ( const Vector<Unit>& other ) const
            {
                return x > other.x || y > other.y || z > other.z || w > other.w;
            }

            template <typename NewUnit> Vector<NewUnit> convert()
            {
                return Vector<NewUnit>( ( NewUnit ) x, ( NewUnit ) y, ( NewUnit ) z, ( NewUnit ) w );
            }

            Vector<Unit> crossProduct( const Vector<Unit>& other ) const
            {
                return Vector<Unit>( y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x, w );
            }

            Unit dotProduct( const Vector<Unit>& other ) const { return x * other.x + y * other.y + z * other.z; }

            bool equals( const Vector<float>& other, float tolerance ) const
            {
                return fabs( x - other.x ) <= tolerance && fabs( y - other.y ) <= tolerance && fabs( z - other.z ) <= tolerance && fabs( w - other.w ) <= tolerance;
            }

            Unit get( int axis ) const
            {
                switch ( axis )
                {
                    case 0: return x;
                    case 1: return y;
                    case 2: return z;
                    case 3: return w;
                    default: return 0;
                }
            }

            Unit getLength() const { return ( Unit )( sqrt( x * x + y * y + z * z ) ); }
            Vector2<Unit> getXy() const { return Vector2<Unit>( x, y ); }

            Vector<Unit> maximum( const Vector<Unit>& other ) const { return Vector<Unit>( li::maximum( x, other.x ), li::maximum( y, other.y ), li::maximum( z, other.z ), li::maximum( w, other.w ) ); }
            Vector<Unit> minimum( const Vector<Unit>& other ) const { return Vector<Unit>( li::minimum( x, other.x ), li::minimum( y, other.y ), li::minimum( z, other.z ), li::minimum( w, other.w ) ); }

            Vector<Unit> normalize() const
            {
                Unit length = getLength();

                if ( length != 0 )
                    return Vector<Unit>( x / length, y / length, z / length, w );
                else
                    return Vector<Unit>();
            }

            template <typename Angle> Vector<Unit> rotate( const Vector<Unit>& axis, Angle angle )
            {
                Unit xx = axis.x * x;
                Unit xy = axis.x*y;
                Unit xz = axis.x*z;
                Unit yx = axis.y*x;
                Unit yy = axis.y*y;
                Unit yz = axis.y*z;
                Unit zx = axis.z*x;
                Unit zy = axis.z*y;
                Unit zz = axis.z*z;
                float sa = sin( angle );
                float ca = cos( angle );

                return Vector<Unit>( axis.x * ( xx + yy + zz ) + ( x * ( axis.y * axis.y + axis.z * axis.z ) - axis.x * ( yy + zz ) ) * ca + ( -zy + yz ) * sa,
                        y =axis.y*(xx+yy+zz)+(y*(axis.x*axis.x+axis.z*axis.z)-axis.y*(xx+zz))*ca+(zx-xz)*sa,
                        z =axis.z*(xx+yy+zz)+(z*(axis.x*axis.x+axis.y*axis.y)-axis.z*(xx+yy))*ca+(-yx+xy)*sa );
            }

            template <typename Angle> Vector<Unit> rotateX( Angle angle ) const
            {
                return Vector<Unit>( x, y * cos( angle ) - z * sin( angle ), y * sin( angle ) + z * cos( angle ) );
            }

            template <typename Angle> Vector<Unit> rotateY( Angle angle ) const
            {
                return Vector<Unit>( z * sin( angle ) + x * cos( angle ), y, z * cos( angle ) - x * sin( angle ) );
            }

            template <typename Angle> Vector<Unit> rotateZ( Angle angle ) const
            {
                return Vector<Unit>( x * cos( angle ) - y * sin( angle ), x * sin( angle ) + y * cos( angle ), z );
            }

            Vector<Unit> round() const { return Vector<Unit>( li::round( x ), li::round( y ), li::round( z ), li::round( w ) ); }
            Vector<Unit> roundUp() const { return Vector<Unit>( ceil( x ), ceil( y ), ceil( z ), ceil( w ) ); }

            void set( int axis, Unit value )
            {
                switch ( axis )
                {
                    case 0: x = value; break;
                    case 1: y = value; break;
                    case 2: z = value; break;
                    case 3: w = value; break;
                }
            }

            String toString() const
            {
                if ( w == 0.0f )
                    return ( String ) x + ", " + y + ", " + z;
                else
                    return ( String ) x + ", " + y + ", " + z + ", " + w;
            }

            /*uint16_t toDirUint16() const
            {
                uint8_t xx = ( int8_t )::round( x * 127.0f );
                uint8_t yy = ( int8_t )::round( y * 127.0f );

                return xx | ( yy << 8 );
            }*/

            Vector<Unit> operator - () const { return Vector<Unit>( -x, -y, -z, -w ); }

            operator glm::vec3 () const { return glm::vec3( x, y, z ); }
            operator glm::vec4 () const { return glm::vec4( x, y, z, w ); }
    };

    /*template <typename Unit> Vector<Unit> Vector<Unit>::fromDirUint16( uint16_t vec )
    {
        float x = ( int8_t )( vec & 0xFF ) / 127.0f;
        float y = ( int8_t )( vec >> 8 ) / 127.0f;

        float z2 = 1.0f - x * x - y * y;
        float z = ( z2 > 0.0f ) ? sqrt( z2 ) : z;

        return Vector<Unit>( x, y, z );
    }*/

    template <typename Unit> inline Vector2<Unit> operator * ( Unit factor, const Vector2<Unit>& vector )
    {
        return vector * factor;
    }

    template <typename Unit> inline Vector<Unit> operator * ( Unit factor, const Vector<Unit>& vector )
    {
        return vector * factor;
    }
}
