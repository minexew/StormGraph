#pragma once

#include <StormGraph/Engine.hpp>
#include <StormGraph/GraphicsDriver.hpp>
#include <StormGraph/ResourceManager.hpp>
#include <StormGraph/Scene.hpp>

#include <Radiance/Radiance.hpp>
#include <Radiance/EpicStyler.hpp>
#include <Radiance/OnyxStyler.hpp>

namespace SgDemo
{
    using namespace StormGraph;
    using namespace Radiance;

    class DemoScene : public IScene, public Radiance::EventListener
    {
        IGraphicsDriver* driver;
        IResourceManager* resMgr;

        Object<Camera> sceneCamera, uiCamera;

        Reference<IModel> model;
        List<Transform> modelTransforms;

        Object<Styler> styler;
        Object<UI> ui;
        Object<LinearAnimator> blurAnimator, styleHueAnimator;

        Reference<IFont> font;
        Text* hello;

        Vector<unsigned> window;

        Vector2<int> mousePos;
        int leftMouseButton;

        public:
            DemoScene( IGraphicsDriver* driver, IResourceManager* resMgr, const DisplayMode& dm );
            ~DemoScene();

            void load();

            virtual void onKeyState( int16_t key, Key::State state, Unicode::Char character );
            virtual void onMouseMoveTo( const Vector2<int>& mouse );
            virtual void onRadianceEvent( Widget* widget, const String& eventName, void* eventProperties );
            virtual void render();
            virtual void update( double delta );
    };
}
