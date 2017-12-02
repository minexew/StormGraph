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
#include <StormGraph/Scene.hpp>

namespace Orion
{
    using namespace StormGraph;

    class MenuScene : public ICommandListener, public IScene
    {
        IEngine* sg;
        IGraphicsDriver* graphicsDriver;

        Reference<IResourceManager> resMgr;

        Object<IGui> gui;
        Object<ICommandLine> console;

        Object<Camera> camera;
        Reference<IModel> skybox;

        public:
            li_ReferencedClass_override( MenuScene )

            MenuScene( IEngine* sg );
            virtual ~MenuScene();

            virtual void init() override;
            void uninit() override {}
            virtual bool onCommand( const List<String>& tokens ) override;
            virtual void onRender() override;
            virtual void onUpdate( double delta ) override;
    };
}
