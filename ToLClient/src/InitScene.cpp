
#include "InitScene.hpp"
#include "TolClient.hpp"

#include <StormGraph/Engine.hpp>
#include <StormGraph/ResourceManager.hpp>

namespace TolClient
{
    InitScene::InitScene( IGraphicsDriver* driver, const Vector2<unsigned>& windowSize )
            : driver( driver ), windowSize( windowSize ), state( preloading ), progress( 0.0f )
    {
        IResourceManager* bootstrapResMgr = Resources::getBootstrapResMgr();
        bootstrapResMgr->addPath( "" );

        Reference<ITexture> texture = bootstrapResMgr->getTexture( "TolClient/UI/LoadPiece.png" );

        PlaneCreationInfo plane;
        plane.dimensions = texture->getDimensions().getXy();
        plane.origin = plane.dimensions / 2;
        plane.uv0 = Vector2<>( 0.0f, 0.0f );
        plane.uv1 = Vector2<>( 1.0f, 1.0f );
        plane.withNormals = false;
        plane.withUvs = true;
        plane.material = driver->createSolidMaterial( 0, Colour( 0.0f, 0.0f, 0.0f, 0.7f ), texture.detach() );
        model = driver->createPlane( 0, &plane );

        transforms.add( Transform( Transform::rotate, Vector<float>( 0.0f, 0.0f, 1.0f ), 0.0f ) );
        transforms.add( Transform( Transform::translate, windowSize / 2 ) );

        driver->setClearColour( Colour::white() );
        driver->set2dMode( -1.0f, 1.0f );

        // Do not start it before everything is loaded here
        // We're not ready for that yet.
        preloader = new TitleScenePreloader();
        preloader->start();
    }

    InitScene::~InitScene()
    {
        if ( preloader )
            preloader->waitFor();
    }

    void InitScene::onRender()
    {
        model->render( transforms );
    }

    void InitScene::onUpdate( double delta )
    {
        if ( state == preloading )
        {
            progress += delta;

            while ( progress > 0.08f )
            {
                transforms[0].angle -= M_PI / 4;
                progress -= 0.08f;
            }

            if ( !preloader->isRunning() )
            {
                if ( preloader->failed )
                    StormGraph::Exception::rethrow( preloader->exception );

                material = driver->createSolidMaterial( 0, Colour( 0.0f, 0.0f, 0.0f, 0.0f ), 0 );

                PlaneCreationInfo plane;

                plane.dimensions = windowSize;
                plane.origin = Vector<float>();
                plane.withNormals = false;
                plane.withUvs = false;
                plane.material = material->reference();

                model = driver->createPlane( 0, &plane );
                transforms.clear();

                state = fadeout;
                progress = 0.0f;
            }
        }
        else
        {
            progress = minimum<float>( progress + delta / 0.2f, 1.0f );

            if ( progress > 0.98f )
            {
                Reference<TitleScene> title = new TitleScene( driver, windowSize, preloader.detach() );
                sg->changeScene( title.detach() );
                return;
            }

            material->setColour( Colour( 0.0f, 0.0f, 0.0f, progress ) );
        }
    }
}
