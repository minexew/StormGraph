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

#include "Internal.hpp"

#include <StormGraph/Core.hpp>

#if defined( _DEBUG ) && defined( _MSC_VER )
#include <crtdbg.h>
#endif

namespace StormGraph
{
    class Core : public ICore
    {
        public:
            virtual IEngine* createEngine( const char* app, int argc, char** argv );
    };

    IEngine* Core::createEngine( const char* app, int argc, char** argv )
    {
        return StormGraph::createEngine( app, argc, argv );
    }

#ifdef StormGraph_Static_Core
    ICore* createCore( const char* apiVersion )
#else
    extern "C" StormGraph_Library_Export ICore* createCore( const char* apiVersion )
#endif
    {
        if ( strcmp( apiVersion, StormGraph_API_Version ) != 0 )
            return nullptr;

        return new Core();
    }
}

#ifdef li_MSW
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
