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
    static Vector<float> getPositiveVertex( const Vector<float>& min, const Vector<float>& max, const Vector<float>& normal )
    {
        Vector<float> result = min;

        if ( normal.x > 0 )
            result.x = max.x;

        if ( normal.y > 0 )
            result.y = max.y;

        if ( normal.z > 0 )
            result.z = max.z;

        return result;
    }

    static Vector<float> getNegativeVertex( const Vector<float>& min, const Vector<float>& max, const Vector<float>& normal )
    {
        Vector<float> result = min;

        if ( normal.x < 0 )
            result.x = max.x;

        if ( normal.y < 0 )
            result.y = max.y;

        if ( normal.z < 0 )
            result.z = max.z;

        return result;
    }

    ViewFrustum::ViewFrustum()
    {
    }

    ViewFrustum::~ViewFrustum()
    {
    }

    void ViewFrustum::setProjection( float angle, float ratio, float nearD, float farD )
    {
    	this->ratio = ratio;
    	this->angle = angle;
    	this->nearD = nearD;
    	this->farD = farD;

    	tang = ( float ) tan( angle * 0.5 * M_PI / 180.f );
    	nh = nearD * tang;
    	nw = nh * ratio;
    	fh = farD  * tang;
    	fw = fh * ratio;
    }

    void ViewFrustum::setView( const Vector<float>& p, const Vector<float>& l, const Vector<float>& up )
    {
    	Vector<float> dir, nc, fc, X, Y, Z;

    	Z = ( p - l ).normalize();
    	X = up.crossProduct( Z ).normalize();
    	Y = Z.crossProduct( X );

    	nc = p - Z * nearD;
    	fc = p - Z * farD;

    	ntl = nc + Y * nh - X * nw;
    	ntr = nc + Y * nh + X * nw;
    	nbl = nc - Y * nh - X * nw;
    	nbr = nc - Y * nh + X * nw;

    	ftl = fc + Y * fh - X * fw;
    	ftr = fc + Y * fh + X * fw;
    	fbl = fc - Y * fh - X * fw;
    	fbr = fc - Y * fh + X * fw;

    	planes[top].set3Points( ntr, ntl, ftl );
    	planes[bottom].set3Points( nbl, nbr, fbr );
    	planes[left].set3Points( ntl, nbl, fbl );
    	planes[right].set3Points( nbr, ntr, fbr );
    	planes[nearClip].set3Points( ntl, ntr, nbr );
    	planes[farClip].set3Points( ftr, ftl, fbl );
    }

    ViewFrustum::TestResult ViewFrustum::pointInFrustum( const Vector<float>& point )
    {
        // For the sake of speed, near & far are ignored
        // Applies also to sphereInFrustum()

    	for ( int i = 0; i < 6; i++ )
    		if ( planes[i].pointDistance( point ) < 0 )
    			return outside;

    	return inside;
    }

    ViewFrustum::TestResult ViewFrustum::sphereInFrustum( const Vector<float>& center, float radius )
    {
        TestResult result = inside;
    	float distance;

    	for ( int i = 0; i < 6; i++ )
        {
    		distance = planes[i].pointDistance( center );

    		if ( distance < -radius )
    			return outside;
    		else if ( distance < radius )
    			result = intersect;
    	}

    	return result;
    }

    ViewFrustum::TestResult ViewFrustum::boxInFrustum( const Vector<float>& min, const Vector<float>& max )
    {
    	TestResult result = inside;

    	for ( int i = 0; i < 6; i++ )
    	{
    		if ( planes[i].pointDistance( getPositiveVertex( min, max, planes[i].normal ) ) < 0 )
    			return outside;
    		else if ( planes[i].pointDistance( getNegativeVertex( min, max, planes[i].normal ) ) < 0 )
    			result = intersect;
    	}

    	return result;
    }
}

