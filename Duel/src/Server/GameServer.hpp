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

#include "Server.hpp"

#include <littl/TcpSocket.hpp>
#include <littl/Thread.hpp>

namespace Duel
{
    class GameServerThread;

    class BotEnt : public IEnt
    {
        EntId entId;
        String name;
        Vector<> pos;
        bool spawned;

        float angle, dist, speed;

        GameServerThread* thread;

        public:
            BotEnt( GameServerThread* thread, EntId entId );
            //virtual ~BotEnt();

            void init();
            void update( double delta );

            virtual bool isSpawned() override { return spawned; }
            virtual EntId getId() { return entId; }
            virtual const char* getName() override { return name; }
            virtual Vector<> getPos() override;
            virtual void spawnAt( const Vector<>& pos ) override;
    };

    class LocalPlayerEnt : public ILocalPlayerEnt, protected Mutex
    {
        EntId entId;
        String name;
        Vector<> pos;
        bool spawned;

        GameServerThread* thread;

        List<LocalEvent> eventQueues[2];
        int current;

        public:
            LocalPlayerEnt( GameServerThread* thread, const char* name, EntId entId );
            //virtual ~LocalPlayerEnt();

            void addEvent( const LocalEvent& ev );
            void init();

            // Duel.IEnt
            virtual bool isSpawned() override { return spawned; }
            virtual EntId getId() { return entId; }
            virtual const char* getName() override { return name; }
            virtual Vector<> getPos() override { return pos; }
            virtual void spawnAt( const Vector<>& pos ) override;

            // Duel.ILocalPlayerEnt
            virtual List<LocalEvent>& swapEventQueues() override;

            virtual void move( const Vector<>& vec ) override {}
    };

    class RemotePlayerEnt : public IEnt
    {
        protected:
            EntId entId;
            String name;
            Vector<> pos;
            bool spawned;

            GameServerThread* thread;
            Reference<TcpSocket> socket;

            bool hasSaidHello;

            ArrayIOStream messageBuffer, receiveBuffer;

        public:
            RemotePlayerEnt( GameServerThread* thread, EntId entId, TcpSocket* socket );
            //virtual ~RemotePlayerEnt();

            template <typename T> void bufferMessage( const T& message ) { bufferRaw( &message, sizeof( message ) ); }
            void bufferRaw( const void* data, size_t length );

            virtual bool isSpawned() override { return spawned; }
            virtual EntId getId() override { return entId; }
            virtual const char* getName() override { return name; }
            virtual Vector<> getPos() override { return pos; }
            virtual void spawnAt( const Vector<>& pos ) override;

            virtual void flush();
            virtual void init();
            virtual void update();            
    };

    class GameServer : public ICommandListener, public IGameServer
    {
        protected:
            IEngine* engine;

            Object<GameServerThread> thread;

        public:
            GameServer( IEngine* engine );
            virtual ~GameServer();

            virtual ILocalPlayerEnt* addLocalPlayer( const char* name ) override;

            virtual void init() override;
            virtual bool onCommand( const List<String>& tokens ) override;
            virtual void start() override;
            virtual void stop() override;
            virtual void uninit() override;
    };

    class GameServerThread : public Thread
    {
        public:
            IEngine* engine;

            volatile uint16_t nextEntId;

            Reference<TcpSocket> listenSocket;

            List<BotEnt*> bots;
            List<LocalPlayerEnt*> localPlayers;
            List<RemotePlayerEnt*> remotePlayers;

            List<IEnt*> spawnQueue;

            Mutex botsMtx, localPlayersMtx, remotePlayersMtx, spawnQueueMtx;

        public:
            GameServerThread( IEngine* engine );
            virtual ~GameServerThread();

            // on-update: call these only when the respective pools are locked
            void broadcastLocal( LocalPlayerEnt* exclude, const LocalEvent& ev );
            void broadcastRemote( RemotePlayerEnt* exclude, const void* data, size_t length );



            void addBot( BotEnt* ent );
            LocalPlayerEnt* addLocalPlayer( const char* name );
            void addRemotePlayer( TcpSocket* socket );
            void broadcastEntSpawn( IEnt* ent );
            void init();
            void initTcpHost();
            void readyToSpawn( IEnt* ent );
            void spawnBot1();
            void uninit();

            virtual void run() override;
    };

    class DedicatedGameServer : public IDedicatedServer
    {
        protected:
            Object<IEngine> engine;

            Object<GameServer> server;

        public:
            virtual bool initDedicatedServer( int argc, char** argv ) override;
            virtual void runDedicatedServer() override;
    };
}