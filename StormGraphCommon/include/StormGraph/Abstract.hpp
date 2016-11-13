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

#include <StormGraph/Common.hpp>
#include <StormGraph/Vector.hpp>

namespace StormGraph
{
    static const size_t TEXTURES_PER_VERTEX = 1;

    SgStruct Colour
    {
        public:
            float r, g, b, a;

        public:
            Colour( float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f )
                    : r( r ), g( g ), b( b ), a( a )
            {
            }

            Colour( float channels[4] )
                    : r( channels[0] ), g( channels[1] ), b( channels[2] ), a( channels[3] )
            {
            }

            Colour( int r, int g, int b, int a = 255 )
                    : r( r / 255.0f ), g( g / 255.0f ), b( b / 255.0f ), a( a / 255.0f )
            {
            }

            Colour( String text );
            static Colour fromHsl( const Vector<float>& hsl );
            static Colour fromInts( int r, int g, int b, int a = 255 ) { return Colour( r, g, b, a ); }
            static Colour fromRgbaUint32( uint32_t rgba ) { return Colour::fromInts( rgba & 0xFF, ( rgba >> 8 ) & 0xFF, ( rgba >> 16 ) & 0xFF, ( rgba >> 24 ) & 0xFF ); }

            static Colour blue( float alpha = 1.0f ) { return Colour( 0.0f, 0.0f, 1.0f, alpha ); }
            static Colour green( float alpha = 1.0f ) { return Colour( 0.0f, 1.0f, 0.0f, alpha ); }

            static Colour grey( float level, float alpha = 1.0f )
            {
                return Colour( level, level, level, alpha );
            }

            uint32_t toRgbaUint32() const
            {
                return getR() | ( getG() << 8 ) | ( getB() << 16 ) | ( getA() << 24 );
            }

            String toString() const
            {
                return ( String ) r + ", " + g + ", " + b;
            }

            static Colour white( float alpha = 1.0f )
            {
                return grey( 1.0f, alpha );
            }

            bool equals( const Colour& other, float tolerance ) const;

            unsigned getR() const { return limitTo( ( int )( r * 255.f ), 0, 255 ); }
            unsigned getG() const { return limitTo( ( int )( g * 255.f ), 0, 255 ); }
            unsigned getB() const { return limitTo( ( int )( b * 255.f ), 0, 255 ); }
            unsigned getA() const { return limitTo( ( int )( a * 255.f ), 0, 255 ); }

            Vector<float> toHsl() const;

            /*olour operator + ( const Colour& other )
            {
                Colour result = other * other.a.sum( *this * a * ( 1.0f - other.a ) );
                result /= result.a;

                return result;
            }*/

            Colour& operator += ( const Colour& other )
            {
                r += other.r;
                g += other.g;
                b += other.b;

                return *this;
            }

            Colour& operator /= ( float factor )
            {
                r /= factor;
                g /= factor;
                b /= factor;

                return *this;
            }

            Colour operator + ( const Colour& other ) const
            {
                return Colour( r + other.r, g + other.g, b + other.b, a + other.a );
            }

            Colour operator * ( float factor ) const
            {
                return Colour( r * factor, g * factor, b * factor, a * factor );
            }

            Colour operator * ( const Colour& other ) const
            {
                return Colour( r * other.r, g * other.g, b * other.b, a * other.a );
            }
    };

    inline Colour operator * ( float factor, const Colour& colour )
    {
        return colour * factor;
    }

    SgClass Camera
    {
        public:
            Vector<> eye, center, up;

        public:
            Camera();
            Camera( const Vector<>& eye, const Vector<>& center, const Vector<>& up );
            Camera( const Vector<>& center, float dist, float angle, float angle2 );

            static void convert( float dist, float angle, float angle2, Vector<>& eye, Vector<>& center, Vector<>& up );
            static void convert( const Vector<>& eye, const Vector<>& center, const Vector<>& up, float& dist, float& angle, float& angle2 );

            //const Vector<>& getCenterPos() const { return center; }
            float getDistance() const;
            //const Vector<>& getEyePos() const { return eye; }
            //const Vector<>& getUpVector() const { return up; }

            void move( const Vector<>& vec );

            void setCenterPos( const Vector<>& pos ) { center = pos; }
            void setEyePos( const Vector<>& pos ) { eye = pos; }
            void setUpVector( const Vector<>& pos ) { up = pos; }

            void rotateXY( float alpha, bool absolute = false );
            void rotateZ( float alpha, bool absolute = false );

            void zoom( float amount, bool absolute = false );
    };

    SgClass Plane
    {
        public:
        	Vector<float> normal, point;
        	float d;

        public:
            Plane() {}
        	Plane( const Vector<float>& v1, const Vector<float>& v2, const Vector<float>& v3 );

            float pointDistance( const Vector<float>& point ) const;
        	void set3Points( const Vector<float> &v1, const Vector<float> &v2, const Vector<float> &v3 );
        	void setNormalAndPoint( const Vector<float> &normal, const Vector<float> &point );
        	void setCoefficients( float a, float b, float c, float d );

        	void print();
    };

    struct Transform
    {
        enum Operation { translate, rotate, scale, matrix } operation;
        Vector<float> vector;   // translation, rotation, scaling
        float angle;            // rotation only
        glm::mat4 transformation;

        Transform() {}

        Transform( Operation operation, const Vector<float>& vector, float angle = 0.0f )
                : operation( operation ), vector( vector ), angle( angle )
        {
        }
    };

    struct Vertex
    {
        Vector<> pos, normal;
        Colour colour;
        Vector2<> uv[TEXTURES_PER_VERTEX], lightUv;
    };

    struct VertexProperties
    {
        bool hasNormals, hasColours;
        unsigned numTextures;
        bool hasLightUvs;
    };

    SgStruct Polygon
    {
        static const unsigned MAX_VERTICES = 8;

        unsigned numVertices;
        Vertex v[MAX_VERTICES];

        public:
            unsigned breakIntoTriangles( List<uint32_t>& indices );
    };

    SgStruct BspPolygon
    {
        static const unsigned MAX_VERTICES = Polygon::MAX_VERTICES;

        unsigned numVertices;
        Vertex v[MAX_VERTICES];

        unsigned materialIndex;

        public:
            BspPolygon() {}
            BspPolygon( const Polygon& polygon, unsigned materialIndex );
    };
}
