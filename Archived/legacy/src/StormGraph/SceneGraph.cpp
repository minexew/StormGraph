
#include <StormGraph/StormGraph.hpp>

namespace StormGraph
{
    Node::~Node()
    {
    }

    RenderableNode::RenderableNode( const char* name )
            : Node( name )
    {
    }

    RenderableNode::~RenderableNode()
    {
    }

    ObjectNode::ObjectNode( const char* name )
            : RenderableNode( name )
    {
    }

    ObjectNode::~ObjectNode()
    {
    }

    OrderedListNode::OrderedListNode( const char* name )
            : RenderableNode( name ), current( 0 )
    {
        counter = new Mutex();
    }

    OrderedListNode::~OrderedListNode()
    {
        printf( "OrderedListNode cleaning up\n" );

        iterate ( objects[current] )
            objects[current].current().node->release();

        delete counter;
    }

    void OrderedListNode::add( ObjectNode* node )
    {
        Entry entry = { node, 0.0f };
        objects[current].add( entry );
    }

    String OrderedListNode::getTable()
    {
        String table = getName();

        iterate ( objects[current] )
            table += __li_lineEnd " |- " + objects[current].current().node->getName();

        return table;
    }

    void OrderedListNode::pick( Picking* picking )
    {
        /*if ( maxDist < 0.0f )
        {*/
            iterate ( objects[current] )
                objects[current].current().node->pick( picking );
        /*}
        else
        {
            iterate ( objects[current] )
                if ( objects[current].current().distance <= maxDist )
                    objects[current].current().node->pick( picking );
        }*/
    }

    void OrderedListNode::render()
    {
        iterate ( objects[current] )
            //if ( objects[current].iter() < 1 )
                objects[current].current().node->render();
    }

    void OrderedListNode::reset()
    {
        counter->leave();
    }

    void OrderedListNode::run()
    {
        while ( !shouldEnd )
        {
            counter->enter();
            enter();

            /*LARGE_INTEGER freq, begin, end;
            QueryPerformanceFrequency( &freq );
            QueryPerformanceCounter( &begin );*/

            List<Entry>& src = objects[current];
            List<Entry>& dest = objects[1 - current];

            dest.clear();

            iterate ( src )
            {
                if ( src.current().node->isValid() )
                {
                    float distance = ( src.current().node->getLoc() - centralPoint ).getLength();

                    unsigned i;

                    for ( i = 0; i < dest.getLength(); i++ )
                    {
                        if ( distance < dest[i].distance )
                            break;
                    }

                    i = dest.insert( src.current(), i );
                    dest[i].distance = distance;
                    dest[i].node->updateDistance( distance );
                }
                else
                    src.current().node->release();
            }

            current = 1 - current;

            /*QueryPerformanceCounter( &end );
            double us = ( end.QuadPart - begin.QuadPart ) * 1000000.0 / freq.QuadPart;

            if ( us > 10 )
                printf( "Reordered in %g us.\n", us );*/

            leave();
        }
    }

    void OrderedListNode::setCentralPoint( const Vector<float>& loc )
    {
        centralPoint = loc;
    }

    SceneGraph::SceneGraph()
    {
    }

    SceneGraph::~SceneGraph()
    {
        printf( "SceneGraph cleaning up\n" );
        iterate ( nodes )
            delete nodes.current();
    }

    void SceneGraph::add( RenderableNode* node )
    {
        nodes.add( node );
    }

    Node* SceneGraph::getNode( const char* name )
    {
        iterate ( nodes )
            if ( nodes.current()->getName() == name )
                return nodes.current();

        return 0;
    }

    void SceneGraph::pick( Picking* picking )
    {
        iterate ( nodes )
            nodes.current()->pick( picking );
    }

    void SceneGraph::render()
    {
        iterate ( nodes )
            nodes.current()->render();
    }
}
