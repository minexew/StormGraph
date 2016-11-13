
#define _WIN32_WINNT 0x0502

#include <StormGraph/Common.hpp>
#include <StormGraph/Server.hpp>

#include <littl.hpp>
#include <littl/Library.hpp>

#include <stdio.h>
#include <windows.h>

#ifdef _DEBUG
#include <crtdbg.h>
#endif

using namespace li;

int main( int argc, char** argv )
{
#ifdef _DEBUG
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    Map<String, String> args = Map<String, String>::create( argc, argv );

    String game = args.get( "game" );

    if ( game.isEmpty() )
        game = "Duel";

    bool dedicated = args.get( "dedicated" ).toBool();

	String path = game + "/bin/";
	/*
#ifndef WIN64
#ifdef __GNUC__
    String path = game + "/bin/";
    SetDllDirectoryA( "bin" );
#else
    String path = game + "/vc10-Win32/";
    SetDllDirectoryA( "vc10-Win32" );
#endif
#else
#ifdef __GNUC__
    String path = game + "/gcc4-win64/";
    SetDllDirectoryA( "gcc4-win64" );
#else
    String path = game + "/vc10-Win64/";
    SetDllDirectoryA( "vc10-Win64" );
#endif
#endif
	*/
    String libraryName = !dedicated ? path + "Client.dll" : path + "Server.dll";

    Object<Library> library = Library::open( libraryName );

    if ( library == nullptr )
        return -1;

    StormGraph::InterfaceProvider createInterface = library->getEntry<StormGraph::InterfaceProvider>( "createInterface" );

    if ( createInterface == nullptr )
        return -1;

    if ( !dedicated )
    {
        Object<StormGraph::IApplication> application = reinterpret_cast<StormGraph::IApplication*>( createInterface( "StormGraph.IApplication" ) );

        if ( application == nullptr )
            return -1;

        return application->main( argc, argv );
    }
    else
    {
        Object<StormGraph::IDedicatedServer> server = reinterpret_cast<StormGraph::IDedicatedServer*>( createInterface( "StormGraph.IDedicatedServer" ) );

        if ( server == nullptr )
            return -1;

        if ( server->initDedicatedServer( argc, argv ) )
            server->runDedicatedServer();

        return 0;
    }
}
