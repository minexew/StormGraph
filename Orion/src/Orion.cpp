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

#include "MenuScene.hpp"

#include <StormGraph/Engine.hpp>

namespace Orion
{
    extern "C" int main( int argc, char** argv )
    {
        try
        {
            Object<IEngine> sg = Common::getCore( StormGraph_API_Version )->createEngine( "Orion", argc, argv );
            sg->addFileSystem( "native" );
            sg->executeFile("Orion/startup.txt");
            sg->startup();
            sg->startupGraphics();

            IGraphicsDriver* graphicsDriver = sg->getGraphicsDriver();

            DisplayMode displayMode;
            displayMode.windowTitle = "Orion";
            sg->getDefaultDisplayMode( &displayMode );
            graphicsDriver->setDisplayMode( &displayMode );

            LevelOfDetail lod;
            sg->getDefaultLodSettings( &lod );
            graphicsDriver->setLevelOfDetail( &lod );

            sg->run( new MenuScene( sg ) );
        }
        catch ( Exception& ex )
        {
            Common::displayException( ex, false );
        }

        return 0;
    }

    class Application : public IApplication
    {
        public:
            virtual int main( int argc, char** argv )
            {
                return Orion::main( argc, argv );
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
