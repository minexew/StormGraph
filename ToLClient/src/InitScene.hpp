
#pragma once

#include <StormGraph/GraphicsDriver.hpp>
#include <StormGraph/Scene.hpp>

#include "TitleScene.hpp"
#include "TolClient.hpp"

namespace TolClient
{
    class InitScene : public Scene
    {
        protected:
            GraphicsDriver* driver;
            Vector2<> windowSize;

            enum { preloading, fadeout } state;

            Reference<IMaterial> material;
            Reference<IModel> model;
            List<Transform> transforms;
            double progress;

            Object<TitleScenePreloader> preloader;

        private:
            InitScene( const InitScene& );
            const InitScene& operator = ( const InitScene& );

        public:
            InitScene( GraphicsDriver* driver, const Vector2<unsigned>& windowSize );
            virtual ~InitScene();

            virtual void render();
            virtual void update( double delta );
    };
}
