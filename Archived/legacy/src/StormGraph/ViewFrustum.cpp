
#include <StormGraph/StormGraph.hpp>

namespace StormGraph
{
    Plane::Plane( Vector<float> &v1,  Vector<float> &v2,  Vector<float> &v3)
    {
    	set3Points( v1, v2, v3 );
    }

    Plane::Plane()
    {
    }

    Plane::~Plane()
    {
    }

    void Plane::set3Points( Vector<float>& v1,  Vector<float>& v2,  Vector<float>& v3 )
    {
    	Vector<float> aux1, aux2;

    	aux1 = v1 - v2;
    	aux2 = v3 - v2;

    	normal = aux2.crossProduct( aux1 ).normalize();

    	point = v2;
    	d = -normal.dotProduct( point );
    }

    void Plane::setNormalAndPoint( Vector<float>& newNormal, Vector<float>& point )
    {
    	normal = newNormal.normalize();
    	d = -normal.dotProduct( point );
    }

    void Plane::setCoefficients( float a, float b, float c, float d )
    {
    	float l = Vector<float>( a, b, c ).getLength();
    	normal = Vector<float>( a / l, b / l, c / l );
    	this->d = d / l;
    }

    float Plane::distance( const Vector<float>& point )
    {
    	return d + normal.dotProduct( point );
    }

    void Plane::print()
    {
    	printf( "Plane( %f, %f, %f # %f )\n", normal.x, normal.y, normal.z, d );
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

    	tang = ( float )tan( angle * M_PI / 180.f * 0.5 );
    	nh = nearD * tang;
    	nw = nh * ratio;
    	fh = farD  * tang;
    	fw = fh * ratio;
    }

    void ViewFrustum::setView( const Vector<float> &p, const Vector<float> &l, const Vector<float> &u )
    {
    	Vector<float> dir, nc, fc, X, Y, Z;

    	Z = ( p - l ).normalize();
    	X = u.crossProduct( Z ).normalize();
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
    	//planes[near].set3Points( ntl, ntr, nbr );
    	//planes[far].set3Points( ftr, ftl, fbl );
    }

    int ViewFrustum::pointInFrustum( const Vector<float>& point )
    {
        // For the sake of speed, near & far are ignored
        // Applies also to sphereInFrustum()

    	for ( int i = 0; i < 4; i++ )
    		if ( planes[i].distance( point ) < 0 )
    			return outside;

    	return inside;

    }

    int ViewFrustum::sphereInFrustum( const Vector<float>& center, float radius )
    {
        int result = inside;
    	float distance;

    	for ( int i = 0; i < 4; i++ )
        {
    		distance = planes[i].distance( center );

    		if ( distance < -radius )
    			return outside;
    		else if ( distance < radius )
    			result = intersect;
    	}

    	return result;
    }

    /*
    int ViewFrustum::boxInFrustum(AABox &b) {

    	int result = INSIDE;
    	for(int i=0; i < 6; i++) {

    		if (pl[i].distance(b.getVertexP(pl[i].normal)) < 0)
    			return OUTSIDE;
    		else if (pl[i].distance(b.getVertexN(pl[i].normal)) < 0)
    			result =  INTERSECT;
    	}
    	return(result);

     }
    */
}
