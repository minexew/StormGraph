
#include <StormGraph/StormGraph.hpp>

namespace StormGraph
{
    bool Math::boxesCollide( const Vector<float>& min1, const Vector<float>& max1, const Vector<float>& min2, const Vector<float>& max2 )
    {
        return liesInBox( Vector<float>( min1.x, min1.y, min1.z ), min2, max2 ) || liesInBox( Vector<float>( max1.x, min1.y, min1.z ), min2, max2 )
                || liesInBox( Vector<float>( min1.x, max1.y, min1.z ), min2, max2 ) || liesInBox( Vector<float>( max1.x, max1.y, min1.z ), min2, max2 )
                || liesInBox( Vector<float>( min1.x, min1.y, max1.z ), min2, max2 ) || liesInBox( Vector<float>( max1.x, min1.y, max1.z ), min2, max2 )
                || liesInBox( Vector<float>( min1.x, max1.y, max1.z ), min2, max2 ) || liesInBox( Vector<float>( max1.x, max1.y, max1.z ), min2, max2 )
                || liesInBox( Vector<float>( min2.x, min2.y, min2.z ), min1, max1 ) || liesInBox( Vector<float>( max2.x, min2.y, min2.z ), min1, max1 )
                || liesInBox( Vector<float>( min2.x, max2.y, min2.z ), min1, max1 ) || liesInBox( Vector<float>( max2.x, max2.y, min2.z ), min1, max1 )
                || liesInBox( Vector<float>( min2.x, min2.y, max2.z ), min1, max1 ) || liesInBox( Vector<float>( max2.x, min2.y, max2.z ), min1, max1 )
                || liesInBox( Vector<float>( min2.x, max2.y, max2.z ), min1, max1 ) || liesInBox( Vector<float>( max2.x, max2.y, max2.z ), min1, max1 );
    }

    bool Math::liesInBox( const Vector<float>& point, const Vector<float>& min, const Vector<float>& max )
    {
        return point.x >= min.x && point.y >= min.y && point.z >= min.z && point.x <= max.x && point.y <= max.y && point.z <= max.z;
    }
}