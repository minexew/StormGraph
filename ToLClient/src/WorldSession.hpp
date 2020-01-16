#pragma once

//#include "GameClient.hpp"

#include <StormGraph/Abstract.hpp>

#include <littl.hpp>

namespace TolClient
{
    class WorldScene;

    class WorldSession
    {
        li::TcpSocket* socket;
        li::ArrayIOStream buffer;

        WorldScene* game;
        unsigned playerPid;

    public:
        WorldSession( li::TcpSocket* socket, WorldScene* game );
        ~WorldSession();

        void chat( const li::String& text );
        void movement( const StormGraph::Vector<float>& pos, float angle );
        void process();
    };
}
