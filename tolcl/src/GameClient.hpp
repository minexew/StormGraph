#pragma once

#include <StormGraph/StormGraph.hpp>

namespace GameClient
{
    using namespace StormGraph;

    extern Engine* sg;
    extern ResourceManager* globalResMgr;
    extern Picking* picking;

    const static unsigned maxLights = 4;

    class ItemIconManager
    {
        ResourceManager* resMgr;
        Array<Texture*> icons;

        public:
            ItemIconManager( ResourceManager* resMgr );
            ~ItemIconManager();

            Texture* get( unsigned itemId );
    };

    extern ItemIconManager* globalItemIconMgr;
}
