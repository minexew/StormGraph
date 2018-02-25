#pragma once

#include "GameClient.hpp"

namespace GameClient
{
    class GameScene;

    class WorldSession
    {
        TcpSocket* socket;
        StreamBuffer<> buffer;

        GameScene* game;
        unsigned playerPid;

        public:
            WorldSession( TcpSocket* socket, GameScene* game );
            ~WorldSession();

            void chat( const String& text );
            void movement( const Vector<float>& pos, float angle );
            void process();
    };
}
