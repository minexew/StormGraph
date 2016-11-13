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

#include <StormGraph/Common.hpp>
#include <StormGraph/Core.hpp>

#include <littl/File.hpp>
#include <littl/Library.hpp>

#if defined( _DEBUG ) && defined( _MSC_VER )
#include <crtdbg.h>
#endif

#include <signal.h>

namespace StormGraph
{
    struct LoadedModule
    {
        String name;
        Library* library;
    };

    struct LoggedEvent
    {
        String className, event;
        time_t time;

        String getTime() const
        {
            struct tm * timeInfo;
            char buffer[100];

            timeInfo = localtime( &time );
            strftime( buffer, sizeof( buffer ), "%X", timeInfo );

            return buffer;
        }
    };

    static List<LoadedModule> loadedModules;
    static List<LoggedEvent> loggedEvents;

    static Object<ICore> core;

    static bool abortOnError = false;

    static void logEventDefault( const char* className, const char* event )
    {
        LoggedEvent ev = { className, event };
        time( &ev.time );

        //printf( "%s:\n    %s\n\n", className, event );

        loggedEvents.add( ev );
    }

    static Common::LogEventCallback logEventCallback = logEventDefault;

    static Library* getLibrary( const char* fileName )
    {
        iterate2 ( i, loadedModules )
        {
            LoadedModule& module = i;

            if ( module.name == fileName )
                return module.library;
        }

        Library* library = Library::open( fileName );

        if ( library != nullptr )
        {
            LoadedModule lm = { fileName, library };
            loadedModules.add( lm );
        }

        return library;
    }

    void Common::assertionFail( const String& sourceUnit, int line, const String& functionName, const String& assertion, const String& desc )
    {
        String description = "in " + File::formatFileName( sourceUnit ) + ":" + line + "\n\nfailed assertion `" + assertion + "`";
        throw Exception( functionName, "AssertionFailed", desc.isEmpty() ? description : description + "\n(" + desc + ")" );
    }

    const char* Common::getBinDirectory()
    {
		return "bin";
		/*
#ifndef WIN64
#ifdef __GNUC__
        return "bin";
#else
        return "vc10-Win32";
#endif
#else
#ifdef __GNUC__
        return "gcc4-win64";
#else
        return "vc10-Win64";
#endif
#endif
		*/
    }

    void Common::displayException( const Exception& ex, bool recoverable )
    {
        String text = recoverable ? "An error" : "A fatal error";

        text += " occurred in StormGraph engine.\n\n"
                "Function: " + ex.functionName + "\n"
                "Exception: " + ex.name + "\n\n"
                "Description: " + ex.description;

        if ( !recoverable )
            text += "\n\nThis error is not recoverable.";

#ifdef __li_MSW
        MessageBoxA( 0, text, "OH NOES! Y DIS ALWAYS HAPPEN 2 ME!", MB_ICONERROR | MB_OK );
#else
        printf( "%s\n\n", text.c_str() );
#endif
    }

    ICore* Common::getCore( const char* apiVersion )
    {
        if ( core == nullptr )
#ifdef StormGraph_Static_Core
            core = createCore( apiVersion );
#else
        {
            Library* coreLibrary = getModule( "Core", true );

            CoreProvider provider = coreLibrary->getEntry<CoreProvider>( "createCore" );

            if ( provider == nullptr )
                throw Exception( "StormGraph.Common.getCore", "EntryPointNotFound", ( String ) "Failed to load Core module" );

            core = provider( apiVersion );
        }
#endif

        if ( core == nullptr )
            throw Exception( "StormGraph.Common.getCore", "CoreCreationFailed", ( String ) "Incompatible API version" );

        return core;
    }

    Library* Common::getAppModule( const char* app, const char* name, bool required )
    {
        String fileName = ( String ) app + "/" + getBinDirectory() + "/" + StormGraph_Library_Prefix + name + StormGraph_Library_Suffix;

        Library* library = getLibrary( fileName );

        if ( library == nullptr && required )
            throw Exception( "StormGraph.Common.getAppModule", "LibraryLoadError", "Unable to load " + File::formatFileName( fileName ) );

        return library;
    }

    Library* Common::getModule( const char* name, bool required )
    {
        String fileName = ( String ) getBinDirectory() + "/" + StormGraph_Library_Prefix + name + StormGraph_Library_Suffix;

        Library* library = getLibrary( fileName );

        if ( library == nullptr && required )
            throw Exception( "StormGraph.Common.getModule", "LibraryLoadError", "Unable to load " + File::formatFileName( fileName ) );

        return library;
    }

    void Common::logEvent( const char* className, const char* event )
    {
        logEventCallback( className, event );
    }

    void Common::printEventLog( OutputStream* output )
    {
        SG_assert( output != nullptr )

        time_t t = time( 0 );
        struct tm * timeInfo;
        char buffer[100];

        timeInfo = localtime( &t );
        strftime( buffer, sizeof( buffer ), "%Y/%m/%d %X %Z", timeInfo );

        output->writeLine( "<html><head><style>body, td { font-family: Verdana; font-size: 10pt } td { padding: 7pt; vertical-align: top }</style></head>" );
        output->writeLine( "<body>" );
        output->writeLine( "<h2 style=\"color: #002244\">StormGraph Engine Event Log</h2>" );
        output->writeLine( ( String ) "saved on " + buffer + "<br />" );

        output->writeLine( "<hr />" );
        output->writeLine( "<table style=\"border: none\">" );

        iterate ( loggedEvents )
        {
            output->writeLine( "<tr>" );
            output->writeLine( ( String ) "<td><b>" + loggedEvents.current().getTime() + "</b></td>" );
            output->writeLine( "<td><b style=\"color: #003366\">"
                    + loggedEvents.current().className + "</b></td>" );
            output->writeLine( "<td style=\"border-bottom: #AAA solid 1px\">" + loggedEvents.current().event.replaceAll( "\n", "<br />" ) + "</td>" );
            output->writeLine( "</tr>" );
        }

        output->writeLine( "</table>" );
        output->writeLine( "</body></html>" );
    }

    void Common::releaseModules()
    {
        core.release();

        iterate2 ( i, loadedModules )
        {
            LoadedModule& module = i;

            delete module.library;
        }

        loadedModules.clear();
    }

    void Common::setAbortOnError( bool abort )
    {
        abortOnError = abort;
    }

    void Common::setLogEventCallback( LogEventCallback callback )
    {
        logEventCallback = ( callback != nullptr ) ? callback : logEventDefault;
    }
}

#ifdef __li_MSW
BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
    switch ( fdwReason )
    {
        case DLL_PROCESS_ATTACH:
/*#ifdef _DEBUG
            _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF );
#endif*/
            break;

        case DLL_PROCESS_DETACH:
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;
    }

    return TRUE;
}
#endif
