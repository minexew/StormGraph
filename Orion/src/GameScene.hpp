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

#include <StormGraph/CommandLine.hpp>
#include <StormGraph/GraphicsDriver.hpp>
#include <StormGraph/GuiDriver.hpp>
#include <StormGraph/KeyScanner.hpp>
#include <StormGraph/Scene.hpp>

namespace Orion
{
    using namespace StormGraph;

    struct ShipProperties
    {
        float forceUp, maxUp;
        float forceDown, maxDown;
        float rollForce, maxRoll;
    };

    class Ship
    {
        public:
        const ShipProperties* properties;

        Reference<IModel> model;
        Transform transforms[4];

        float pitchDir, rollDir;

        glm::mat4 transformation;

        public:
            Ship( IResourceManager* resMgr );

            Vector<> transform( const Vector<>& vec );

            void render();

            void update( double delta, bool up, bool down, bool left, bool right );
    };

    class GameScene : public IScene
    {
        IEngine* sg;
        IGraphicsDriver* graphicsDriver;

        Reference<IResourceManager> resMgr;

        Object<IGui> gui;
        Object<ICommandLine> console;
        Object<IKeyScanner> keyScanner;

        Object<Camera> camera;
        Object<ILight> light;
        Reference<IModel> skybox;

        Object<Ship> playerShip;

        size_t up, down, left, right;

        public:
            GameScene( IEngine* sg );
            virtual ~GameScene();

            virtual void init() override;
            void uninit() override {}
            //virtual void onKeyState( int16_t key, Key::State state, Unicode::Char character );
            virtual void onRender() override;
            virtual void onUpdate( double delta ) override;

            void renderScene();
    };
}
