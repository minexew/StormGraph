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

#pragma once

#include "GameServer.hpp"

#include <StormGraph/Engine.hpp>

#include <littl/Console.hpp>

namespace Duel
{
    BotEnt::BotEnt( GameServerThread* thread, EntId entId )
            : entId( entId ), spawned( false ), thread( thread )
    {
        name = "Derpy";
    }

    Vector<> BotEnt::getPos()
    {
        return pos;
    }

    void BotEnt::init()
    {
        thread->readyToSpawn( this );
    }

    void BotEnt::spawnAt( const Vector<>& pos )
    {
        this->pos = pos;
        spawned = true;

        angle = 0.0f;
        dist = 4.5f;
        speed = 7.0f / dist;
    }

    void BotEnt::update( double delta )
    {
        angle += speed * delta;
        pos.x = cos( angle ) * dist;
        pos.y = -sin( angle ) * dist;

        LocalEvent ev;
        ev.type = entTeleport;
        ev.data.entTeleport.entId = entId;
        ev.data.entTeleport.x = pos.x;
        ev.data.entTeleport.y = pos.y;
        ev.data.entTeleport.z = pos.z;
        thread->broadcastLocal( nullptr, ev );

        EntTeleMessage movement = { MSG_ENT_TELE, entId, pos.x, pos.y, pos.z };
        thread->broadcastRemote( nullptr, &movement, sizeof( movement ) );
    }

    LocalPlayerEnt::LocalPlayerEnt( GameServerThread* thread, const char* name, EntId entId )
            : entId( entId ), name( name ), spawned( false ), thread( thread ), current( 0 )
    {
    }

    void LocalPlayerEnt::addEvent( const LocalEvent& ev )
    {
        CriticalSection cs( this );

        eventQueues[current].add( ev );
    }

    void LocalPlayerEnt::init()
    {
        thread->readyToSpawn( this );
    }

    void LocalPlayerEnt::spawnAt( const Vector<>& pos )
    {
        this->pos = pos;
        spawned = true;

        LocalEvent ev;
        ev.type = EventType::playerSpawn;
        ev.data.playerSpawn.entId = entId;
        ev.data.playerSpawn.x = pos.x;
        ev.data.playerSpawn.y = pos.y;
        ev.data.playerSpawn.z = pos.z;

        addEvent( ev );
    }

    List<LocalEvent>& LocalPlayerEnt::swapEventQueues()
    {
        CriticalSection cs( this );

        List<LocalEvent>& q = eventQueues[current];
        current ^= 1;

        return q;
    }

    RemotePlayerEnt::RemotePlayerEnt( GameServerThread* thread, EntId entId, TcpSocket* socket )
            : entId( entId ), spawned( false ), thread( thread ), socket( socket ), hasSaidHello( false )
    {
    }

    /*RemotePlayerEnt::~RemotePlayerEnt()
    {
    }*/

    void RemotePlayerEnt::bufferRaw( const void* data, size_t length )
    {
        messageBuffer.write( data, length );
    }

    void RemotePlayerEnt::flush()
    {
        if ( messageBuffer.getSize() > 0 )
        {
            socket->write( messageBuffer.getPtr(), messageBuffer.getSize() );
            messageBuffer.clear();
        }
    }

    void RemotePlayerEnt::init()
    {
        ArrayIOStream prewelcome;

        prewelcome.writeString( "Storm.Sandbox" );
        prewelcome.writeString( thread->engine->getVariableValue( "map_name", true ) );
        prewelcome.write<int16_t>( 0 );
        prewelcome.write<int16_t>( 8 );

        socket->send( prewelcome );
    }

    void RemotePlayerEnt::spawnAt( const Vector<>& pos )
    {
        this->pos = pos;
        spawned = true;

        PlayerSpawnMessage message = { MSG_PLAYER_SPAWN, entId, pos.x, pos.y, pos.z };
        socket->write( &message, sizeof( message ) );
    }

    void RemotePlayerEnt::update()
    {
        //MsgType msgType;

        if ( !hasSaidHello )
        {
            JoinHeader message;

            if ( socket->read( &message, sizeof( message ), 0, true ) )
            {
                if ( message.type == MSG_JOIN )
                {
                    size_t totalLength = sizeof( message ) + message.nameLength;

                    receiveBuffer.resize( totalLength, true );

                    if ( socket->read( *receiveBuffer, totalLength, 0, false ) )
                    {
                        name.set( ( const char* ) receiveBuffer.getPtr( sizeof( message ) ), message.nameLength );

                        printf( "%s connected.\n", name.c_str() );

                        hasSaidHello = true;

                        thread->readyToSpawn( this );
                    }
                }
                else
                {
                    // FIXME: error here
                    SG_assert( false )
                }
            }
        }
        else
        {
            MsgType msgType;

            if ( socket->read( &msgType, sizeof( msgType ), 0, true ) )
            {
                switch ( msgType )
                {
                    case MSG_PLAYER_MOVE:
                    {
                        PlayerMoveMessage message;

                        if ( !socket->read( &message, sizeof( message ), 0, false ) )
                            break;

                        // Broadcast locally
                        LocalEvent ev;
                        ev.type = entMovement;
                        ev.data.entMovement.entId = entId;
                        ev.data.entMovement.x = message.x;
                        ev.data.entMovement.y = message.y;
                        ev.data.entMovement.z = message.z;
                        thread->broadcastLocal( nullptr, ev );

                        // Broadcast remote
                        EntMoveMessage movement = { MSG_ENT_MOVE, entId, message.x, message.y, message.z };
                        thread->broadcastRemote( this, &movement, sizeof( movement ) );

                        // Update self
                        pos.x += message.x;
                        pos.y += message.y;
                        pos.z += message.z;
                        break;
                    }

                    default:
                        SG_assert( false )
                }
            }
        }
    }

    GameServer::GameServer( IEngine* engine ) : engine( engine )
    {
    }

    GameServer::~GameServer()
    {
    }

    ILocalPlayerEnt* GameServer::addLocalPlayer( const char* name )
    {
        return thread->addLocalPlayer( name );
    }

    void GameServer::init()
    {
        engine->setVariable( "host_max",        engine->createIntVariable( -1 ),                true );
        engine->setVariable( "host_port",       engine->createIntVariable( 0xD0E1 ),            true );
        //engine->setVariable( "map_name",        engine->createStringVariable( "uc_0" ),         true );

        thread = new GameServerThread( engine );
        thread->init();

        if ( engine->getVariableValue( "hostname", false ) != "#nonet" )
            thread->initTcpHost();

        engine->registerCommandListener( this );

        engine->executeFile( "Duel/servermain.txt" );
        engine->executeFile( "Duel/servermain_" StormGraph_BuildTarget ".txt" );
    }

    bool GameServer::onCommand( const List<String>& tokens )
    {
        if ( tokens[0] == "debugbot" )
        {
            thread->spawnBot1();
            return true;
        }

        return false;
    }

    void GameServer::start()
    {
        thread->start();
    }

    void GameServer::stop()
    {
        thread->end();
        thread->waitFor();
    }

    void GameServer::uninit()
    {
        engine->unregisterCommandListener( this );
    }

    GameServerThread::GameServerThread( IEngine* engine )
            : engine( engine ), nextEntId( 0 )
    {
    }

    GameServerThread::~GameServerThread()
    {
        {
            CriticalSection cs( botsMtx );

            iterate2 ( i, bots )
                delete i;
        }

        {
            CriticalSection cs( localPlayersMtx );

            iterate2 ( i, localPlayers )
                delete i;
        }

        {
            CriticalSection cs( remotePlayersMtx );

            iterate2 ( i, remotePlayers )
                delete i;
        }

        /*
        iterate2 ( i, remotePlayers )
            delete i;
            */
    }

    void GameServerThread::addBot( BotEnt* ent )
    {
        CriticalSection cs( botsMtx );

        bots.add( ent );
    }

    LocalPlayerEnt* GameServerThread::addLocalPlayer( const char* name )
    {
        LocalPlayerEnt* ent = new LocalPlayerEnt( this, name, nextEntId++ );
        ent->init();

        LocalEvent ev;
        ev.type = EventType::entList;
        ev.data.entList = new EntListData;

        if ( !localPlayers.isEmpty() )
        {
            CriticalSection cs( localPlayersMtx );

            iterate2( i, localPlayers )
            {
                if ( i->isSpawned() )
                {
                    EntSpawnData item = { i->getId(), i->getPos(), i->getName() };

                    ev.data.entList->ents.add( ( EntSpawnData&& ) item );
                }
            }
        }

        if ( !remotePlayers.isEmpty() )
        {
            CriticalSection cs( remotePlayersMtx );

            iterate2( i, remotePlayers )
            {
                if ( i->isSpawned() )
                {
                    EntSpawnData item = { i->getId(), i->getPos(), i->getName() };

                    ev.data.entList->ents.add( ( EntSpawnData&& ) item );
                }
            }
        }

        if ( !bots.isEmpty() )
        {
            CriticalSection cs( botsMtx );
            
            iterate2 ( i, bots )
            {
                if ( i->isSpawned() )
                {
                    EntSpawnData item = { i->getId(), i->getPos(), i->getName() };

                    ev.data.entList->ents.add( ( EntSpawnData&& ) item );
                }
            }
        }

        ent->addEvent( ev );

        localPlayers.add( ent );

        return ent;
    }

    void GameServerThread::addRemotePlayer( TcpSocket* socket )
    {
        Object<RemotePlayerEnt> ent;

        try
        {
            ent = new RemotePlayerEnt( this, nextEntId++, socket );
            ent->init();

            if ( !localPlayers.isEmpty() )
            {
                CriticalSection cs( localPlayersMtx );

                iterate2( i, localPlayers )
                {
                    if ( i->isSpawned() )
                    {
                        const Vector<> pos = i->getPos();
                        String name = i->getName();

                        EntSpawnMessageHeader message = { MSG_ENT_SPAWN, i->getId(), pos.x, pos.y, pos.z, name.getNumBytes() };

                        ent->bufferMessage( message );
                        ent->bufferRaw( name, name.getNumBytes() );
                    }
                }
            }

            if ( !bots.isEmpty() )
            {
                CriticalSection cs( botsMtx );
            
                iterate2 ( i, bots )
                {
                    if ( i->isSpawned() )
                    {
                        const Vector<> pos = i->getPos();
                        String name = i->getName();

                        EntSpawnMessageHeader message = { MSG_ENT_SPAWN, i->getId(), pos.x, pos.y, pos.z, name.getNumBytes() };

                        ent->bufferMessage( message );
                        ent->bufferRaw( name, name.getNumBytes() );
                    }
                }
            }

            CriticalSection cs( remotePlayersMtx );

            iterate2( i, remotePlayers )
            {
                if ( i->isSpawned() )
                {
                    const Vector<> pos = i->getPos();
                    String name = i->getName();

                    EntSpawnMessageHeader message = { MSG_ENT_SPAWN, i->getId(), pos.x, pos.y, pos.z, name.getNumBytes() };

                    ent->bufferMessage( message );
                    ent->bufferRaw( name, name.getNumBytes() );
                }
            }

            remotePlayers.add( ent.detach() );
        }
        catch ( Exception ex )
        {
            ex.print();
        }
    }

    void GameServerThread::broadcastEntSpawn( IEnt* ent )
    {
        uint16_t entId = ent->getId();

        printf( "[%04X] Spawning '%s' @ %s\n", entId, ent->getName(), ent->getPos().toString().c_str() );

        if ( !localPlayers.isEmpty() )
        {
            CriticalSection cs( localPlayersMtx );

            iterate2 ( i, localPlayers )
            {
                if ( i == ent )
                    continue;

                LocalEvent ev;

                ev.type = EventType::entSpawn;

                ev.data.entSpawn = new EntSpawnData;
                ev.data.entSpawn->entId = entId;
                ev.data.entSpawn->pos = ent->getPos();
                ev.data.entSpawn->name = ent->getName();

                i->addEvent( ev );
            }
        }

        if ( !remotePlayers.isEmpty() )
        {
            CriticalSection cs( remotePlayersMtx );

            iterate2 ( i, remotePlayers )
            {
                if ( i == ent )
                    continue;

                const Vector<> pos = ent->getPos();
                String name = ent->getName();

                EntSpawnMessageHeader message = { MSG_ENT_SPAWN, entId, pos.x, pos.y, pos.z, name.getNumBytes() };

                i->bufferMessage( message );
                i->bufferRaw( name, name.getNumBytes() );
            }
        }
    }

    void GameServerThread::broadcastLocal( LocalPlayerEnt* exclude, const LocalEvent& ev )
    {
        iterate2 ( i, localPlayers )
        {
            if ( i != exclude )
                i->addEvent( ev );
        }
    }

    void GameServerThread::broadcastRemote( RemotePlayerEnt* exclude, const void* data, size_t length )
    {
        iterate2 ( i, remotePlayers )
        {
            if ( i != exclude )
                i->bufferRaw( data, length );
        }
    }

    void GameServerThread::init()
    {
    }

    void GameServerThread::initTcpHost()
    {
        int host_max = String::toInt( engine->getVariableValue( "host_max", true ) );
        int host_port = String::toInt( engine->getVariableValue( "host_port", true ) );

        SG_assert ( host_port > 0 )

        listenSocket = TcpSocket::create( false );

        SG_assert ( listenSocket->listen( host_port ) )
    }

    void GameServerThread::run()
    {
        unsigned counter = 0, interval = 0;
        const unsigned intervalSecs = 5;
        Timer timer;

        Timer deltaTimer;

        while ( !shouldEnd )
        {
            if ( counter == 0 )
                timer.start();
            else if ( counter == 1 )
            {
                printf( "Delta Time: %u us\n\n", ( unsigned int ) timer.getMicros() );
                interval = intervalSecs * 1000000 / timer.getMicros();
                timer.stop();
            }

            if ( listenSocket != nullptr )
            {
                Reference<TcpSocket> incoming = listenSocket->accept( false );

                if ( incoming != nullptr )
                    addRemotePlayer( incoming.detach() );
            }

            double delta = deltaTimer.getMicros() / 1000000.0;
            deltaTimer.start();

            {
                // Lock and update all players/ents

                CriticalSection cs3( localPlayersMtx );
                CriticalSection cs( remotePlayersMtx );
                CriticalSection cs2( botsMtx );

                iterate2 ( i, bots )
                    i->update( delta );

                iterate2 ( i, remotePlayers )
                    i->update();

                iterate2 ( i, remotePlayers )
                    i->flush();
            }

            if ( !spawnQueue.isEmpty() )
            {
                CriticalSection cs( spawnQueueMtx );

                iterate2 ( ent, spawnQueue )
                {
                    ent->spawnAt( Vector<>( rand() % 16 - 8, rand() % 16 - 8, rand() % 2 ) );
                    broadcastEntSpawn( ent );
                }

                spawnQueue.clear();
            }

            if ( counter == 0 )
                printf( "Update Time: %u us\n", ( unsigned int ) timer.getMicros() );

            if ( ++counter == interval )
                counter = 0;

            pauseThread( 1 );
        }
    }

    void GameServerThread::readyToSpawn( IEnt* ent )
    {
        CriticalSection cs( spawnQueueMtx );

        spawnQueue.add( ent );
    }

    void GameServerThread::spawnBot1()
    {
        Object<BotEnt> ent = new BotEnt( this, nextEntId++ );
        ent->init();

        addBot( ent.detach() );
    }

    bool DedicatedGameServer::initDedicatedServer( int argc, char** argv )
    {
        printf( "Minexew Games Storm Dedicated Server 1.0 (" StormGraph_BuildTarget ")\n" );

#ifdef _DEBUG
        /*_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF );
        //_CrtSetBreakAlloc( 11103 );

        printf( "##########################################\n" );
        printf( "  Leak Detection Enabled\n" );
        printf( "##########################################\n" );*/
#endif

        try
        {
            engine = Common::getCore( StormGraph_API_Version )->createEngine( "Duel", argc, argv );

            engine->addFileSystem( "native" );

            engine->startup();

            server = new GameServer( engine );
            server->init();

            return true;
        }
        catch ( Exception& ex )
        {
            Common::displayException( ex, false );
        }

        return false;
    }

    void DedicatedGameServer::runDedicatedServer()
    {
        server->start();

        while ( true )
        {
            Console::write( ">" );
            String line = Console::readLine();

            if ( line.isEmpty() )
                break;

            engine->command( line );
        }

        server->stop();
        server->uninit();

        server.release();

        engine.release();

        Common::releaseModules();
    }
}