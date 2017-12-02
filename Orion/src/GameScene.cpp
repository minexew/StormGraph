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

#include <StormGraph/Engine.hpp>
#include <StormGraph/ResourceManager.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace Orion
{
    static const ShipProperties props
    {
        M_PI / 40.0f, M_PI / 20.0f,
        M_PI / 80.0f, M_PI / 30.0f,
        M_PI / 100.0f, M_PI / 9.0f,
    };

    Ship::Ship( IResourceManager* resMgr )
            : pitchDir( 0.0f ), rollDir( 0.0f )
    {
        this->properties = &props;
        model = resMgr->getModel( "Orion/Models/spaceship.ms3d" );
    }

    Vector<> Ship::transform( const Vector<>& vec )
    {
        //return vec;

        glm::vec4 t = transformation * glm::vec4( vec.x, vec.y, vec.z, vec.w );

        return Vector<>( t.x, t.y, t.z );
    }

    void Ship::render()
    {
        transforms[0].operation = Transform::matrix;
        transforms[0].transformation = transformation;

        model->render( transforms, 1 );
    }

    void Ship::update( double delta, bool up, bool down, bool left, bool right )
    {
        if ( down )
            pitchDir = maximum<float>( pitchDir - delta * properties->forceDown, -properties->maxDown );
        else if ( up )
            pitchDir = minimum<float>( pitchDir + delta * properties->forceUp, properties->maxUp );
        else if ( pitchDir != 0.0f && fabs( pitchDir ) > 0.01f )
            pitchDir /= 1.01f;
        else
            pitchDir = 0.0f;

        if ( left )
            rollDir = minimum<float>( rollDir + delta * properties->rollForce, properties->maxRoll );
        else if ( right )
            rollDir = maximum<float>( rollDir - delta * properties->rollForce, -properties->maxRoll );
        else if ( rollDir != 0.0f && fabs( rollDir ) > 0.01f )
            rollDir /= 1.01f;
        else
            rollDir = 0.0f;

        if ( pitchDir != 0.0f )
            transformation = glm::rotate( transformation, float( -delta * pitchDir * 180.0f / M_PI ), glm::vec3( 1.0f, 0.0f, 0.0f ) );

        if ( rollDir != 0.0f )
            transformation = glm::rotate( transformation, float( -delta * rollDir * 180.0f / M_PI ), glm::vec3( 0.0f, 1.0f, 0.0f ) );

        const float speed = delta * 10.0f;
        transformation = glm::translate( transformation, glm::vec3( /*transformation * */glm::vec4( 0.0f, -1.0f, 0.0f, 0.0f ) ) * speed );
    }

    GameScene::GameScene( IEngine* sg )
            : sg( sg )
    {
        graphicsDriver = sg->getGraphicsDriver();
    }

    GameScene::~GameScene()
    {
        sg->unregisterEventListener( gui );
    }

    void GameScene::init()
    {
        resMgr = sg->createResourceManager( "resMgr", true );

        IGuiDriver* guiDriver = sg->getGuiDriver();

        Vector<unsigned> viewport = graphicsDriver->getViewportSize();

        gui = guiDriver->createGui( Vector<>(), viewport );
        console = sg->createCommandLine( gui );

        keyScanner = sg->createKeyScanner();
        up = keyScanner->registerKey( "W" );
        down = keyScanner->registerKey( "S" );
        left = keyScanner->registerKey( "A" );
        right = keyScanner->registerKey( "D" );

        camera = new Camera( Vector<>( 0.0f, 0.0f, 10.0f ), 30.0f, M_PI * 1.5f, M_PI / 6.0f );
        light = graphicsDriver->createDirectionalLight( Vector<float>( 0.0f, 0.0f, -1.0f ).normalize(), Colour(), Colour( 0.05f, 0.1f, 0.4f ), Colour::grey( 0.0f ) );

        const Vector<> skyboxSize( 10000.0f, 10000.0f, 10000.0f );
        CuboidCreationInfo skyboxCreationInfo( skyboxSize, skyboxSize / 2, false, true, false, resMgr->getMaterial( "Orion/Materials/skybox.cfx2", true ), true );
        skybox = graphicsDriver->createCuboid( "skybox", &skyboxCreationInfo );

        playerShip = new Ship( resMgr );

        graphicsDriver->setSceneAmbient( Colour() );

        sg->registerEventListener( gui );
    }

    /*void GameScene::onKeyState( int16_t key, Key::State state, Unicode::Char character )
    {
    }*/

    void GameScene::onRender()
    {
        Vector2<unsigned> ws = graphicsDriver->getWindowSize();

        graphicsDriver->setViewport( Vector2<int>(), ws, ws );
        graphicsDriver->set3dMode( 1.0f, 10000.0f );

        camera->setEyePos( playerShip->transform( Vector<>( 0.0f, 40.0f, 15.0f, 1.0f ) ) );
        camera->setCenterPos( playerShip->transform( Vector<>( 0.0f, 0.0f, 15.0f, 1.0f ) ) );
        camera->setUpVector( playerShip->transform( Vector<>( 0.0f, 0.0f, 1.0f, 0.0f ) ) );

        graphicsDriver->setCamera( camera );
        renderScene();

        /*graphicsDriver->setViewport( Vector2<int>(), ws / 4, ws );
        graphicsDriver->set3dMode( 1.0f, 1000.0f );

        camera->setEyePos( Vector<>( 500.0f, 0.0f, 15.0f ) );
        camera->setCenterPos( Vector<>( 0.0f, 0.0f, 15.0f ) );
        camera->setUpVector( Vector<>( 0.0f, 0.0f, 1.0f ) );

        graphicsDriver->setCamera( camera );
        renderScene();*/

        graphicsDriver->set2dMode( -1.0f, 1.0f );

        graphicsDriver->drawRectangle( Vector<>( 20.0f, 20.0f ), Vector2<>( 100.0f, 100.0f ), Colour(), nullptr );

        Vector2<> worldSpace = playerShip->transform( Vector<>( 0.0f, 0.0f, 0.0f, 1.0f ) ).getXy();
        graphicsDriver->drawRectangle( Vector<>( 20.0f, 20.0f ) + ( worldSpace + Vector2<>( 500.0f, 500.0f ) ) * 0.1f, Vector2<>( 4.0f, 4.0f ), Colour::white(), nullptr );

        graphicsDriver->drawStats();
    }

    void GameScene::onUpdate( double delta )
    {
        playerShip->update( delta, keyScanner->getKeyState( down ), keyScanner->getKeyState( up ),
                keyScanner->getKeyState( left ), keyScanner->getKeyState( right ) );
    }

    void GameScene::renderScene()
    {
        light->render();

        playerShip->render();

        skybox->render();
    }
}
