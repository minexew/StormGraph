
//#include "GameClient.hpp"

#include <StormGraph/Abstract.hpp>
#include <StormGraph/GraphicsDriver.hpp>
#include <StormGraph/SceneGraph.hpp>

namespace TolClient
{
    class Map;
    struct ObjectNode { li::String name; };
//    class OrderedListNode;

    class PickingListener
    {
    public:
        virtual void onPickingMatch( ObjectNode* node ) = 0;
    };

    class WorldObjNode : public ObjectNode
    {
        StormGraph::Vector<float> loc;
        float orientation;
        StormGraph::IModel* model;

        // Ordered List
        bool valid;
        float distance;

        // Picking
        PickingListener* pickingListener;
        unsigned pickingId;

        friend class Sector;

    public:
//        li_ReferencedClass_override( WorldObjNode )

        WorldObjNode( PickingListener* pickingListener, const li::String& name, const StormGraph::Vector<float>& loc, float orientation, StormGraph::IModel* model );
        ~WorldObjNode();

        const StormGraph::Vector<float>& getLoc();
        void invalidate();
        virtual bool isValid();
//        virtual void pick( Picking* picking );
        virtual void render();
        virtual void updateDistance( float dist );
    };

    struct WorldMesh
    {
        unsigned wmid, numRefs;
        float x, y, z;
        StormGraph::IModel* model;

        float w, h;
        StormGraph::IHeightMap* height;

        void render();
    };

    class Sector
    {
        Map* map;
        unsigned sx, sy;

        li::List<unsigned> meshIds;
        li::List<WorldObjNode*> objects;

    public:
        Sector( Map* map, unsigned sx, unsigned sy );
        ~Sector();

        void addWorldObj( const li::String& name, float x, float y, float z, float o );
        void deleteWorldObj( float x, float y );
        void save();
        void saveAs( const li::String& fileName );
    };

    class Map
    {
//        OrderedListNode* world;
        PickingListener* pickingListener;

        Sector* sectors[5][5];
        int csx, csy;

        li::List<WorldMesh> terrains;

        StormGraph::ISceneGraph* graph;

        friend class Sector;

    public:
        Map( StormGraph::ISceneGraph* graph, PickingListener* pickingListener, float xc, float yc );
        ~Map();

        void addReference( unsigned wmid );
        float getHeightAt( float x, float y );
        Sector* getSectorAt( float x, float y );
//        OrderedListNode* getWorld();
        StormGraph::ISceneGraph* getSceneGraph() { return graph; }
        WorldMesh* getWorldMesh( unsigned wmid );
        void load( unsigned mx, unsigned my );
        void lock();
        void moveCenter( float xc, float yc );
        void render();
        void shiftDown( int count );
        void shiftRight( int count );
        void removeReference( unsigned wmid );
        void unlock( const StormGraph::Vector<float>& playerLoc );
    };
}
