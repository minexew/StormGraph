
#include "InitScene.hpp"
#include "TolClient.hpp"

#include <StormGraph/Core.hpp>
#include <StormGraph/Engine.hpp>
#include <StormGraph/ResourceManager.hpp>

#include <React/React.hpp>

#include <littl/File.hpp>

using namespace StormGraph;
using namespace StormRender;

namespace TolClient
{
    IResourceManager* Resources::bootstrapResMgr, * Resources::uiResMgr, * Resources::musicResMgr;
    IEngine* engine = nullptr;

    IR* r = nullptr;
    ISys* sys = nullptr;

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
            bootstrapResMgr = engine->createResourceManager( "bootstrapResMgr", true );

        return bootstrapResMgr;
    }

    IResourceManager* Resources::getUiResMgr( bool create )
    {
        if ( !uiResMgr && create )
            uiResMgr = engine->createResourceManager( "uiResMgr", true );

        return uiResMgr;
    }

    IResourceManager* Resources::getMusicResMgr( bool create )
    {
        if ( !musicResMgr && create )
            musicResMgr = engine->createResourceManager( "musicResMgr", true );

        return musicResMgr;
    }

    static void main( int argc, char** argv )
    {
        bool showConsole = false, dumpLog = false;

        try
        {
#ifdef _DEBUG
            Common::setAbortOnError( true );
#endif

            // Engine
            engine = Common::getCore( StormGraph_API_Version )->createEngine( "TolClient", argc, argv );
            sys = Common::getCore( StormGraph_API_Version )->createSys( "TolClient", argc, argv );

            engine->addFileSystem( "native:" );

            engine->addFileSystem( "mox:TolClient/en_GB.mox", false );
            engine->addFileSystem( "mox:TolClient/Music.0.mox", false );
            engine->addFileSystem( "mox:TolClient/UI.mox", false );
            engine->addFileSystem( "mox:System.mox", false );

            engine->executeFile( "TolClient/startup.txt" );
            engine->executeFile( "TolClient/startup_" StormGraph_BuildTarget ".txt" );

            engine->startup();
            engine->startupGraphics();

            engine->addStringTable( "TolClient/Localized/Strings.cfx2" );

            // Resource Managers
            Resources resourcesGuard;

            IGraphicsDriver* graphicsDriver = engine->getGraphicsDriver();
            r = graphicsDriver->getR();

            // Some settings
            showConsole = engine->getConfigInt( "Dev/showConsole", false );

            sys->ExecList( "TolClient/config.txt" );

            // Set Display Mode
            r->StartVideo();

            /*DisplayMode displayMode;
            memset( &displayMode, 0, sizeof( displayMode ) );

            displayMode.windowTitle = "Tales of Lanthaia";
            displayMode.resizable = true;
            engine->getDefaultDisplayMode( &displayMode );

            graphicsDriver->setDisplayMode( &displayMode );*/

            // Set LOD
            /*LevelOfDetail lod;
            engine->getDefaultLodSettings( &lod );
            graphicsDriver->setLevelOfDetail( &lod );*/

            // Go!

            InitScene* initScene = new InitScene();

            initScene->init();
            initScene->Run();

            delete initScene;

            //engine->run( new TolClient::InitScene() );
            //resMgr->releaseUnused();

            //resMgr.release();

            li::destroy( engine );
        }
        catch ( Exception ex )
        {
            li::destroy( engine );

            Common::logEvent( "ToLClient", "Exception caught.\n\n"
                    "<b>Function</b>: " + ex.functionName + "\n"
                    "<b>Exception</b>: " + ex.getName() + "\n\n"
                    "<b>Description</b>: " + ex.getDesc() + "\n\n" );

            Common::displayException( ex, true );

            dumpLog = true;
        }

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
                MessageBox( 0, "FAILED TO SAVE GAME EVENT LOG!!!!!!11111oneoneone", 0, MB_ICONERROR );
            else
                Common::printEventLog( file.detach() );
        }

        if ( showConsole )
        {
            printf( "\n--- DONE ---\n" );
            getchar();
        }
    }
}

int main( int argc, char** argv )
{
    TolClient::main( argc, argv );
    return 0;
}

