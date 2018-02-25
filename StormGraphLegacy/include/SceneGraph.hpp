#pragma once

#include <StormGraph/StormGraph.hpp>

namespace StormGraph
{
    class SceneGraph;

    class Node
    {
        protected:
            String name;

            Node( const char* name ) : name( name )
            {
            }

        public:
            virtual ~Node() = 0;

            const String& getName() const { return name; }
    };

    class RenderableNode : public Node
    {
        public:
            RenderableNode( const char* name );
            virtual ~RenderableNode();

            virtual void pick( Picking* picking ) = 0;
            virtual void render() = 0;
    };

    class ObjectNode : public ReferencedClass, public RenderableNode
    {
        public:
            ObjectNode( const char* name );
            virtual ~ObjectNode();

            virtual const Vector<float>& getLoc() = 0;
            virtual bool isValid() = 0;
            virtual void updateDistance( float dist ) = 0;
    };

    class OrderedListNode : public RenderableNode, public Thread, public Mutex
    {
        struct Entry
        {
            ObjectNode* node;
            float distance;
        };

        List<Entry> objects[2];
        int current;

        Vector<float> centralPoint;

        Mutex* counter;

        public:
            OrderedListNode( const char* name );
            virtual ~OrderedListNode();

            void add( ObjectNode* node );
            virtual void pick( Picking* picking );
            virtual void render();
            void reset();
            virtual void run();
            void setCentralPoint( const Vector<float>& loc );

            String getTable();
    };

    class SceneGraph
    {
        List<RenderableNode*> nodes;

        public:
            SceneGraph();
            virtual ~SceneGraph();

            void add( RenderableNode* node );
            Node* getNode( const char* name );
            void pick( Picking* picking );
            void render();
    };
}
