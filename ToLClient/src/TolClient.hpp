/*
    Tales of Lanthaia Game Client
    Copyright (c) 2011 The Tales of Lanthaia Project

    All rights reserved.
    Created by: Xeatheran Minexew
*/

#pragma once

#include <StormGraph/Engine.hpp>
#include <StormGraph/Sys.hpp>
#include <StormGraph/VisualInterface.hpp>

namespace TolClient
{
    using namespace StormBase;
    using namespace StormGraph;
    using namespace StormRender;

    extern IEngine* engine;

    extern IR* r;
    extern ISys* sys;

    const static uint32_t clientVersion = 3;

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
