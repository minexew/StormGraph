
#include "GameClient.hpp"

namespace GameClient
{
    ItemIconManager::ItemIconManager( ResourceManager* resMgr )
            : resMgr( resMgr )
    {
    }

    ItemIconManager::~ItemIconManager()
    {
        for ( unsigned i = 0; i < icons.getCapacity(); i++ )
            if ( icons[i] )
                icons[i]->release();
    }

    Texture* ItemIconManager::get( unsigned itemId )
    {
        if ( !icons[itemId] )
        {
            char buffer[MAX_PATH];

            snprintf( buffer, MAX_PATH, "tolcl/gfx/item/item%03u.png", itemId );
            icons[itemId] = resMgr->getTexture( buffer );
        }

        return icons[itemId];
    }
}
