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

    extern Object<IEngine> sg;

    const static uint32_t clientVersion = 3;

    constexpr double M_PI = 3.1415;

    class Resources
    {
        static IResourceManager* bootstrapResMgr, * uiResMgr, * musicResMgr;

        public:
            Resources();
            ~Resources();

            static IResourceManager* getBootstrapResMgr();
            static IResourceManager* getUiResMgr( bool create = true );
            static IResourceManager* getMusicResMgr( bool create = true );
    };
}
