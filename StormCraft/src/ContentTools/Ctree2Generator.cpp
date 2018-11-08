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

#include <StormGraph/IO/Ctree2Generator.hpp>

#define Ct2_print_stuff

#define FLOAT_ERR   0.001f

namespace StormGraph
{
    class Ctree2GeneratorImpl : public Ctree2Generator
    {
        protected:
            unsigned nodeSegLimit;

            void getBounds( const Ct2Line* lines, size_t count, Vector2<> bounds[2] );
            Ct2Node* partition( const Ct2Line* lines, size_t count );

        public:
            Ctree2GeneratorImpl( unsigned nodeSegLimit )
                    : nodeSegLimit( nodeSegLimit )
            {
            }

            virtual Ct2Node* generate( const Ct2Line* lines, size_t count ) override;
    };

    class Ctree2WriterImpl : public Ctree2Writer
    {
        void writeNode( const Ct2Node* node, OutputStream* output );

        public:
            virtual void write( const Ct2Node* tree, OutputStream* output ) override;
    };

    /*static bool intersect( const Ct2Line& seg1, const Ct2Line& seg2, Vector2<>& pt )
    {
        const Vector2<> bb1 = seg1.b - seg1.a;
        const Vector2<> bb2 = seg2.b - seg2.a;

        float d = bb1.x * bb2.y - bb1.y * bb2.x;

        if ( fabs( d ) < FLOAT_ERR )
            return false;

        const Vector2<> w = seg1.a - seg2.a;

        float s = bb1.x * w.y - bb1.y * w.x;

        if ( s < 0 || s > d )
            return false;

        float t = bb2.x * w.y - bb2.y * w.x;

        if ( t < 0 || t > d )
            return false;

        pt = seg1.a + s / d * bb1;
        return true;
    }*/

    Ctree2Generator* Ctree2Generator::create( unsigned nodeSegLimit )
    {
        return new Ctree2GeneratorImpl( nodeSegLimit );
    }

    Ct2Node* Ctree2GeneratorImpl::generate( const Ct2Line* lines, size_t count )
    {
        for ( size_t i = 0; i < count; i++ )
        {
            if ( ( lines[i].a - lines[i].b ).getLength() < FLOAT_ERR )
                throw Exception( "StormGraph.Ctree2GeneratorImpl.generate", "InvalidLineSegment", "Collision line segment with zero or near-zero length" );
        }

        return partition( lines, count );
    }

    void Ctree2GeneratorImpl::getBounds( const Ct2Line* lines, size_t count, Vector2<> bounds[2] )
    {
        SG_assert( count > 0 )
        SG_assert( lines != nullptr )

        bounds[0] = bounds[1] = lines[0].a;

        // Iterate through every collision line

        for ( size_t i = 0; i < count; i++ )
        {
            // a vs minbound
            for ( int axis = 0; axis < 2; axis++ )
                if ( lines[i].a[axis] < bounds[0][axis] )
                    bounds[0][axis] = lines[i].a[axis];

            // a vs maxbound
            for ( int axis = 0; axis < 2; axis++ )
                if ( lines[i].a[axis] > bounds[1][axis] )
                    bounds[1][axis] = lines[i].a[axis];

            // b vs minbound
            for ( int axis = 0; axis < 2; axis++ )
                if ( lines[i].b[axis] < bounds[0][axis] )
                    bounds[0][axis] = lines[i].b[axis];

            // b vs maxbound
            for ( int axis = 0; axis < 2; axis++ )
                if ( lines[i].b[axis] > bounds[1][axis] )
                    bounds[1][axis] = lines[i].b[axis];
        }
    }

    Ct2Node* Ctree2GeneratorImpl::partition( const Ct2Line* lines, size_t count )
    {
        Object<Ct2Node> node = new Ct2Node;
        getBounds( lines, count, node->bounds );

#ifdef Ct2_print_stuff
        printf( "BOUNDS: [%s] to [%s]\n", node->bounds[0].toString().c_str(), node->bounds[1].toString().c_str() );
#endif

        // size of the node boundary
        const Vector2<float> boundsSize = node->bounds[1] - node->bounds[0];

        // center of the node volume (in world space!!)
        //const Vector<float> boundsCenter = node->bounds[0] + boundsSize / 2;

#ifdef Ct2_print_stuff
        printf( "Ct2gen:\t%" PRIuPTR " VS %u lines\n", count, nodeSegLimit );
#endif

        if ( count > nodeSegLimit )
        {
            // Guess along which axis should we do the split
            int axis;

            if ( boundsSize.y >= boundsSize.x )
                axis = 1;
            else
                axis = 0;

#ifdef Ct2_print_stuff
            printf( "Ct2gen:\tSplitting along %i axis...\n", axis );
#endif

            // We'll loop through the possible partitionings until the polygon count ratio
            // of the 2 new partitions starts to get worse.

            // With these settings the loop will iterate 21 times

            double limit = boundsSize[axis] / 4.0f;
            double step = ( boundsSize[axis] - 2 * limit ) / 20.0f;

            double bestPolygonRatio = 0.0f, bestCoord = 0.0f;
            unsigned bestCounts[2] = { 0, 0 };

            double center = node->bounds[0][axis] + boundsSize[axis] / 2.0f;

            SG_assert( step > FLOAT_ERR )

            for ( double c = node->bounds[0][axis] + limit; c <= node->bounds[1][axis] - limit; c += step )
            {
                // Count the lines on each side of the splitting plane
                //printf( "c = [%g: %g :%g] (+ %g)\n", c, node->bounds[0][axis] + limit, node->bounds[1][axis] - limit, step );

                size_t counts[2] = { 0, 0 };

                for ( size_t i = 0; i < count; i++ )
                {
                    int side[2] = { 0, 0 };

                    if ( lines[i].a[axis] < c - FLOAT_ERR )
                        side[0] = 1;
                    else if ( lines[i].a[axis] > c + FLOAT_ERR )
                        side[1] = 1;

                    if ( lines[i].b[axis] < c - FLOAT_ERR )
                        side[0] = 1;
                    else if ( lines[i].b[axis] > c + FLOAT_ERR )
                        side[1] = 1;

                    if ( !side[0] && !side[1] )
                        side[0] = 1;

                    counts[0] += side[0];
                    counts[1] += side[1];
                }

                // No lines on one of the sides? Such partitioning would be, of course, meaningless
                if ( counts[0] == 0 || counts[1] == 0 )
                    continue;

                // Calculate the polygon count ratio for this partitioning (always >= 1.0)
                //double ratio = ( counts[0] > counts[1] ) ? ( double ) counts[0] / counts[1] : ( double ) counts[1] / counts[0];

                double ratio = ( double ) ( counts[0] + counts[1] ) / count;
                //ratio *= ( counts[0] > counts[1] ) ? ( double ) counts[0] / counts[1] : ( double ) counts[1] / counts[0];

                if ( ( bestCounts[0] == 0 || ratio < bestPolygonRatio )
                        || ( ratio < bestPolygonRatio + FLOAT_ERR && fabs( center - c ) < fabs( center - bestCoord ) ) )
                {
                    // We've got a new candidate
                    bestPolygonRatio = ratio;
                    bestCoord = c;

                    // Cache these values to allocate a big enough array later
                    bestCounts[0] = counts[0];
                    bestCounts[1] = counts[1];
                }
            }

            // WHAT THE-
            SG_assert( bestPolygonRatio >= 1.0f )

#ifdef Ct2_print_stuff
            printf( "Ct2gen:\tBest poly ratio %g (@ [%i] = %g)\n", bestPolygonRatio, axis, bestCoord );
#endif

            // Pre-allocate the space
            List<Ct2Line> partitions[2] = { bestCounts[0], bestCounts[1] };

            // Add the polygons to their respective partitions
            for ( size_t i = 0; i < count; i++ )
            {
                Vector2<> intersectPoint;

                bool isAOnLeft = lines[i].a[axis] < bestCoord;
                bool isBOnLeft = lines[i].b[axis] < bestCoord;

                if ( isAOnLeft && isBOnLeft )
                    partitions[0].add( lines[i] );
                else if ( !isAOnLeft && !isBOnLeft )
                    partitions[1].add( lines[i] );
                else
                {
                    const Vector2<> lineBox = lines[i].b - lines[i].a;

                    float t = ( bestCoord - lines[i].a[axis] ) / lineBox[axis];

                    const Vector2<> intersectPoint = Vector2<>( axis == 0 ? bestCoord : ( lines[i].a.x + t * lineBox.x ), axis == 1 ? bestCoord : ( lines[i].a.y + t * lineBox.y ) );

                    printf( "[%s] .. [%s] .. [%s]\n\t([%i] = %g)\n", lines[i].a.toString().c_str(), intersectPoint.toString().c_str(), lines[i].b.toString().c_str(), axis, bestCoord );
                    //getchar();

                    if ( isAOnLeft )
                    {
                        partitions[0].add( Ct2Line { lines[i].a, intersectPoint } );
                        partitions[1].add( Ct2Line { intersectPoint, lines[i].b } );
                    }
                    else
                    {
                        partitions[0].add( Ct2Line { lines[i].b, intersectPoint } );
                        partitions[1].add( Ct2Line { intersectPoint, lines[i].a } );
                    }
                }
            }

#ifdef Ct2_print_stuff
            printf( "Left child: %" PRIuPTR " tris\n", partitions[0].getLength() );
            printf( "Right child: %" PRIuPTR " tris\n", partitions[1].getLength() );
#endif

            // but 2 children
            node->children[0] = partition( partitions[0].getPtr(), partitions[0].getLength() );
            node->children[1] = partition( partitions[1].getPtr(), partitions[1].getLength() );

            return node.detach();
        }
        else
        {
            // Yeah, no prob

            node->lines.load( lines, count );

            return node.detach();
        }
    }

    Ctree2Writer* Ctree2Writer::create()
    {
        return new Ctree2WriterImpl();
    }

    void Ctree2WriterImpl::writeNode( const Ct2Node* node, OutputStream* output )
    {
        SG_assert( output->writeItems<Vector2<float>>( node->bounds, 2 ) == 2 )

        SG_assert( output->write<uint32_t>( node->lines.getLength() ) )

        iterate2 ( i, node->lines )
        {
            SG_assert( output->write<Vector2<>>( (*i).a ) )
            SG_assert( output->write<Vector2<>>( (*i).b ) )
        }

        if ( node->children[0] != nullptr || node->children[1] != nullptr )
        {
            SG_assert( output->write<uint8_t>( 1 ) )

            writeNode( node->children[0], output );
            writeNode( node->children[1], output );
        }
        else
        {
            SG_assert( output->write<uint8_t>( 0 ) )
        }
    }

    void Ctree2WriterImpl::write( const Ct2Node* tree, OutputStream* output )
    {
        Reference<> outputGuard( output );

        SG_assert( tree != nullptr )
        SG_assert( output != nullptr )

        writeNode( tree, output );
    }
}
