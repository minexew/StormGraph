
#include "InitScene.hpp"
#include "TolClient.hpp"

#include <StormGraph/Engine.hpp>
#include <StormGraph/ResourceManager.hpp>

#include <StormGraph/GuiDriver.hpp>

namespace TolClient
{
    InitScene::InitScene()
            : state( preloading ), angle( 0.0f ), progress( 0.0f )
    {
        graphicsDriver = engine->getGraphicsDriver();
    }

    InitScene::~InitScene()
    {
        if ( preloader )
            preloader->waitFor();
    }

    void InitScene::init()
    {
        IResourceManager* bootstrapResMgr = Resources::getBootstrapResMgr();

        texture = bootstrapResMgr->getTexture( "TolClient/UI/LoadPiece.png" );

        graphicsDriver->setClearColour( Colour::white() );
        graphicsDriver->set2dMode( -1.0f, 1.0f );

        // Do not start it before everything is loaded here
        // We're not ready for that yet.
        preloader = new TitleScenePreloader();
        preloader->start();
    }

    void InitScene::onRender()
    {
        if ( state == preloading )
            graphicsDriver->draw2dCenteredRotated( texture, 1.0f, angle, Colour( 0.0f, 0.0f, 0.0f, 0.7f ) );
        else
            graphicsDriver->drawRectangle( Vector<>(), graphicsDriver->getViewportSize(), Colour( 0.0f, 0.0f, 0.0f, progress ), nullptr );
    }

    void InitScene::onUpdate( double delta )
    {
        if ( state == preloading )
        {
            progress += delta;

            while ( progress > 0.08f )
            {
                angle -= M_PI / 4;
                progress -= 0.08f;
            }

            if ( !preloader->isRunning() )
            {
                if ( preloader->failed )
                    StormGraph::Exception::rethrow( preloader->exception );

                state = fadeout;
                progress = 0.0f;
            }
        }
        else
        {
            progress = minimum<float>( progress + delta / 0.2f, 1.0f );

            if ( progress > 0.98f )
            {
                engine->changeScene( new TitleScene( preloader.detach() ) );
                return;
            }
        }
    }

    void InitScene::Run()
    {
        Event_t* event;

        while ( true )
        {
            r->beginFrame();

            // EVENTS
            while ( ( event = r->getEvent() ) != nullptr )
            {
                switch ( event->type )
                {
                    case EV_VKEY:
                        if ( event->vkey.vk == V_CLOSE && event->vkey.triggered() )
                            return;
                        break;
                }
            }

            // UPDATE
            double delta = sys->Update();

            onUpdate( delta );

            // RENDER
            onRender();

            r->endFrame();
        }
    }

    void InitScene::uninit()
    {
        texture.release();
    }
}
