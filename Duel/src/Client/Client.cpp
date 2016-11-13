/*
    Copyright (c) 2011 Xeatheran Minexew

    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

#include "TitleScene.hpp"

#include <StormGraph/Engine.hpp>
#include <StormGraph/Profiler.hpp>

#include <littl/File.hpp>

#ifdef _DEBUG
#include <crtdbg.h>
#endif

namespace Duel
{
    extern "C" int main( int argc, char** argv )
    {
        printf( "Minexew Games Storm Pre Alpha Client (" StormGraph_BuildTarget ")\n" );
        printf( "All rights reserved..\n\n" );

#ifdef _DEBUG
        _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF );
        //_CrtSetBreakAlloc( 11103 );

        printf( "##########################################\n" );
        printf( "  Leak Detection Enabled\n" );
        printf( "##########################################\n" );
#endif

        try
        {
            Object<IEngine> engine = Common::getCore( StormGraph_API_Version )->createEngine( "Duel", argc, argv );
            
            engine->addFileSystem( "native" );
            engine->addFileSystem( "mox:Duel.mox", false );
            engine->addFileSystem( "mox:Common.mox", false );

            // FIXME: do not use host_port for connect

            engine->setVariable( "hostname",        engine->createStringVariable( "" ),             true );
            //engine->setVariable( "host_port",       engine->createIntVariable( 0xD0E1 ),            true );

            engine->executeFile( "Duel/startup.txt" );
            engine->executeFile( "Duel/startup_" StormGraph_BuildTarget ".txt" );

            engine->startup();
            engine->startupGraphics();

            IGraphicsDriver* graphicsDriver = engine->getGraphicsDriver();

            DisplayMode displayMode;
            memset( &displayMode, 0, sizeof( displayMode ) );

            displayMode.windowTitle = "Storm prealpha";
            //displayMode.resizable = true;
            engine->getDefaultDisplayMode( &displayMode );

            graphicsDriver->setDisplayMode( &displayMode );

            LevelOfDetail lod;
            engine->getDefaultLodSettings( &lod );
            graphicsDriver->setLevelOfDetail( &lod );

            engine->run( new TitleScene( engine ) );
        }
        catch ( Exception& ex )
        {
            Common::displayException( ex, false );
        }

        //
        {
            Reference<File> eventLogDump = File::open( "DuelClientEventLog.html", true );

            if ( eventLogDump != nullptr )
                Common::printEventLog( eventLogDump );
        }

        Common::releaseModules();
        return 0;
    }

    class Application : public IApplication
    {
        public:
            virtual int main( int argc, char** argv )
            {
                return Duel::main( argc, argv );
            }
    };

    Sg_implementInterfaceProvider( name )
    {
        if ( strcmp( name, "StormGraph.IApplication" ) == 0 )
        {
            StormGraph::IApplication* object = new Application();

            return object;
        }
        else
            return nullptr;
    }
}