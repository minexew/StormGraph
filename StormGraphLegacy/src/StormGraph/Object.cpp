
#include <StormGraph/StormGraph.hpp>

namespace StormGraph
{
    Object::~Object()
    {
    }

    void Object::move( const Vector<float>& vector, bool relative )
    {
        if ( relative )
            loc += vector;
        else
            loc = vector;
    }
}
