
#ifdef MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stdlib.h>

#define littl_GZFileStream

#include "../Binding/SGE.hpp"

#include <Helium/Disassembler.hpp>
#include <Helium/Optimizer.hpp>

using namespace li;

static Helium::Script* loadAppCode( const String& appName )
{
    List<String> moduleStorages;
    moduleStorages.add( "" );
    moduleStorages.add( appName + "/" );

    Object<Helium::Script> script;

    try
    {
        //* Try to load a precompiled program
        script = Helium::CodeIO::load( new GZFileStream( appName + "/bin/" + appName ), true );
    }
    catch ( Helium::Variable ex )
    {
        // Something went wrong. The file must have appeared as a valid binary but seems it wasn't. We can't help here.
        puts( " - caught an exception:" );
        ex.print();
        ex.release();
        return 0;
    }

    try
    {
        Object<Helium::Compiler> compiler = new Helium::Compiler();

        if ( !script )
        {
            // If the program is not binary (previous load failed)
            script = compiler->compileFile( appName + "/main.he" );

            Object<Helium::Linker> linker = new Helium::Linker( compiler, moduleStorages );
            linker->link( script );

            Helium::Optimizer::optimizeWithStatistics( script );
            Helium::Disassembler::disassemble( script, new File( appName + ".asm", true ) );
        }
    }
    catch ( Helium::CompileException ex )
    {
        printf( "ERROR\t: Compilation failed:\n    %s - %s\n", ex.name.c_str(), ex.desc.c_str() );
        return 0;
    }

    return script.detach();
}

void run( const char* program )
{
    Object<Helium::Script> script = loadAppCode( program );
    Object<Helium::HeVM> vm;

    if ( !script )
        return;

    Helium::LoadState loadInfo;
    Helium::registerStdAPI( &loadInfo );
    Helium::Platform::registerIO( &loadInfo );
    Helium::SGBinding::registerSGE( &loadInfo );

    try
    {
        vm = new Helium::HeVM( script, &loadInfo );
        vm->run();
    }
    catch ( Helium::Variable ex )
    {
        puts( " - caught an exception:" );
        ex.print();
        ex.release();
    }
    catch ( StormGraph::Exception exception )
    {
#if defined( __li_MSW ) && defined( Storm_Release_Build )
        // In release we've got no console on Fail32
        File* errorLog = new File( "ErrorLog.txt", "wb" );
        errorLog->writeLine( "Application event log:" );
        StormGraph::Engine::printEventLog( errorLog->reference() );
        errorLog->release();
#endif

        exception.print();

#if  !( defined( __li_MSW ) && defined( Storm_Release_Build ) )
        StormGraph::Engine::printEventLog( new Console() );
#endif
    }
    catch ( li::String error )
    {
        printf( "OLD WAI Game Crash: %s\n\n", error.c_str() );
        getchar();
    }

    Helium::releaseStdAPI();
}

#ifndef __li_MSW
int main( int argc, char *argv[] )
{
#ifdef _DEBUG
#ifdef MSC_VER
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    //_CrtSetBreakAlloc( 28527 );
#endif

    printf( "StormGraph DEBUG BUILD\n" );
#endif

    run( argc > 1 ? argv[1] : "sgconf" );

    Helium::Variable::printStatistics();

    return 0;
}
#else
//int/* CALLBACK*/ WinMain( __in HINSTANCE hInstance, __in HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, __in int nCmdShow )
int CALLBACK WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    run( ( lpCmdLine && *lpCmdLine ) ? lpCmdLine : "sgconf" );

    Helium::Variable::printStatistics();

    return 0;
}
#endif
