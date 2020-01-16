
#include "Map.hpp"
#include "TolClient.hpp"

#include <StormGraph/HeightMap.hpp>
#include <StormGraph/Image.hpp>
#include <StormGraph/ResourceManager.hpp>

#include <littl/File.hpp>

// todo: releasing non-terrain meshes ??

namespace TolClient
{
    extern IResourceManager* globalResMgr;

    WorldObjNode::WorldObjNode( PickingListener* pickingListener, const li::String& name, const StormGraph::Vector<float>& loc, float orientation, StormGraph::IModel* model )
            : ObjectNode{name}, loc( loc ), orientation( orientation ), model( model ), valid( true ), pickingListener( pickingListener )
    {
    }

    WorldObjNode::~WorldObjNode()
    {
        printf( "destroying WorldObjNode\n" );
        model->release();
    }

    const StormGraph::Vector<float>& WorldObjNode::getLoc()
    {
        return loc;
    }

    void WorldObjNode::invalidate()
    {
//        release();
        valid = false;
    }

    bool WorldObjNode::isValid()
    {
        return valid;
    }

//    void WorldObjNode::pick( Picking* picking )
//    {
//        if ( !sg->isPointVisible( loc ) )
//            return;
//
//        model->renderBegin();
//        model->translate( loc );
//        model->rotate( orientation, Vector<float>( 0.0f, 0.0f, 1.0f ) );
//        pickingId = model->pickingRender( picking );
//        model->renderEnd();
//    }

    void WorldObjNode::render()
    {
//        if ( pickingId != 0 && pickingId == picking->getId() )
//            pickingListener->onPickingMatch( this );
//
//        pickingId = 0;

//        if ( !gr->isPointVisible( loc ) )
//            return;

        Transform transforms[] = {
            {Transform::translate, loc},
            {Transform::rotate,    Vector<float>(0.0f, 0.0f, 1.0f), orientation},
        };
        model->render(transforms, 2);
    }

    void WorldObjNode::updateDistance( float dist )
    {
        distance = dist;
    }

    Sector::Sector( Map* map, unsigned sx, unsigned sy )
            : map( map ), sx( sx ), sy( sy )
    {
        li::String fileName = ( li::String )"tolcl/area/" + sx + "+" + sy + ".sect";

        InputStream* sectorFile = sg->getFileSystem()->openInput( fileName );

        while ( sectorFile && sectorFile->isReadable() )
        {
            unsigned entType = sectorFile->read<uint8_t>();

            if ( entType == 0 )
                break;
            else if ( entType == 1 )
            {
                // WorldMesh, v1

                unsigned wmid = sectorFile->read<uint16_t>();

                meshIds.add( wmid );
                map->addReference( wmid );
            }
            else if ( entType == 2 )
            {
                // WorldObj, v1

                String name = sectorFile->readString();
                float x = sectorFile->read<float>();
                float y = sectorFile->read<float>();
                float o = sectorFile->read<float>();

                addWorldObj( name, x + sx * 200.0f, y + sy * 200.0f, 0.0, o );
            }
            else
                printf( "ERROR Sector::Sector() : unknown ent-type %02X\n", entType );
        }
    }

    Sector::~Sector()
    {
        iterate ( meshIds )
            map->removeReference( meshIds.current() );

        iterate ( objects )
            objects.current()->invalidate();
    }

    void Sector::addWorldObj( const String& name, float x, float y, float z, float o )
    {
        InputStream* objectInfo = sg->getFileSystem()->openInput( ( String )"tolcl/world/" + name + ".obj" );

        if ( !objectInfo || !objectInfo->isReadable() )
        {
            printf( "ERROR Sector::addWorldObj() : load WorldObj (%s) failed.\n", name.c_str() );
            return;
        }

        objectInfo->readLine();

        String type = objectInfo->readLine();

        if ( type == "light" )
        {
        }
        else if ( type == "model" )
        {
            String source = objectInfo->readLine();

            StormGraph::IModel* model = globalResMgr->getModel( source );
            z = map->getHeightAt( x, y );

//            WorldObjNode* node = new WorldObjNode( map->pickingListener, name, Vector<float>( x, y, z ), o, model );
//            map->getWorld()->add( node->reference() );
//            objects.add( node );

            map->getSceneGraph()->addModel(model, {x, y, z}, {});
        }
    }

    void Sector::deleteWorldObj( float x, float y )
    {
        reverse_iterate ( objects )
        {
            printf( "dist: %g\n", objects.current()->distance );

            if ( objects.current()->distance < 1.0f )
            {
                objects.current()->invalidate();
                objects.remove( objects.iter() );
                return;
            }
        }
    }

    void Sector::save()
    {
        auto f = File::open( ( String )"tolcl/area/" + sx + "+" + sy + ".sect", true, true );
        auto& output = *f;

        iterate ( meshIds )
        {
            WorldMesh* mesh = map->getWorldMesh( meshIds.current() );

            if ( mesh )
            {
                output.write<uint8_t>( 1 );
                output.write<uint16_t>( mesh->wmid );
                output.write<float>( mesh->x );
                output.write<float>( mesh->y );
            }
        }

        /*iterate ( objects )
        {
            WorldObj& obj = objects.current();

            output.write<uint8_t>( 2 );
            output.writeString( obj.name );
            output.write<float>( obj.x - sx * 200.0f );
            output.write<float>( obj.y - sy * 200.0f );
            output.write<float>( obj.orientation );
        }*/
    }

    void Sector::saveAs( const String& fileName )
    {
        auto f = File::open( fileName, true, true );
        auto& output = *f;

        iterate ( meshIds )
        {
            WorldMesh* mesh = map->getWorldMesh( meshIds.current() );

            if ( mesh )
                output.writeLine( ( String )"WorldMesh wmid:" + mesh->wmid + " x:" + mesh->x + " y:" + mesh->y );
        }

        /*iterate ( objects )
        {
            WorldObj& obj = objects.current();

            output.writeLine( "WorldObj name:" + obj.name + " x:" + obj.x + " y:" + obj.y + " o:" + obj.orientation );
        }*/
    }

    Map::Map( ISceneGraph* graph, PickingListener* pickingListener, float xc, float yc )
            :  graph( graph ), pickingListener( pickingListener ), csx( -1 ), csy( -1 )
    {
//        world = new OrderedListNode( "world_objs" );
//        graph->add( world );

        memset( sectors, 0, sizeof( sectors ) );

        csx = ( unsigned )floor( xc / 200.f );
        csy = ( unsigned )floor( yc / 200.f );

        for ( unsigned x = 1; x < 4; x++ )
            for ( unsigned y = 1; y < 4; y++ )
                load( x, y );

        // TODO: remove one time
        //world->enter();

        // Start the world sorter
        // Will block for some time as a call to map->lock() occurs from GameScene::render() shortly after calling this constructor
        // Whatever.
//        world->start();
    }

    Map::~Map()
    {
        iterate ( terrains )
        {
            delete terrains.current().height;
            terrains.current().model->release();
        }
    }

    void Map::addReference( unsigned wmid )
    {
        printf( "Map::addReference( %u )\n", wmid );

        iterate ( terrains )
            if ( terrains.current().wmid == wmid )
            {
                printf( " -- is a known terrain -> ignoring all other params\n" );
                terrains.current().numRefs++;
                return;
            }

        printf( " -- not found, loading %s\n", ( ( String )"tolcl/world/" + wmid + ".mesh" ).c_str() );

        InputStream* meshInfo = sg->getFileSystem()->openInput( ( String )"tolcl/world/" + wmid + ".mesh" );

        if ( !meshInfo || !meshInfo->isReadable() )
        {
            printf( "ERROR Map::addReference() : load WorldMesh (#%u) failed.\n", wmid );
            return;
        }

        meshInfo->readLine();

        String type = meshInfo->readLine();

        if ( type == "heightmap" )
        {
            String source = meshInfo->readLine();
            String texture = meshInfo->readLine();

            float x = meshInfo->readLine();
            float y = meshInfo->readLine();
            float z0 = meshInfo->readLine();
            float w = meshInfo->readLine();
            float h = meshInfo->readLine();
            float range = meshInfo->readLine();

            auto stream = sg->getFileSystem()->openInput(source.c_str());
            auto heightMap = sg->getImageLoader()->load(stream, true);
            auto tex = globalResMgr->getTexture(texture.c_str());

            IHeightMap* height = sg->createHeightMap( heightMap->size.getXy() );

            for (int y = 0; y < heightMap->size.y; y++) {
                for (int x = 0; x < heightMap->size.x; x++) {
                    auto h = heightMap->data[((y * heightMap->size.x) + x) * 3] / 255.0f;
                    height->set(x, y, z0 + h * range);
                }
            }

            auto mat = sg->getGraphicsDriver()->createSolidMaterial( "mat", Colour::white(), tex );

            TerrainCreationInfo tci {height, Vector<>(w, h, 1.0f), Vector<>(0.0f, 0.0f, 0.0f), height->getResolution(),
                                     Vector2<>(), Vector2<float>( w, h ) / 5.0f, true, true, false, mat};
            IModel* model = sg->getGraphicsDriver()->createTerrain( "terrain", &tci, 0 );

            WorldMesh terrain = { wmid, 1, x, y, 0.0f, model, w, h, height };
            terrains.add( terrain );
        }
    }

    float Map::getHeightAt( float x, float y )
    {
        iterate ( terrains )
        {
            const WorldMesh& curr = terrains.current();

            if ( x >= curr.x && y >= curr.y && x < curr.x + curr.w && y < curr.y + curr.h )
                return curr.height->get( {(x - curr.x) / curr.w, (y - curr.y) / curr.h} );
        }

        return 0.0f;
    }

    Sector* Map::getSectorAt( float x, float y )
    {
        int mx = ( unsigned )floor( x / 200.f ) - csx + 2;
        int my = ( unsigned )floor( y / 200.f ) - csy + 2;

        printf( "Spawning in local sector %i, %i\n", mx, my );

        if ( mx > 0 && my > 0 && mx < 5 && my < 5 )
            return sectors[mx][my];
        else
            return 0;
    }

//    OrderedListNode* Map::getWorld()
//    {
//        return world;
//    }

    WorldMesh* Map::getWorldMesh( unsigned wmid )
    {
        iterate ( terrains )
            if ( terrains.current().wmid == wmid )
                return &terrains.current();

        return 0;
    }

    void Map::load( unsigned mx, unsigned my )
    {
        SG_assert( mx < 5 )
                SG_assert( my < 5 )

        if ( sectors[mx][my] )
        {
            delete sectors[mx][my];
            sectors[mx][my] = 0;
        }

        // cs* + m* - 2 (world sector coords) must be > 0
        if ( csx + mx >= 2 && csy + my >= 2 )
        {
            printf( "Loading sector (%u %u) at [%u %u]\n", csx - 2 + mx, csy - 2 + my, mx, my );
            sectors[mx][my] = new Sector( this, csx - 2 + mx, csy - 2 + my );
        }
    }

    void Map::lock()
    {
//        world->enter();
    }

    void Map::moveCenter( float xc, float yc )
    {
        bool sectChange = false;

        int newCsx = ( unsigned )floor( xc / 200.0f );
        int newCsy = ( unsigned )floor( yc / 200.0f );

        if ( csx != newCsx )
        {
            shiftRight( csx - newCsx );
            csx = newCsx;
            sectChange = true;
        }

        if ( csy != newCsy )
        {
            shiftDown( csy - newCsy );
            csy = newCsy;
            sectChange = true;
        }

        if ( sectChange )
        {
            for ( unsigned x = 1; x < 4; x++ )
                for ( unsigned y = 1; y < 4; y++ )
                    if ( !sectors[x][y] )
                        load( x, y );
        }
    }

    void Map::removeReference( unsigned wmid )
    {
        iterate ( terrains )
            if ( terrains.current().wmid == wmid )
            {
                if ( --terrains.current().numRefs == 0 )
                {
                    terrains.current().model->release();
                    delete terrains.current().height;
                    terrains.remove( terrains.iter() );
                }

                return;
            }
    }

    void Map::render()
    {
        iterate ( terrains )
            terrains.current().render();
    }

    void Map::shiftDown( int count )
    {
        while ( count > 0 )
        {
            for ( unsigned x = 0; x < 5; x++ )
            {
                for ( unsigned y = 4; y >= 1; y-- )
                    sectors[x][y] = sectors[x][y - 1];

                sectors[x][0] = 0;
            }

            count--;
        }

        while ( count < 0 )
        {
            for ( unsigned x = 0; x < 5; x++ )
            {
                for ( unsigned y = 1; y < 5; y++ )
                    sectors[x][y - 1] = sectors[x][y];

                sectors[x][4] = 0;
            }

            count++;
        }
    }

    void Map::shiftRight( int count )
    {
        while ( count > 0 )
        {
            for ( unsigned y = 0; y < 5; y++ )
            {
                for ( unsigned x = 4; x >= 1; x-- )
                    sectors[x][y] = sectors[x - 1][y];

                sectors[0][y] = 0;
            }

            count--;
        }

        while ( count < 0 )
        {
            for ( unsigned y = 0; y < 5; y++ )
            {
                for ( unsigned x = 1; x < 5; x++ )
                    sectors[x - 1][y] = sectors[x][y];

                sectors[4][y] = 0;
            }

            count++;
        }
    }

    void Map::unlock( const StormGraph::Vector<float>& playerLoc )
    {
//        world->leave();
//        world->reset();
//        world->setCentralPoint( playerLoc );
    }

    void WorldMesh::render()
    {
        Transform pos { Transform::translate, {x, y, z} };
        model->render(&pos, 1);
    }
}
