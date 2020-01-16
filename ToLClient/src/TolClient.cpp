
#include "InitScene.hpp"
#include "TolClient.hpp"

#include <StormGraph/Engine.hpp>
#include <StormGraph/ResourceManager.hpp>

#include <littl/File.hpp>

using namespace StormGraph;

namespace TolClient
{
    IResourceManager* Resources::bootstrapResMgr, * Resources::uiResMgr, * Resources::musicResMgr;
    Object<IEngine> sg;

    IResourceManager* globalResMgr;

    Resources::Resources()
    {
        bootstrapResMgr = 0;
        uiResMgr = 0;
    }

    Resources::~Resources()
    {
        li::release( bootstrapResMgr );
        li::release( uiResMgr );
        li::release( musicResMgr );
    }

    IResourceManager* Resources::getBootstrapResMgr()
    {
        if ( !bootstrapResMgr )
            bootstrapResMgr = sg->createResourceManager( "bootstrapResMgr", true );

        return bootstrapResMgr;
    }

    IResourceManager* Resources::getUiResMgr( bool create )
    {
        if ( !uiResMgr && create )
            uiResMgr = sg->createResourceManager( "uiResMgr", true );

        return uiResMgr;
    }

    IResourceManager* Resources::getMusicResMgr( bool create )
    {
        if ( !musicResMgr && create )
            musicResMgr = sg->createResourceManager( "musicResMgr", true);

        return musicResMgr;
    }

    static void main( int argc, char** argv )
    {
        bool showConsole = false, dumpLog = false;

        try
        {
            // Engine
            sg = Common::getCore( StormGraph_API_Version )->createEngine( "TolClient", argc, argv );
            sg->addFileSystem( "native:" );
            sg->startup();
            sg->startupGraphics();

            // Resource Managers
            Resources resourcesGuard;

            IGraphicsDriver* driver = sg->getGraphicsDriver();

            /*
            sg->addFileSystem( "mox:TolClient/en_GB.mox" );
            sg->addFileSystem( "mox:TolClient/Music.0.mox" );
            sg->addFileSystem( "mox:TolClient/UI.mox" );
            sg->addFileSystem( "mox:System.mox" );
            */

            sg->addStringTable( "TolClient/Localized/Strings.cfx2" );

            // Some settings
            showConsole = sg->getConfigInt( "Dev/showConsole", false );

            // Set-up the Resource Manager
            globalResMgr = sg->createResourceManager("globalResMgr", true); // hack

            /*resMgr->addPath( "" );
            resMgr->addPath( "sg_demo/res/" );

            resMgr->addModelPath( "tolcl/model/char0/" );

            resMgr->addTexturePath( "" );*/
            globalResMgr->addPath( "tolcl/tex/" );
            /*resMgr->addTexturePath( "sg_demo/res/" );*/

            // Set Display Mode
            DisplayMode displayMode;
            sg->getDefaultDisplayMode( &displayMode );

            displayMode.windowTitle = "Tales of Lanthaia";
            driver->setDisplayMode( &displayMode );

            // Set LOD
            LevelOfDetail lod;
            sg->getDefaultLodSettings( &lod );
            driver->setLevelOfDetail( &lod );

            // Go!
            sg->run( new TolClient::InitScene( driver, Vector2<unsigned>( displayMode.width, displayMode.height ) ) );
            //resMgr->releaseUnused();

            //resMgr.release();
        }
        catch ( Exception ex )
        {
            Common::logEvent( "ToLClient", "Exception caught.\n\n"
                    "<b>Function</b>: " + ex.functionName + "\n"
                    "<b>Exception</b>: " + ex.getName() + "\n\n"
                    "<b>Description</b>: " + ex.getDesc() + "\n\n" );

            sg.release();

            Common::displayException( ex, true );

            dumpLog = true;
        }

        sg.release();

        if ( showConsole )
        {
            printf( "--- UNRELEASED RESOURCES: ---\n" );
            Resource::listResources();

            printf( "--- DUMPING EVENT LOG ---\n" );
        }

        if ( showConsole || dumpLog )
        {
            Reference<File> file = File::open( "ToL Client Log.html", "wb" );

            if ( !file )
                //MessageBox( 0, "FAILED TO SAVE GAME EVENT LOG!!!!!!11111oneoneone", 0, MB_ICONERROR );
                {}
            else
                Common::printEventLog( file.detach() );
        }

        if ( showConsole )
        {
            printf( "\n--- DONE ---\n" );
//            getchar();
        }
    }
}

int main( int argc, char** argv )
{
    TolClient::main( argc, argv );
    return 0;
}

