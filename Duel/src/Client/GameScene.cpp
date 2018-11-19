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

#include "GameScene.hpp"

#include <StormGraph/Engine.hpp>
#include <StormGraph/ResourceManager.hpp>

namespace Duel
{
    static unsigned leavesTested;

    static bool intersects( const Vector2<>& center, float r, const Vector2<>& min, const Vector2<>& max )
    {
        // http://stackoverflow.com/questions/401847/circle-rectangle-collision-detection-intersection

        const Vector2<> halfRect = ( max - min ) / 2.0f;
        const Vector2<> circleDistance = ( min + halfRect - center ).fabs();

        if ( circleDistance.x > halfRect.x + r )
            return false;

        if ( circleDistance.y > halfRect.y + r )
            return false;

        if ( circleDistance.x <= halfRect.x )
            return true;

        if ( circleDistance.y <= halfRect.y )
            return true;

        return pow( ( circleDistance.x - halfRect.x ), 2 ) + pow( ( circleDistance.y - halfRect.y ), 2 ) <= pow( r, 2 );
    }

    GameScene::GameScene( IEngine* sg, Map* map, IGameServer* server, TcpSocket* socket )
            : sg( sg ), playerSpawned( false ), player( nullptr ), map( map ), server( server ), socket( socket )
    {
        graphicsDriver = sg->getGraphicsDriver();
    }

    void GameScene::init()
    {
        drawCollisions = false;
        noclip = false;

        sg->setVariable( "drawCollisions", sg->createBoolRefVariable( drawCollisions ), true );
        sg->setVariable( "noclip", sg->createBoolRefVariable( noclip ), true );

        resMgr = sg->createResourceManager( "resMgr", true );
        resMgr->setLoadFlag( LoadFlag::useShadowMapping, true );

        IGuiDriver* guiDriver = sg->getGuiDriver();

        viewport = graphicsDriver->getViewportSize();

        gui = guiDriver->createGui( Vector<>(), viewport );
        console = sg->createCommandLine( gui );

        ScreenRect sr = { Vector2<uint16_t>( 16, viewport.y / 2 ), Vector2<uint16_t>( viewport.x / 2, viewport.y / 4 ) };
        log = sg->createOnScreenLog( sr );
        log->addLine( "Hello World!" );

        keyScanner = sg->createKeyScanner();
        up =        keyScanner->registerKey( "W" );
        down =      keyScanner->registerKey( "S" );
        left =      keyScanner->registerKey( "A" );
        right =     keyScanner->registerKey( "D" );

        Reference<IModel> model = resMgr->getModel( "Duel/Models/player.ms3d" );

        // scene graph
        sceneGraph = map->resMgr->loadSceneGraph( "_SCENEGRAPH", true );
        renderQueue = graphicsDriver->createRenderQueue();

        world = sceneGraph->addStaticModel( map->bsp.detach()->finalize() );

        ctree2 = map->resMgr->loadCtree2( "_Ctree2", true );

        // gameserver
        if ( server == nullptr && socket == nullptr )
            spawnEntity( true, 0, "", Vector<>() );

        if ( server != nullptr )
            playerEnt = server->addLocalPlayer( sg->getVariableValue( "playername", true ) );
    }

    GameScene::~GameScene()
    {
    }

    void GameScene::collideNode( const Ct2Node* node, Movement& movement )
    {
        if ( !node->lines.isEmpty() )
            leavesTested++;

        iterate2 ( i, node->lines )
        {
            const Ct2Line& line = i;

            const Vector2<> posProj = ( movement.newPos - line.a )  * line.normalize;

            float t = line.dir.dot( posProj );

            if ( t < 0.0f )
                t = 0.0f;
            else if ( t > 1.0f )
                t = 1.0f;

            if ( ( line.a + line.length * t - movement.newPos ).getLength() < movement.r )
            {
                movement.vec = line.dir * movement.vec.dot( line.dir );
                movement.recalc();
            }
        }

        for ( int i = 0; i < 2; i++ )
        {
            if ( node->children[i] != nullptr && intersects( movement.newPos, movement.r, node->children[i]->bounds[0], node->children[i]->bounds[1] ) )
                collideNode( node->children[i], movement );
        }
    }

    void GameScene::onFrameBegin()
    {
    }

    /*void GameScene::onKeyState( int16_t key, Key::State state, Unicode::Char character )
    {
    }*/

    void GameScene::onMouseMoveTo( const Vector2<int>& mouse )
    {
        this->mouse = mouse;

        if ( player != nullptr )
            player->setYaw( atan2( -( float )( int )( mouse.y - viewport.y / 2 ), ( float )( int )( mouse.x - viewport.x / 2 ) ) );
    }

    void GameScene::onRender()
    {
        unsigned maxDepth = 0;
        size_t numLeaves = 0;

        if ( player != nullptr )
        {
            Vector<> playerPos = player->getPos();
            camera.setEyePos( playerPos + Vector<>( 0.0f, 0.0f, 40.0f ) );
            //camera->setEyePos( playerPos + Vector<>( 0.0f, 0.0f, 25.0f ) );
            camera.setCenterPos( playerPos );
            camera.setUpVector( Vector<>( 0.0f, -1.0f, 0.0f ) );
        }

        sceneGraph->prerender();

        graphicsDriver->set3dMode( 1.0f, 1000.0f );
        graphicsDriver->setCamera( &camera );
        sceneGraph->render();

        if ( drawCollisions )
        {
            graphicsDriver->setRenderFlag( RenderFlag::depthTest, false );
            renderCtree2Node( ctree2, 1, maxDepth, numLeaves );
            graphicsDriver->setRenderFlag( RenderFlag::depthTest, true );
        }

        graphicsDriver->set2dMode( -1.0f, 1.0f );
        graphicsDriver->drawStats();

        if ( drawCollisions )
        {
            if ( osdFont == nullptr )
                osdFont = resMgr->getFont( "Common/Fonts/DejaVuSans.ttf", 14, IFont::bold );

            osdFont->drawString( viewport.getXy().convert<float>() - Vector2<>( 20.0f, 20.0f ), ( String ) "ctree depth = " + maxDepth + "; " + numLeaves + " leaves (" + leavesTested + " tested)",
                    Colour::green(), IFont::right | IFont::bottom );

            leavesTested = 0;
        }
    }

    void GameScene::onUpdate( double delta )
    {
        if ( player != nullptr )
        {
            if ( keyScanner->getKeyState( up ) )
                playerMove( Vector2<>( 0.0f, -1.0f ), ( float ) delta );

            if ( keyScanner->getKeyState( down ) )
                playerMove( Vector2<>( 0.0f, 1.0f ), ( float ) delta );

            if ( keyScanner->getKeyState( left ) )
                playerMove( Vector2<>( -1.0f, 0.0f ), ( float ) delta );

            if ( keyScanner->getKeyState( right ) )
                playerMove( Vector2<>( 1.0f, 0.0f ), ( float ) delta );
        }

        if ( socket != nullptr )
            processIncoming();

        if ( playerEnt != nullptr )
            processLocalEvents();
    }

    void GameScene::playerMove( Vector2<> vec, float delta )
    {
        static const float playerR = 0.8f;

        Movement movement = { player->getPos().getXy(), vec, playerR, delta * 7.0f };
        movement.recalc();

        if ( ctree2 != nullptr && !noclip )
            collideNode( ctree2, movement );

        vec = movement.vec * movement.speed;

        if ( socket != nullptr )
        {
            PlayerMoveMessage message = { MSG_PLAYER_MOVE, vec.x, vec.y, 0.0f };
            socket->write( &message, sizeof( message ) );
        }

        player->move( vec );
    }

    void GameScene::processIncoming()
    {
        MsgType msgType;

        while ( socket->read( &msgType, sizeof( msgType ), 0, true ) )
        {
            switch ( msgType )
            {
                case MSG_ENT_MOVE:
                {
                    EntMoveMessage message;

                    if ( !socket->read( &message, sizeof( message ), 0, false ) )
                        break;

                    Entity* ent = entities.get( message.entId );

                    if ( ent == nullptr )
                    {
                        printf( "Failed Movement of ent %i\n", message.entId );
                        break;
                    }

                    ent->modelNode->move( Vector<>( message.x, message.y, message.z ), false );
                    break;
                }

                case MSG_ENT_TELE:
                {
                    EntTeleMessage message;

                    if ( !socket->read( &message, sizeof( message ), 0, false ) )
                        break;

                    Entity* ent = entities.get( message.entId );

                    if ( ent == nullptr )
                    {
                        printf( "Failed Teleport of ent %i\n", message.entId );
                        break;
                    }

                    ent->modelNode->move( Vector<>( message.x, message.y, message.z ), true );
                    break;
                }

                case MSG_ENT_SPAWN:
                {
                    EntSpawnMessageHeader message;

                    if ( !socket->read( &message, sizeof( message ), 0, true ) )
                        break;

                    size_t totalLength = sizeof( message ) + message.nameLength;

                    receiveBuffer.resize( totalLength, true );

                    if ( socket->read( *receiveBuffer, totalLength, 0, false ) )
                    {
                        String name( ( const char* ) receiveBuffer.getPtr( sizeof( message ) ), message.nameLength );

                        spawnEntity( false, message.entId, name, Vector<>( message.x, message.y, message.z ) );
                    }

                    break;
                }

                case MSG_PLAYER_SPAWN:
                {
                    PlayerSpawnMessage message;

                    if ( !socket->read( &message, sizeof( message ), 0, false ) )
                        break;

                    spawnEntity( true, message.entId, "", Vector<>( message.x, message.y, message.z ) );
                    break;
                }

                default:
                    SG_assert( false )
            }
        }
    }

    void GameScene::processLocalEvents()
    {
        List<LocalEvent>& events = playerEnt->swapEventQueues();

        if ( !events.isEmpty() )
        {
            iterate2 ( i, events )
            {
                const LocalEvent& ev = i;

                switch ( ev.type )
                {
                    case EventType::entAngleZ:
                        // TODO: implement
                        break;

                    case EventType::entList:
                    {
                        //printf( "ClientSide Entity spawn #%i\n", ev.data.entSpawn->entId );

                        iterate2 ( i, ev.data.entList->ents )
                            spawnEntity( false, (*i).entId, (*i).name, (*i).pos );

                        delete ev.data.entList;
                        break;
                    }

                    case EventType::entMovement:
                    {
                        Entity* ent = entities.get( ev.data.entMovement.entId );

                        if ( ent == nullptr )
                        {
                            printf( "Failed Movement of ent %i (localevt)\n", ev.data.entMovement.entId );
                            break;
                        }

                        ent->modelNode->move( Vector<>( ev.data.entMovement.x, ev.data.entMovement.y, ev.data.entMovement.z ), false );
                        break;
                    }

                    case EventType::entSpawn:
                    {
                        printf( "ClientSide Entity spawn #%i\n", ev.data.entSpawn->entId );

                        spawnEntity( false, ev.data.entSpawn->entId, ev.data.entSpawn->name, ev.data.entSpawn->pos );

                        delete ev.data.entSpawn;
                        break;
                    }

                    case EventType::entTeleport:
                    {
                        Entity* ent = entities.get( ev.data.entTeleport.entId );

                        if ( ent == nullptr )
                        {
                            printf( "Failed Teleport of ent %i (localevt)\n", ev.data.entTeleport.entId );
                            break;
                        }

                        ent->modelNode->move( Vector<>( ev.data.entTeleport.x, ev.data.entTeleport.y, ev.data.entTeleport.z ), true );
                        break;
                    }

                    /*case EventType::entRespawn:
                    {
                        player = sceneGraph->addModel( resMgr->getModel( "Duel/Models/player.ms3d" ), ev.data.playerSpawn->pos, Vector<>() );
                        playerSpawned = true;

                        delete ev.data.entRespawn;
                        break;
                    }*/

                    case EventType::playerSpawn:
                        spawnEntity( true, ev.data.playerSpawn.entId, "", Vector<>( ev.data.playerSpawn.x, ev.data.playerSpawn.y, ev.data.playerSpawn.z ) );
                        break;
                }
            }

            events.clear( true );
        }
    }

    void GameScene::renderCtree2Node( const Ct2Node* node, unsigned depth, unsigned& maxDepth, size_t& numLeaves )
    {
        if ( depth > maxDepth )
            maxDepth = depth;

        graphicsDriver->drawRectangleOutline( node->bounds[0], node->bounds[1] - node->bounds[0], Colour( 0.0f, 1.0f, 0.0f, 0.2f ), nullptr );

        if ( !node->lines.isEmpty() )
            numLeaves++;

        Vector<> mouseProj;

        if ( graphicsDriver->unproject( mouse, mouseProj ) && !( mouseProj.getXy() < node->bounds[0] || mouseProj.getXy() > node->bounds[1] ) )
        {
            Colour colour = node->lines.isEmpty() ? Colour::white( 0.05f ) : Colour( 0.0f, 0.0f, 1.0f, 0.25f );

            graphicsDriver->drawRectangle( node->bounds[0], node->bounds[1] - node->bounds[0], colour, nullptr );
        }

        iterate2 ( i, node->lines )
            graphicsDriver->drawLine( (*i).a, (*i).b, Colour( 1.0f, 0.0f, 0.0f, 0.5f ) );

        if ( node->children[0] )
            renderCtree2Node( node->children[0], depth + 1, maxDepth, numLeaves );

        if ( node->children[1] )
            renderCtree2Node( node->children[1], depth + 1, maxDepth, numLeaves );
    }

    void GameScene::spawnEntity( bool localplayer, uint16_t entId, String name, const Vector<>& pos )
    {
        printf( "\nAdding '%s'\n", name.c_str() );
        printf( "spawn of ent %i\n\n", entId );

        IModelNode* modelNode = sceneGraph->addModel( resMgr->getModel( "Duel/Models/player.ms3d" ), pos, Vector<>() );

        if ( localplayer )
        {
            SG_assert( !playerSpawned )

            player = modelNode;
            playerSpawned = true;
        }
        else
        {
            Entity* entity = new Entity;
            entity->entId = entId;
            entity->modelNode = modelNode;
            entities.set( ( uint16_t&& ) entId, ( Entity*&& ) entity );
        }
    }

    void GameScene::uninit()
    {
        if ( server != nullptr )
            server->stop();

        iterate2 ( i, entities )
            delete (*i).value;

        entities.clear();

        // Unlink
        sg->setVariable( "drawCollisions", sg->createBoolVariable( drawCollisions ), false );
    }
}
