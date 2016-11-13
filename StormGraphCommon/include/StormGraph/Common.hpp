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

#pragma once

// For __li_MSW etc.
#include <littl/Base.hpp>

#if defined( __GNUC__ )
#define StormGraph_ABI "gcc4"
#elif _MSC_VER >= 1600
#pragma warning ( disable : 4251 4482 )
#define StormGraph_ABI "vc10"
#else
#error Unknown Compiler/Platform
#endif

#ifdef __li_MSW
#define StormGraph_Library_Prefix ""
#define StormGraph_Library_Suffix ".dll"

#ifdef WIN64
#define StormGraph_Bin StormGraph_ABI "-win64"
#define StormGraph_Platform "amd64"
#else
#define StormGraph_Bin "bin"
#define StormGraph_Platform "x86"
#endif

#endif

#if defined( _DEBUG )
#define StormGraph_BuildTarget "DEBUG"
#else
#define StormGraph_BuildTarget "RELEASE"
#endif

#define StormGraph_Target StormGraph_BuildTarget " " StormGraph_ABI "-" StormGraph_Platform

#if !defined( DOXYGEN ) && defined( li_MSW ) && defined( StormGraph_Build_Common_DLL )
#define SgClass class __declspec( dllexport )
#define SgStruct struct __declspec( dllexport )
#elif !defined( DOXYGEN ) && defined( li_MSW ) && !defined( StormGraph_Static_Common )
#define SgClass class __declspec( dllimport )
#define SgStruct struct __declspec( dllimport )
#endif

#ifndef SgClass
#define SgClass class
#define SgStruct struct
#endif

//#define SG_assert( expr_, class_, method_ ) if ( !( expr_ ) ) StormGraph::Assertion::assertionFail( __FILE__, __LINE__, class_, method_, #expr_, "" );
#define SG_assert( expr_ ) if ( !( expr_ ) ) StormGraph::Common::assertionFail( __FILE__, __LINE__, li_functionName, #expr_, "" );
#define SG_assert2( expr_, class_, method_, desc_ ) if ( !( expr_ ) ) StormGraph::Common::assertionFail( __FILE__, __LINE__, class_, method_, #expr_, desc_ );
#define SG_assert3( expr_, function_ ) if ( !( expr_ ) ) StormGraph::Common::assertionFail( __FILE__, __LINE__, function_, #expr_, "" );
#define SG_assert4( expr_, desc_ ) if ( !( expr_ ) ) StormGraph::Common::assertionFail( __FILE__, __LINE__, li_functionName, #expr_, desc_ );

#define StormGraph_API_Version "StormGraph_T_10"
#define StormGraph_Version "TRUNK"

#include <StormGraph/Vector.hpp>

#include <littl/cfx2.hpp>
#include <littl/BaseIO.hpp>
#include <littl/Exception.hpp>
#include <littl/Library.hpp>
#include <littl/String.hpp>

namespace StormGraph
{
    using namespace li;

    class ICore;
    class IEngine;
    class IResource;

    enum
    {
        VAR_UTF8 = 1,
        VAR_INT = 2,
        VAR_FLOAT = 4,
        VAR_TYPEMASK = 0xff,
        VAR_LOCK = 1<<30,
        VAR_LOCKTYPE = 1<<31
    };

    typedef void* ( *InterfaceProvider )( const char* name );

#define Sg_implementInterfaceProvider( name_ ) extern "C" __declspec( dllexport ) void* createInterface( const char* name_ )

    class IApplication
    {
        public:
            virtual ~IApplication() {}

            virtual int main( int argc, char** argv ) = 0;
    };

    SgClass Common
    {
        public:
            typedef void ( *LogEventCallback )( const char* className, const char* event );

        public:
            // Common functions
            li_noreturn( static void assertionFail( const String& sourceUnit, int line, const String& functionName, const String& assertion, const String& desc ) );
            static void displayException( const Exception& ex, bool recoverable );
            static Vector<float> getAttribVector( cfx2::Node node, const char* name );
            static const char* getBinDirectory();
            static Library* getAppModule( const char* app, const char* name, bool required );
            static Library* getModule( const char* name, bool required );
            static void releaseModules();
            static void setAbortOnError( bool abort );

            // StormGraph Core
            static ICore* getCore( const char* apiVersion );

            // Logging
            static void logEvent( const char* className, const char* event );
            static void printEventLog( OutputStream* output );
            static void setLogEventCallback( LogEventCallback callback );
    };

    SgClass Resource
    {
        public:
            static void add( IResource* resource );
            static void listResources();
            static void remove( IResource* resource );
    };

    SgClass Timer
    {
        uint64_t startTicks, pauseTicks;
        bool paused, started;

        public:
            Timer();

            uint64_t getMicros() const;
            static uint64_t getRelativeMicroseconds();
            bool isStarted() const { return started; }
            bool isPaused() const { return paused; }
            void pause();
            void reset();
            void start();
            void stop();
            void unpause();
    };
}
