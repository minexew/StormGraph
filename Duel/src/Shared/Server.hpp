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

#include <StormGraph/Server.hpp>

namespace Duel
{
    using namespace StormGraph;

    li_enum_class( EventType ) { entAngleZ, entList, entMovement, entSpawn, entTeleport, playerSpawn };

    typedef uint16_t EntId;
    typedef uint16_t MsgType;

    static const MsgType MSG_JOIN               = 0x0001;
    static const MsgType MSG_PLAYER_MOVE        = 0x0101;
    static const MsgType MSG_PLAYER_SPAWN       = 0x0200;
    static const MsgType MSG_ENT_SPAWN          = 0x0201;
    static const MsgType MSG_ENT_MOVE           = 0x0202;
    static const MsgType MSG_ENT_TELE           = 0x0203;

    struct EntAngleZData
    {
        float angle;
    };
    
    typedef struct EntMovementData
    {
        uint16_t entId;
        float x, y, z;
    }
    EntTeleportData;

    /*struct EntRespawnData
    {
        uint16_t entId;
        float x, y, z;
    };*/

    struct EntSpawnData
    {
        uint16_t entId;
        Vector<> pos;
        String name;
    };

    struct EntListData
    {
        List<EntSpawnData> ents;
    };

    struct PlayerSpawnData
    {
        uint16_t entId;
        float x, y, z;
    };

    struct LocalEvent
    {
        EventType type;

        union
        {
            EntAngleZData entAngleZ;
            EntListData* entList;
            EntMovementData entMovement;
            //EntRespawnData entRespawnData;
            EntSpawnData* entSpawn;
            EntTeleportData entTeleport;
            PlayerSpawnData playerSpawn;
        }
        data;
    };

    typedef struct EntMoveMessage
    {
        MsgType type;

        uint16_t entId;
        float x, y, z;
    } EntTeleMessage;

    struct EntSpawnMessageHeader
    {
        MsgType type;

        uint16_t entId;
        float x, y, z;
        uint16_t nameLength;
    };

    struct JoinHeader
    {
        MsgType type;

        uint16_t nameLength;
    };

    struct PlayerMoveMessage
    {
        MsgType type;

        float x, y, z;
    };

    struct PlayerSpawnMessage
    {
        MsgType type;

        uint16_t entId;
        float x, y, z;
    };

    class IEnt
    {
        public:
            virtual EntId getId() = 0;
            virtual const char* getName() = 0;
            virtual Vector<> getPos() = 0;
            virtual bool isSpawned() = 0;

            virtual void spawnAt( const Vector<>& pos ) = 0;
    };

    class ILocalPlayerEnt : public IEnt
    {
        public:
            virtual void move( const Vector<>& vec ) = 0;

            //virtual size_t getNumBlobsAvailable() = 0;
            //virtual void retrieveBlobs( Blob* structs, size_t count ) = 0;
            virtual List<LocalEvent>& swapEventQueues() = 0;
    };

    class IGameServer// : public IDedicatedServer
    {
        public:
            virtual ~IGameServer() {}

            virtual ILocalPlayerEnt* addLocalPlayer( const char* name ) = 0;

            virtual void init() = 0;
            virtual void start() = 0;
            virtual void stop() = 0;
            virtual void uninit() = 0;

            //virtual void init( int argc, char** argv ) override;
            //virtual void run() override;
    };

    class IGameServerProvider
    {
        public:
            virtual ~IGameServerProvider() {}

            virtual IGameServer* createGameServer( IEngine* engine ) = 0;
    };
}