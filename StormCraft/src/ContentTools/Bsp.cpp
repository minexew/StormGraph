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

#include <StormGraph/IO/BspGenerator.hpp>
#include <StormGraph/GraphicsDriver.hpp>

//#define Bsp_print_stuff

namespace StormGraph
{
    // assumes that the points are on opposite sides of the plane!
    // Get the point where the line between 2 points crosses the plane
    // Took me some time to figure that out :P
    static Vertex intersect( const Vertex& p1, const Vertex& p2, const Plane& plane )
    {
        const Vector<float> dir = p2.pos - p1.pos;

        float dot1 = dir.dotProduct( plane.normal );
        float dot2 = plane.pointDistance( p1.pos ) - plane.d;

        float t = -( plane.d + dot2 ) / dot1;

        Vertex interpolated;

        interpolated.pos = dir * t + p1.pos;
        interpolated.normal = ( p2.normal - p1.normal ) * t + p1.normal;

        for ( unsigned i = 0; i < TEXTURES_PER_VERTEX; i++ )
            interpolated.uv[i] = ( p2.uv[i] - p1.uv[i] ) * t + p1.uv[i];

        interpolated.lightUv = ( p2.lightUv - p1.lightUv ) * t + p1.lightUv;

        return interpolated;
    }

    static void splitPolygon( const BspPolygon& polygon, const Plane& plane, BspPolygon& front, BspPolygon& back )
    {
        Vertex ptA, ptB;
        double sideA, sideB;

        ptA = polygon.v[polygon.numVertices - 1];
        sideA = plane.pointDistance( ptA.pos );

        // Go through the vertices and add each to proper group(s)
        for ( unsigned i = 0; i < polygon.numVertices; i++ )
        {
            ptB = polygon.v[i];
            sideB = plane.pointDistance( ptB.pos );

            if ( sideB > 0 )            // Point #i-1 is in front of the plane
            {
                if ( sideA < 0 )        // Point #i lies behind the plane - add an intersection point to both sides
                {
                    SG_assert( front.numVertices < BspPolygon::MAX_VERTICES )
                    SG_assert( back.numVertices < BspPolygon::MAX_VERTICES )

                    const Vertex v = intersect( ptB, ptA, plane );

                    front.v[front.numVertices++] = v;
                    back.v[back.numVertices++] = v;
                }

                SG_assert( front.numVertices < BspPolygon::MAX_VERTICES )

                // ...and Point #i-1 to the front side
                front.v[front.numVertices++] = ptB;
            }
            else if ( sideB < 0 )       // Point #i-1 lies behind the plane
            {
                if ( sideA > 0 )        // Point #i lies in front of the plane - add an intersection point to both sides
                {
                    SG_assert( front.numVertices < BspPolygon::MAX_VERTICES )
                    SG_assert( back.numVertices < BspPolygon::MAX_VERTICES )

                    const Vertex v = intersect( ptB, ptA, plane );

                    front.v[front.numVertices++] = v;
                    back.v[back.numVertices++] = v;
                }

                SG_assert( back.numVertices < BspPolygon::MAX_VERTICES )

                // ...and Point #i-1 to the back side
                back.v[back.numVertices++] = ptB;
            }
            else                        // Point #i-1 seems to lie exactly on the plane, add it to both sides
            {
                SG_assert( front.numVertices < BspPolygon::MAX_VERTICES )
                SG_assert( back.numVertices < BspPolygon::MAX_VERTICES )

                front.v[front.numVertices++] = ptB;
                back.v[back.numVertices++] = ptB;
            }

            ptA = ptB;
            sideA = sideB;
        }
    }

    static void getBounds( const BspPolygon* polygons, size_t count, Vector<float> bounds[2] )
    {
        // Nothing really interesting here
        // Maybe except for the fact that this is the calculation which requires every BSP to have at least one polygon
        // (it actually quite makes sense - what are the bounds of... nothing?)

        SG_assert( count > 0 )
        SG_assert( polygons != nullptr )

        bounds[0] = bounds[1] = polygons[0].v[0].pos;

        for ( size_t i = 0; i < count; i++ )
        {
            for ( unsigned j = 0; j < polygons[i].numVertices; j++ )
            {
                for ( int axis = 0; axis < 3; axis++ )
                    if ( polygons[i].v[j].pos.get( axis ) < bounds[0].get( axis ) )
                        bounds[0].set( axis, polygons[i].v[j].pos.get( axis ) );

                for ( int axis = 0; axis < 3; axis++ )
                    if ( polygons[i].v[j].pos.get( axis ) > bounds[1].get( axis ) )
                        bounds[1].set( axis, polygons[i].v[j].pos.get( axis ) );
            }
        }
    }

    BspGenerator::BspGenerator( unsigned nodePolyLimit, const Vector<float>& nodeVolumeLimit )
            : nodePolyLimit( nodePolyLimit ), nodeVolumeLimit( nodeVolumeLimit )
    {
    }

    BspGenerator::~BspGenerator()
    {
    }

    unsigned BspGenerator::breakPoly( const BspPolygon& polygon, List<unsigned>& indices )
    {
        if ( polygon.numVertices < 3 )
            return 0;

        indices.add( getVertexIndex( polygon.materialIndex, polygon.v[0] ) );
        indices.add( getVertexIndex( polygon.materialIndex, polygon.v[1] ) );
        indices.add( getVertexIndex( polygon.materialIndex, polygon.v[polygon.numVertices - 1] ) );

        bool even = false;

        for ( unsigned i = 0; i < polygon.numVertices - 3; i++ )
        {
            if ( !even )
            {
                indices.add( getVertexIndex( polygon.materialIndex, polygon.v[polygon.numVertices - 1 - i / 2] ) );
                indices.add( getVertexIndex( polygon.materialIndex, polygon.v[1 + i / 2] ) );
                indices.add( getVertexIndex( polygon.materialIndex, polygon.v[2 + i / 2] ) );
            }
            else
            {
                indices.add( getVertexIndex( polygon.materialIndex, polygon.v[polygon.numVertices - 1 - i / 2] ) );
                indices.add( getVertexIndex( polygon.materialIndex, polygon.v[2 + i / 2] ) );
                indices.add( getVertexIndex( polygon.materialIndex, polygon.v[polygon.numVertices - 2 - i / 2] ) );
            }

            even = !even;
        }

        return polygon.numVertices - 2;
    }

    /*BspTree* Bsp::generate( BspTri* triangles, unsigned count )
    {
        tree = new BspTree;

        tree->root = partition( triangles, count );
        //printf( "## BSP GENERATION COMPLETE | %u Triangles total\n", totalTriangles );

        iterate ( materials )
            tree->materials.add( ( BspMaterial&& ) materials.current() );

        materials.clear();

        return tree.detach();
    }*/

    BspTree* Bsp::generate( const BspPolygon* polygons, size_t count )
    {
        tree = new BspTree;

        tree->root = partition( polygons, count );
        //printf( "## BSP GENERATION COMPLETE | %u Triangles total\n", totalTriangles );

        iterate ( materials )
            tree->materials.add( ( BspMaterial&& ) materials.current() );

        materials.clear();

        return tree.detach();
    }

    unsigned Bsp::getMaterialIndex( const char* name )
    {
        iterate ( materials )
            if ( materials.current().name == name )
                return materials.iter();

        return materials.add( BspMaterial { name, nullptr } );
    }

    unsigned Bsp::getVertexIndex( unsigned materialIndex, const Vertex& vertex )
    {
        iterate ( tree->vertices[materialIndex] )
        {
            const Vertex& current = tree->vertices[materialIndex].current();

            if ( current.pos.equals( vertex.pos, 0.001f ) && current.normal.equals( vertex.normal, 0.001f )
                    && current.uv[0].equals( vertex.uv[0], 0.001f )
                    && current.uv[1].equals( vertex.uv[1], 0.001f )
                    && current.uv[2].equals( vertex.uv[2], 0.001f )
                    && current.lightUv.equals( vertex.lightUv, 0.001f ) )
                return tree->vertices[materialIndex].iter();
        }

        return tree->vertices[materialIndex].add( vertex );
    }
/*
    BspNode* Bsp::partition( BspTri* triangles, const unsigned count )
    {
        Object<BspNode> node = new BspNode;
        getBounds( triangles, count, node->bounds );

#ifdef Bsp_print_stuff
        printf( "BOUNDS: [%s] to [%s]\n", node->bounds[0].toString().c_str(), node->bounds[1].toString().c_str() );
#endif

        // size of the node boundary
        const Vector<float> boundsSize = node->bounds[1] - node->bounds[0];

        // center of the node volume (in world space!!)
        const Vector<float> boundsCenter = node->bounds[0] + boundsSize / 2;

#ifdef Bsp_print_stuff
        printf( "%u VS %u polys; %s vs %s bounds\n", count, nodePolyLimit, boundsSize.toString().c_str(), nodeVolumeLimit.toString().c_str() );
#endif

        // Could the given polygons form a single acceptable node?
        // Damn those unprecise floating points
        if ( count > nodePolyLimit || boundsSize > nodeVolumeLimit * 1.001f )
        {
            // LOLNO, splitting time
            // Ok, let's do this!

            // Find the "most offending" axis with respect to the shape of the maximal node volume
            const Vector<float> overflow = boundsSize / nodeVolumeLimit;

            int axis;

            if ( overflow.y >= overflow.x && overflow.y >= overflow.z )
                axis = 1;
            else if ( overflow.z >= overflow.x && overflow.z >= overflow.y )
                axis = 2;
            else
                axis = 0;

            // The axis is now determined. Let's find some reasonable splitting plane
#ifdef Bsp_print_stuff
            printf( "Splitting along %i axis...\n", axis );
#endif

            // We'll loop through the possible partitionings until the polygon count ratio
            // between the 2 new possible partitions starts to get worse.

            // With these settings the loop will iterate for 11 times
            // Smaller steps would be most likely unreasonable because big nodes will be split further anyway
            double limit = boundsSize.get( axis ) / 3.0f;
            double step = ( boundsSize.get( axis ) - 2 * limit ) / 10.0f;

            double bestPolygonRatio = 0.0f, bestCoord = 0.0f;
            unsigned bestCounts[2] = { 0, 0 };

            SG_assert( step > 0.01 )

            for ( double c = node->bounds[0].get( axis ) + limit; c <= node->bounds[1].get( axis ) - limit; c += step )
            {
                // Count the polygons on each side of the splitting plane
                //printf( "c = [%g: %g :%g] (+ %g)\n", c, node->bounds[0].get( axis ) + limit, node->bounds[1].get( axis ) - limit, step );

                unsigned counts[2] = { 0, 0 };

                for ( unsigned i = 0; i < count; i++ )
                {
                    int side[2] = { 0, 0 };

                    for ( int j = 0; j < 3; j++ )
                        if ( triangles[i].v[j].pos.get( axis ) < c - 0.001f )
                            side[0] = 1;
                        else if ( triangles[i].v[j].pos.get( axis ) > c + 0.001f )
                            side[1] = 1;

                    if ( !side[0] && !side[1] )
                        side[0] = 1;

                    counts[0] += side[0];
                    counts[1] += side[1];
                }

                // No polygons on one of the sides? Such partitioning would be, of course, meaningless
                if ( counts[0] == 0 || counts[1] == 0 )
                    continue;

                // Calculate the polygon count ratio for this partitioning (always >= 1.0)
                double ratio = ( counts[0] > counts[1] ) ? ( double ) counts[0] / counts[1] : ( double ) counts[1] / counts[0];

                // Only test the polygon ratio if we've already had a successful iteration
                if ( bestCounts[0] != 0 )
                {
                    // The polygon ratio just got worse
                    // And it's not going to get better.
                    if ( ratio > bestPolygonRatio )
                        break;

                    // The polygon ratio is exactly the same? Well, test the volume ratio then
                    // This should happen only for a very low-resolution meshes btw
                    if ( ratio == bestPolygonRatio && fabs( boundsCenter.get( axis ) - c ) > fabs( boundsCenter.get( axis ) - bestCoord ) )
                        break;
                }

                // We've got a candidate now
                bestPolygonRatio = ratio;
                bestCoord = c;

                // Cache these values to allocate a big enough array later
                bestCounts[0] = counts[0];
                bestCounts[1] = counts[1];
            }

            // WHAT THE-
            SG_assert3( bestPolygonRatio >= 1.0f, "StormGraph.Bsp.partition" )

#ifdef Bsp_print_stuff
            printf( "BSP ## Best poly ratio %g (@ %g)\n", bestPolygonRatio, bestCoord );
#endif

            // Pre-allocate the space
            // It won't most likely be enough because of the splits
            // But it's better than starting from 0
            List<BspTri> partitions[2] = { bestCounts[0], bestCounts[1] };

            // This will be useful later...
            const Vector<float> axisMask( axis == 0 ? 1.0f : 0.0f, axis == 1 ? 1.0f : 0.0f, axis == 2 ? 1.0f : 0.0f );

            Plane plane;
            plane.setNormalAndPoint( -axisMask, Vector<float>( axis == 0 ? bestCoord : node->bounds[0].x, axis == 1 ? bestCoord : node->bounds[0].y, axis == 2 ? bestCoord : node->bounds[0].z ) );

            // ...so will be these
            BspTri front[2], back[2];
            unsigned numFront, numBack;

            // The uglier part begins here :)
            for ( unsigned i = 0; i < count; i++ )
            {
                BspTri& tri = triangles[i];
                bool onSide[2] = { false, false };

                // Determine on which side(s) of the splitting plane this polygon is
                // No flag is set if the point lies on the plane
                for ( int j = 0; j < 3; j++ )
                    if ( tri.v[j].pos.get( axis ) < bestCoord - 0.001f )
                        onSide[0] = true;
                    else if ( tri.v[j].pos.get( axis ) > bestCoord + 0.001f )
                        onSide[1] = true;

                // If it's on one side only, we're done here...
                // If all the points lie on the plane, add the polygon to partition 0
                if ( !onSide[1] )
                    partitions[0].add( tri );
                else if ( onSide[1] && !onSide[0] )
                    partitions[1].add( tri );
                else
                {
                    // ...but if it's not we're gonna have some more work ^_^
                    //printf( "WARNING: Triangle [%s;%s;%s] requires division\n", tri.v[0].pos.toString().c_str(), tri.v[1].pos.toString().c_str(), tri.v[2].pos.toString().c_str() );

                    numFront = numBack = 0;

                    // Do all the hard work for us, would ya?
                    splitPolygon( tri, plane, front, back, numFront, numBack );

                    // There should be 1 to 2 triangles on each side (2 to 3 total)
                    // FIXME: We have to reassign the material to them, by the way,
                    // as splitPolygon doesnt bother to keep it
                    for ( unsigned i = 0; i < numFront; i++ )
                    {
                        front[i].materialIndex = tri.materialIndex;
                        partitions[0].add( front[i] );
                    }

                    for ( unsigned i = 0; i < numBack; i++ )
                    {
                        back[i].materialIndex = tri.materialIndex;
                        partitions[1].add( back[i] );
                    }
                }
            }

#ifdef Bsp_print_stuff
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

            // Build the material groups
            for ( size_t i = 0; i < count; i++ )
            {
                size_t group;

                for ( group = 0; group < node->meshes.getLength(); group++ )
                    if ( node->meshes[group]->material == triangles[i].materialIndex )
                        break;

                if ( group >= node->meshes.getLength() )
                    group = node->meshes.add( new BspMesh( triangles[i].materialIndex ) );

                for ( int j = 0; j < 3; j++ )
                    node->meshes[group]->indices.add( getVertexIndex( triangles[i].materialIndex, triangles[i].v[j] ) );

                tree->totalTriangles[triangles[i].materialIndex]++;
            }

            return node.detach();
        }
    }
*/
    BspNode* Bsp::partition( const BspPolygon* polygons, size_t count )
    {
        Object<BspNode> node = new BspNode;
        getBounds( polygons, count, node->bounds );

#ifdef Bsp_print_stuff
        printf( "BOUNDS: [%s] to [%s]\n", node->bounds[0].toString().c_str(), node->bounds[1].toString().c_str() );
#endif

        // size of the node boundary
        const Vector<float> boundsSize = node->bounds[1] - node->bounds[0];

        // center of the node volume (in world space!!)
        const Vector<float> boundsCenter = node->bounds[0] + boundsSize / 2;

#ifdef Bsp_print_stuff
        printf( "%"PRIuPTR" VS %u polys; %s vs %s bounds\n", count, nodePolyLimit, boundsSize.toString().c_str(), nodeVolumeLimit.toString().c_str() );
#endif

        // Could the given polygons form a single acceptable node?
        // Damn those unprecise floating points
        if ( count > nodePolyLimit || boundsSize > nodeVolumeLimit * 1.001f )
        {
            // LOLNO, splitting time
            // Ok, let's do this!

            // Find the "most offending" axis with respect to the shape of the maximal node volume
            const Vector<float> overflow = boundsSize / nodeVolumeLimit;

            int axis;

            if ( overflow.y >= overflow.x && overflow.y >= overflow.z )
                axis = 1;
            else if ( overflow.z >= overflow.x && overflow.z >= overflow.y )
                axis = 2;
            else
                axis = 0;

            // The axis is now determined. Let's find some reasonable splitting plane
#ifdef Bsp_print_stuff
            printf( "Splitting along %i axis...\n", axis );
#endif

            // We'll loop through the possible partitionings until the polygon count ratio
            // of the 2 new partitions starts to get worse.

            // With these settings the loop will iterate 11 times
            // Smaller steps would be most likely unreasonable because big nodes will be split further anyway
            double limit = boundsSize.get( axis ) / 3.0f;
            double step = ( boundsSize.get( axis ) - 2 * limit ) / 10.0f;

            double bestPolygonRatio = 0.0f, bestCoord = 0.0f;
            unsigned bestCounts[2] = { 0, 0 };

            SG_assert( step > 0.01 )

            for ( double c = node->bounds[0].get( axis ) + limit; c <= node->bounds[1].get( axis ) - limit; c += step )
            {
                // Count the polygons on each side of the splitting plane
                //printf( "c = [%g: %g :%g] (+ %g)\n", c, node->bounds[0].get( axis ) + limit, node->bounds[1].get( axis ) - limit, step );

                unsigned counts[2] = { 0, 0 };

                for ( unsigned i = 0; i < count; i++ )
                {
                    int side[2] = { 0, 0 };

                    for ( unsigned j = 0; j < polygons[i].numVertices; j++ )
                        if ( polygons[i].v[j].pos.get( axis ) < c - 0.001f )
                            side[0] = 1;
                        else if ( polygons[i].v[j].pos.get( axis ) > c + 0.001f )
                            side[1] = 1;

                    if ( !side[0] && !side[1] )
                        side[0] = 1;

                    counts[0] += side[0];
                    counts[1] += side[1];
                }

                // No polygons on one of the sides? Such partitioning would be, of course, meaningless
                if ( counts[0] == 0 || counts[1] == 0 )
                    continue;

                // Calculate the polygon count ratio for this partitioning (always >= 1.0)
                double ratio = ( counts[0] > counts[1] ) ? ( double ) counts[0] / counts[1] : ( double ) counts[1] / counts[0];

                // Only test the polygon ratio if we've already had a successful iteration
                if ( bestCounts[0] != 0 )
                {
                    // The polygon ratio just got worse
                    // And it's not going to get better.
                    if ( ratio > bestPolygonRatio )
                        break;

                    // The polygon ratio is exactly the same? Well, test the volume ratio then
                    // (this should only happen for very low resolution meshes)
                    if ( ratio == bestPolygonRatio && fabs( boundsCenter.get( axis ) - c ) > fabs( boundsCenter.get( axis ) - bestCoord ) )
                        break;
                }

                // We've got a new candidate
                bestPolygonRatio = ratio;
                bestCoord = c;

                // Cache these values to allocate a big enough array later
                bestCounts[0] = counts[0];
                bestCounts[1] = counts[1];
            }

            // WHAT THE-
            SG_assert( bestPolygonRatio >= 1.0f )

#ifdef Bsp_print_stuff
            printf( "BSP ## Best poly ratio %g (@ %g)\n", bestPolygonRatio, bestCoord );
#endif

            // Pre-allocate the space
            // It won't most likely be enough because of the splits
            // But it's better than starting from 0, amirite?
            List<BspPolygon> partitions[2] = { bestCounts[0], bestCounts[1] };

            // This is going to be useful later on...
            const Vector<float> axisMask( axis == 0 ? 1.0f : 0.0f, axis == 1 ? 1.0f : 0.0f, axis == 2 ? 1.0f : 0.0f );

            Plane plane;
            plane.setNormalAndPoint( -axisMask, Vector<float>( axis == 0 ? bestCoord : node->bounds[0].x, axis == 1 ? bestCoord : node->bounds[0].y, axis == 2 ? bestCoord : node->bounds[0].z ) );

            // Add the polygons to their respective partitions
            for ( size_t i = 0; i < count; i++ )
            {
                BspPolygon front, back;

                front.numVertices = 0;
                back.numVertices = 0;

                splitPolygon( polygons[i], plane, front, back );

                if ( front.numVertices > 0 )
                {
                    front.materialIndex = polygons[i].materialIndex;
                    partitions[0].add( front );
                }

                if ( back.numVertices > 0 )
                {
                    back.materialIndex = polygons[i].materialIndex;
                    partitions[1].add( back );
                }
            }

#ifdef Bsp_print_stuff
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

            // Build the material groups
            for ( size_t i = 0; i < count; i++ )
            {
                size_t group;

                for ( group = 0; group < node->meshes.getLength(); group++ )
                    if ( node->meshes[group]->material == polygons[i].materialIndex )
                        break;

                if ( group >= node->meshes.getLength() )
                    group = node->meshes.add( new BspMesh( polygons[i].materialIndex ) );

                tree->totalTriangles[polygons[i].materialIndex] += breakPoly( polygons[i], node->meshes[group]->indices );
            }

            return node.detach();
        }
    }

    unsigned Bsp::registerMaterial( const char* name, MaterialStaticProperties* material )
    {
        iterate ( materials )
        {
            const MaterialStaticProperties* properties = materials.current().properties;

            if ( properties->numTextures != material->numTextures )
                continue;

            if ( properties->dynamicLighting != material->dynamicLighting )
                continue;

            if ( properties->lightMapping != material->lightMapping )
                continue;

            if ( properties->castsShadows != material->castsShadows )
                continue;

            if ( properties->receivesShadows != material->receivesShadows )
                continue;

            if ( !properties->colour.equals( material->colour, 0.01f ) )
                continue;

            bool failed = false;

            for ( size_t i = 0; i < properties->numTextures; i++ )
                if ( properties->textureNames[i] != material->textureNames[i] )
                {
                    failed = true;
                    break;
                }

            if ( failed )
                continue;

            if ( properties->dynamicLighting )
            {
                // TODO: ...
            }

            if ( properties->lightMapping )
                if ( properties->lightMapName != material->lightMapName )
                    continue;

            delete material;
            return materials.iter();
        }

        return materials.add( BspMaterial { name, material } );
    }

    void BspWriter::save( BspNode* node, OutputStream* output )
    {
        output->write<Vector<float>>( node->bounds[0] );
        output->write<Vector<float>>( node->bounds[1] );

        output->write<uint32_t>( node->meshes.getLength() );

        iterate ( node->meshes )
        {
            BspMesh* mesh = node->meshes.current();

            output->write<uint32_t>( mesh->indices.getLength() / 3 );
            output->write<uint32_t>( mesh->material );

            if ( indexSize == 2 )
            {
                for each_in_list( mesh->indices, i )
                    output->write<uint16_t>( mesh->indices[i] );
            }
            else
            {
                for each_in_list( mesh->indices, i )
                    output->write<uint32_t>( mesh->indices[i] );
            }
        }

        if ( node->children[0] != nullptr || node->children[1] != nullptr )
        {
            output->write<uint8_t>( 1 );

            save( node->children[0], output );
            save( node->children[1], output );
        }
        else
            output->write<uint8_t>( 0 );
    }

    void BspWriter::save( BspTree* tree, OutputStream* output )
    {
        Reference<> outputGuard( output );

        SG_assert( tree != nullptr )
        SG_assert( output != nullptr )

        output->writeString( "Sg_Bsp#0" );

        indexSize = 2;

        for ( size_t i = 0; i < tree->materials.getLength(); i++ )
            if ( tree->vertices[i].getLength() > 0xFFFF )
                indexSize = 4;

        output->write<uint8_t>( indexSize );

        output->write<uint32_t>( tree->materials.getLength() );

        for ( size_t i = 0; i < tree->materials.getLength(); i++ )
        {
            output->writeString( tree->materials[i].name );

            if ( tree->materials[i].properties != nullptr )
            {
                const MaterialStaticProperties* properties = tree->materials[i].properties;

                output->write<uint8_t>( 1 );

                output->write<uint32_t>( properties->colour.toRgbaUint32() );

                output->write<uint32_t>( properties->numTextures );

                for ( unsigned i = 0; i < properties->numTextures; i++ )
                    output->writeString( properties->textureNames[i] );

                output->write<uint8_t>( properties->dynamicLighting ? 1 : 0 );

                if ( properties->dynamicLighting )
                {
                    output->write<uint32_t>( properties->dynamicLightingResponse.ambient.toRgbaUint32() );
                    output->write<uint32_t>( properties->dynamicLightingResponse.diffuse.toRgbaUint32() );
                    output->write<uint32_t>( properties->dynamicLightingResponse.emissive.toRgbaUint32() );
                    output->write<uint32_t>( properties->dynamicLightingResponse.specular.toRgbaUint32() );
                    output->write<float>( properties->dynamicLightingResponse.shininess );
                }

                output->write<uint8_t>( properties->lightMapping ? 1 : 0 );

                if ( properties->lightMapping )
                    output->writeString( properties->lightMapName );

                output->write<uint8_t>( properties->castsShadows ? 1 : 0 );
                output->write<uint8_t>( properties->receivesShadows ? 1 : 0 );
            }
            else
                output->write<uint8_t>( 0 );

            output->write<uint32_t>( tree->vertices[i].getLength() );
            output->write<uint32_t>( tree->totalTriangles[i] );

            iterate ( tree->vertices[i] )
            {
                const Vertex& vertex = tree->vertices[i].current();

                output->write<Vector<float>>( vertex.pos );
                output->write<Vector<float>>( vertex.normal );
                output->write<Vector2<float>>( vertex.uv[0] );
                output->write<Vector2<float>>( vertex.lightUv );
            }
        }

        save( tree->root, output );
    }
}
