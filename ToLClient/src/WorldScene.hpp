/*
    Tales of Lanthaia Game Client
    Copyright (c) 2011 The Tales of Lanthaia Project

    All rights reserved.
    Created by: Xeatheran Minexew
*/

#pragma once

#include "TolClient.hpp"

#include <StormGraph/Scene.hpp>

#include <littl/TcpSocket.hpp>

namespace TolClient
{
    class WorldScene: public IScene
    {
        public:
            WorldScene( TcpSocket* socket );
            virtual ~WorldScene();

            virtual void render();
    };
}
