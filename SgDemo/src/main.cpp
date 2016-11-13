
#include "DemoScene.hpp"

#include <StormGraph/Engine.hpp>

using namespace StormGraph;

int main( int argc, char** argv )
{
    bool showConsole = false, dumpLog = false;

    try
    {
        Engine* sg = new Engine( "sg_demo" );
        sg->addFileSystem( "native:" );
        sg->startup();

        GraphicsDriver* driver = sg->getGraphicsDriver();

        sg->addFileSystem( "mox:sg_demo.mox" );

        // Some settings
        showConsole = sg->getConfigInt( "Dev/showConsole", false );

        // Set-up the Resource Manager
        Reference<ResourceManager> resMgr = new ResourceManager( "resource_manager", sg->getFileSystem() );

        resMgr->addPath( "" );
        resMgr->addPath( "sg_demo/res/" );
        resMgr->addPath( "tolcl/tex/char0/" );

        resMgr->addPath( "tolcl/model/char0/" );

        // Set Display Mode
        DisplayMode displayMode;
        sg->getDefaultDisplayMode( &displayMode );

        displayMode.windowTitle = "SG_Demo";
        driver->setDisplayMode( &displayMode );

        // Set LOD
        LevelOfDetail lod;
        sg->getDefaultLodSettings( &lod );
        driver->setLevelOfDetail( &lod );

        // Go!
        sg->run( new SgDemo::DemoScene( driver, resMgr, displayMode ) );
        resMgr->releaseUnused();

        if ( showConsole )
        {
            printf( "--- UNRELEASED RESOURCES BEGIN ---\n" );
            resMgr->listResources();
            printf( "--- UNRELEASED RESOURCES END ---\n" );
        }

        resMgr.release();

        delete sg;
    }
    catch ( li::Exception ex )
    {
        Engine::displayException( ex, false );

        Engine::logEvent( "SgDemo", "Exception caught.\n\n"
                "<b>Function</b>: " + ex.functionName + "\n"
                "<b>Exception</b>: " + ex.getName() + "\n\n"
                "<b>Description</b>: " + ex.getDesc() + "\n\n" );

        dumpLog = true;
    }

    if ( showConsole )
        printf( "--- DUMPING EVENT LOG ---\n" );

    if ( showConsole || dumpLog )
        Engine::printEventLog( File::open( "Log.html", "wb" ) );

    if ( showConsole )
    {
        printf( "\n--- DONE ---\n" );
        getchar();
    }

    return 0;
}
