
#include "MainMenu.hpp"

#include <io.h>

namespace GameClient
{
    Engine* sg;
    ResourceManager* globalResMgr;
    ItemIconManager* globalItemIconMgr;
    Picking* picking;

    static void run()
    {
        try
        {
            sg = new Engine( "tolcl" );

            // virtual filesystems

            sg->addFileSystem( "mox:common/sg_assets.mox" );

            sg->addFileSystem( "mox:tolcl/gfx_fonts.mox" );
            sg->addFileSystem( "mox:tolcl/gfx_ui.mox" );

            sg->addFileSystem( "mox:tolcl/model_char0.mox" );
            sg->addFileSystem( "mox:tolcl/model_env0.mox" );
            sg->addFileSystem( "mox:tolcl/model_item0.mox" );
            sg->addFileSystem( "mox:tolcl/model_struct0.mox" );

            sg->addFileSystem( "mox:tolcl/tex_char0.mox" );
            sg->addFileSystem( "mox:tolcl/tex_env0.mox" );
            sg->addFileSystem( "mox:tolcl/tex_item0.mox" );
            sg->addFileSystem( "mox:tolcl/tex_struct0.mox" );

            sg->addFileSystem( "mox:tolcl/heightmap.mox" );
            sg->addFileSystem( "mox:tolcl/world.mox" );

            sg->setMode( "Tales of Lanthaia" );

            picking = new Picking();

            // resource manager

            globalResMgr = new ResourceManager();
            globalResMgr->addTexturePath( "storm/tex/" );
            globalResMgr->addTexturePath( "tolcl/tex/" );
            globalResMgr->addModelPath( "" );

            // fonts

            String uiFont = sg->getConfig( "tolcl/uiFont", false );
            String chatFontSize = sg->getConfig( "tolcl/chatFontSize", false );

            if ( uiFont.isEmpty() )
                uiFont = "tolcl/gfx/CELTG___.TTF";

            // ui font
            globalResMgr->addNamedFont( "ui", uiFont, 16, 128 );
            globalResMgr->addNamedFont( "ui_big", uiFont, 64, 128 );
            globalResMgr->addNamedFont( "ui_small", uiFont, 12, 128 );

            // chat font
            globalResMgr->addNamedFont( "chat", "tolcl/gfx/DejaVuSans.ttf", !chatFontSize.isEmpty() ? ( int )chatFontSize : 16, 512 );

            // legacy
            globalResMgr->addNamedFont( "font_ui", uiFont, 16 );

            globalItemIconMgr = new ItemIconManager( globalResMgr );

            MainMenuScene* menu = new MainMenuScene();

#ifdef __li_MSW
            HWND launcherWindow = FindWindow( "LanthaiaLauncher", 0 );

            if ( launcherWindow )
                PostMessage( launcherWindow, WM_CLOSE, 0, 0 );
#endif

            sg->setCursor( true, "tolcl/gfx/cursor.png" );
            sg->run( menu );

            delete globalItemIconMgr;
            delete globalResMgr;

            delete sg;
        }
        catch ( StormGraph::Exception ex )
        {
            ex.print();
        }
    }
}

int main( int argc, char** argv )
{
    GameClient::run();
    return 0;
}
