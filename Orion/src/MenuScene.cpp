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

#include "GameScene.hpp"
#include "MenuScene.hpp"

#include <StormGraph/Engine.hpp>
#include <StormGraph/ResourceManager.hpp>

namespace Orion
{
    MenuScene::MenuScene( IEngine* sg )
            : sg( sg )
    {
        graphicsDriver = sg->getGraphicsDriver();
    }

    MenuScene::~MenuScene()
    {
        sg->unregisterCommandListener( this );
        sg->unregisterEventListener( gui );
    }

    void MenuScene::init()
    {
        resMgr = sg->createResourceManager( "resMgr", true );

        IGuiDriver* guiDriver = sg->getGuiDriver();

        Vector<unsigned> viewport = graphicsDriver->getViewportSize();

        gui = guiDriver->createGui( Vector<>(), viewport );

        /*IPanel* panel = gui->createPanel( Vector<>( 50.0f, viewport.y - 250.0f ), Vector<>( 300.0f, 200.0f ) );
        gui->add( panel );*/

        console = sg->createCommandLine( gui );

        camera = new Camera( Vector<>(), 0.01f, 0.0f, 0.0f );

        const Vector<> skyboxSize( 1000.0f, 1000.0f, 1000.0f );
        CuboidCreationInfo skyboxCreationInfo( skyboxSize, skyboxSize / 2, false, true, false, resMgr->getMaterial( "Orion/Materials/skybox.cfx2", true ), true );
        skybox = graphicsDriver->createCuboid( "skybox", &skyboxCreationInfo );

        sg->registerCommandListener( this );
        sg->registerEventListener( gui );

        sg->command( sg->getVariableValue( "command", false ) );
    }

    bool MenuScene::onCommand( const List<String>& tokens )
    {
        if ( tokens[0] == "local" )
            sg->changeScene( new GameScene( sg ) );
        else
            return false;

        return true;
    }

    void MenuScene::onRender()
    {
        graphicsDriver->set3dMode( 1.0f, 1000.0f );
        graphicsDriver->setCamera( camera );

        skybox->render();

        // but why?
        graphicsDriver->set2dMode( -1.0f, 1.0f );
    }

    void MenuScene::onUpdate( double delta )
    {
        camera->rotateZ( delta * M_PI / 100.0f );
    }
}
