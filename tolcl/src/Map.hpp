
#include "GameClient.hpp"

namespace GameClient
{
    class Map;

    class PickingListener
    {
        public:
            virtual void onPickingMatch( ObjectNode* node ) = 0;
    };

    class WorldObjNode : public ObjectNode
    {
        Vector<float> loc;
        float orientation;
        Model* model;

        // Ordered List
        bool valid;
        float distance;

        // Picking
        PickingListener* pickingListener;
        unsigned pickingId;

        friend class Sector;

        public:
            li_ReferencedClass_override( WorldObjNode )

            WorldObjNode( PickingListener* pickingListener, const String& name, const Vector<float>& loc, float orientation, Model* model );
            ~WorldObjNode();

            const Vector<float>& getLoc();
            void invalidate();
            virtual bool isValid();
            virtual void pick( Picking* picking );
            virtual void render();
            virtual void updateDistance( float dist );
    };

    struct WorldMesh
    {
        unsigned wmid, numRefs;
        float x, y, z;
        Model* model;

        float w, h;
        HeightMap* height;

        void render()
        {
            model->renderTerrain( Vector<float>( x, y, z ), height );
        }
    };

    class Sector
    {
        Map* map;
        unsigned sx, sy;

        List<unsigned> meshIds;
        List<WorldObjNode*> objects;

        public:
            Sector( Map* map, unsigned sx, unsigned sy );
            ~Sector();

            void addWorldObj( const String& name, float x, float y, float z, float o );
            void deleteWorldObj( float x, float y );
            void save();
            void saveAs( const String& fileName );
    };

    class Map
    {
        OrderedListNode* world;
        PickingListener* pickingListener;

        Sector* sectors[5][5];
        int csx, csy;

        List<WorldMesh> terrains;

        friend class Sector;

        public:
            Map( SceneGraph* graph, PickingListener* pickingListener, float xc, float yc );
            ~Map();

            void addReference( unsigned wmid );
            float getHeightAt( float x, float y );
            Sector* getSectorAt( float x, float y );
            OrderedListNode* getWorld();
            WorldMesh* getWorldMesh( unsigned wmid );
            void load( unsigned mx, unsigned my );
            void lock();
            void moveCenter( float xc, float yc );
            void render();
            void shiftDown( int count );
            void shiftRight( int count );
            void removeReference( unsigned wmid );
            void unlock( const Vector<float>& playerLoc );
    };
}
