/*
    Tales of Lanthaia Game Client
    Copyright (c) 2011 The Tales of Lanthaia Project

    All rights reserved.
    Created by: Xeatheran Minexew
*/

#pragma once

#include <StormGraph/Engine.hpp>

namespace TolClient
{
    using namespace StormGraph;

    extern Object<Engine> sg;

    const static uint32_t clientVersion = 3;

    class Resources
    {
        static ResourceManager* bootstrapResMgr, * uiResMgr, * musicResMgr;

        public:
            Resources();
            ~Resources();

            static ResourceManager* getBootstrapResMgr();
            static ResourceManager* getUiResMgr( bool create = true );
            static ResourceManager* getMusicResMgr( bool create = true );
    };
}
