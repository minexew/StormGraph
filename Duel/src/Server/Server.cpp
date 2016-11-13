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

#include "GameServer.hpp"
#include "Server.hpp"

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
        printf( "Minexew Games Storm Pre Alpha Host\n" );
        printf( "All rights reserved..\n\n" );

#ifdef _DEBUG
        _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF );
        //_CrtSetBreakAlloc( 1564 );

        printf( "##########################################\n" );
        printf( "  Leak Detection Enabled\n" );
        printf( "##########################################\n" );
#endif

        try
        {
            /*Object<IEngine> sg = Common::getCore( StormGraph_API_Version )->createEngine( "Duel", argc, argv );
            
            sg->addFileSystem( "native" );
            sg->addFileSystem( "mox:Duel.mox", false );
            sg->addFileSystem( "mox:Common.mox", false );

            sg->setVariable( "command",         sg->createStringVariable( "" ),             true );
            sg->setVariable( "map_name",        sg->createStringVariable( "uc_0" ),         true );

            sg->startup();

            IGraphicsDriver* graphicsDriver = sg->getGraphicsDriver();

            DisplayMode displayMode;
            displayMode.windowTitle = "Duel DEV";
            sg->getDefaultDisplayMode( &displayMode );
            graphicsDriver->setDisplayMode( &displayMode );

            LevelOfDetail lod;
            sg->getDefaultLodSettings( &lod );
            graphicsDriver->setLevelOfDetail( &lod );

            sg->run( new TitleScene( sg ) );*/
        }
        catch ( Exception& ex )
        {
            Common::displayException( ex, false );
        }

        //
        {
            Reference<File> eventLogDump = File::open( "DuelServerEventLog.html", true );

            if ( eventLogDump != nullptr )
                Common::printEventLog( eventLogDump );
        }

        Common::releaseModules();
        return 0;
    }

    class GameServerProvider : public IGameServerProvider
    {
        public:
            virtual ~GameServerProvider() {}

            virtual IGameServer* createGameServer( IEngine* engine ) override
            {
                return new GameServer( engine );
            }
    };

    Sg_implementInterfaceProvider( name )
    {
        if ( strcmp( name, "Duel.IGameServerProvider" ) == 0 )
        {
            Duel::IGameServerProvider* object = new GameServerProvider();

            return object;
        }
        else if ( strcmp( name, "StormGraph.IDedicatedServer" ) == 0 )
        {
            StormGraph::IDedicatedServer* object = new DedicatedGameServer();

            return object;
        }
        else
            return nullptr;
    }
}