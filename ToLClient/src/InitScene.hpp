#pragma once

#include <StormGraph/GraphicsDriver.hpp>
#include <StormGraph/Scene.hpp>

#include "TitleScene.hpp"
#include "TolClient.hpp"

namespace TolClient
{
    class InitScene : public IScene
    {
        protected:
            IGraphicsDriver* graphicsDriver;

            enum { preloading, fadeout } state;

            Reference<ITexture> texture;
            float angle, progress;

            Object<TitleScenePreloader> preloader;

        private:
            InitScene( const InitScene& );
            const InitScene& operator = ( const InitScene& );

        public:
            InitScene();
            virtual ~InitScene();

            virtual void init() override;
            virtual void uninit() override;

            virtual void onRender() override;
            virtual void onUpdate( double delta ) override;

            void Run();
    };
}
