#define _USE_MATH_DEFINES

#include "DemoScene.hpp"

namespace SgDemo
{
    ILight* labLight, * light[2];

    List<Transform> l1T, lightTransforms, empty;

    DemoScene::DemoScene( IGraphicsDriver* driver, IResourceManager* resMgr, const DisplayMode& dm )
            : driver( driver ), resMgr( resMgr ), window( dm.width, dm.height )
    {
        float vfov = 45.0f * M_PI / 180.0f;

        // Cameras
        sceneCamera = new Camera( Vector<float>( 3.0f, 0.0f, 0.0f ), Vector<float>( 0.0f, 0.0f, 0.0f ), Vector<float>( 0.0f, 0.0f, 1.0f ) );
        uiCamera = new Camera( Vector<float>( window.x / 2, window.y / 2, window.y / 2 / tan( vfov / 2.0f ) ), Vector<float>( window.x / 2, window.y / 2 ), Vector<float>( 0.0f, -1.0f ) );

        driver->setSceneAmbient( Colour( 0.1f, 0.0f, 0.0f ) );

        // Model
        model = resMgr->getModel( "human_0_sny.ms3d" );

        modelTransforms.add( Transform( Transform::translate, Vector<float>( 0.0f, 0.0f, -1.0f ) ) );
        modelTransforms.add( Transform( Transform::scale, Vector<float>( 1.0f, 1.0f, 1.0f ) ) );
        modelTransforms.add( Transform( Transform::rotate, Vector<float>( 0.0f, 1.0f, 0.0f ), 0.0f ) );
        modelTransforms.add( Transform( Transform::rotate, Vector<float>( 0.0f, 0.0f, 1.0f ), 0.0f ) );

        //styler = new EpicStyler( driver, resMgr->reference() );
        styler = new OnyxStyler( driver );

        ui = new UI( styler, Vector<float>(), window );
            Window* wnd = new Window( styler, Vector<float>( 80.0f, 80.0f ), Vector<float>( 464.0f, 288.0f, 30.0f ), "Hello World!" );
            //panel = new Panel( styler, Vector<float>( 50.0f, 50.0f ), Vector<float>( 400.0f, 240.0f, 30.0f ) );
                Button* button = new Button( styler, Vector<float>( 16.0f, 64.0f ), Vector<float>( 112.0f, 36.0f, 5.0f ), "\\B\\w\\ OK \\0!" );
                wnd->add( button );

                Input* input = new Input( styler, Vector<float>( 16.0f, 16.0f ), Vector<float>( 240.0f, 36.0f ) );
                wnd->add( input );
            ui->add( wnd );

        light[0] = driver->createLight( ILight::line, Vector<float>( window.x / 2, 0.0f, 0.0f ), Colour(), Colour( 1.0f, 1.0f, 1.0f ), window.y * 2 );
        light[1] = driver->createLight( ILight::directional, Vector<float>( 0.0f, 0.0f, -1.0f ), Colour( 0.6f, 0.6f, 0.6f ), Colour( 1.0f, 1.0f, 1.0f ), 0.0f );

        lightTransforms.add( Transform( Transform::translate, Vector<float>( window.x / 4, window.y, 20.0f ) ) );

        // Scene (3D) light
        labLight = driver->createLight( ILight::positional, Vector<float>(), Colour( 0.4f, 0.2f, 0.1f ), Colour( 0.4f, 1.4f, 2.0f ), 2.0f );
        l1T.add( Transform( Transform::translate, Vector<float>( 1.0f, 0.0f, 0.0f ) ) );

        resMgr->finalizePreloads();
        resMgr->releaseUnused();

        /*EpicPanelStyle* epicStyle = ( Radiance::EpicPanelStyle* ) panel->getStyle()->getInterface( "Radiance.EpicPanelStyle" );

        if ( epicStyle )
        {
            blurAnimator = new LinearAnimator( epicStyle, EpicPanelStyle::blur );
            blurAnimator->setName( "blurAnimator" );
            blurAnimator->setOnAnimationEnd( this );
            blurAnimator->animate( 0.0f, 0.0f, 0.0f );
        }*/

        OnyxWidgetStyle* wndStyle = ( Radiance::OnyxWidgetStyle* ) wnd->getStyle()->getInterface( "Radiance.OnyxWidgetStyle" );

        if ( wndStyle )
        {
            wndStyle->setTexture( resMgr->getTexture( "radiance.png" ) );

            styleHueAnimator = new LinearAnimator( wndStyle, OnyxWidgetStyle::hue );
            styleHueAnimator->setName( "styleHueAnimator" );
            styleHueAnimator->setOnAnimationEnd( this );
            //styleHueAnimator->animate( 0.0f, 0.0f, 0.0f );
        }

        font = resMgr->getFont( "gravitat.ttf", 12, IFont::normal );
        hello = font->layoutText( "\\#579\\ StormGraph.Font\\w finally ported as interface\\#795 StormGraph.IFont\\w and implementation\\#975 OpenGlDriver.Font\\w!",
                Colour(), IFont::left | IFont::top );

        leftMouseButton = driver->getKey( "Left Mouse Button" );
    }

    DemoScene::~DemoScene()
    {
        font->releaseText( hello );
    }

    void DemoScene::onKeyState( int16_t key, Key::State state, Unicode::Char character )
    {
        if ( character )
            ui->onKeyState( character, state );

        if ( key == leftMouseButton )
        {
            if ( state == Key::pressed )
                ui->mouseDown( mousePos );
            else if ( state == Key::released )
                ui->mouseUp( mousePos );
        }
    }

    void DemoScene::onMouseMoveTo( const Vector2<int>& mouse )
    {
        mousePos = mouse;

        ui->mouseMove( mouse );
    }

#if 0
    void DemoScene::mouseWheel( bool down )
    {
        resMgr->listResources();

        /*if ( down )
            modelTransforms[1].vector /= 1.2f;
        else
            modelTransforms[1].vector *= 1.2f;*/
    }
#endif

    void DemoScene::onRadianceEvent( Widget* widget, const String& eventName, void* eventProperties )
    {
        if ( widget->getName() == "styleHueAnimator" && eventName == "animationEnd" )
            styleHueAnimator->animate( 0.0f, 1.0f, 0.1f );
    }

    void DemoScene::render()
    {
        driver->set3dMode( 0.1f, window.x );
        driver->enableDepthTesting();

        driver->clearLights();
        driver->setCamera( sceneCamera );

        labLight->render( l1T );
        model->render( modelTransforms );

        driver->clearLights();
        driver->setCamera( uiCamera );

        light[0]->render( lightTransforms );
        light[1]->render( lightTransforms );
        ui->render();

        font->renderText( 20.0f, 20.0f, hello );
    }

    void DemoScene::update( double delta )
    {
        if ( blurAnimator )
            blurAnimator->update( delta );

        if ( styleHueAnimator )
            styleHueAnimator->update( delta );

        ui->update( delta );

        modelTransforms[3].angle -= delta * M_PI_2;
    }
}
