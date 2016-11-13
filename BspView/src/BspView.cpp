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

#define _USE_MATH_DEFINES

#include <StormGraph/Abstract.hpp>
#include <StormGraph/CommandLine.hpp>
#include <StormGraph/Engine.hpp>
#include <StormGraph/GuiDriver.hpp>
#include <StormGraph/GraphicsDriver.hpp>
#include <StormGraph/ResourceManager.hpp>
#include <StormGraph/Scene.hpp>
#include <StormGraph/SceneGraph.hpp>

#include <littl/File.hpp>

using namespace StormGraph;

namespace BspView
{
    class BspViewScene : public ICommandListener, public IGuiEventListener, public IScene
    {
        protected:
            IEngine* sg;
            IGraphicsDriver* driver;

            String fileName, fileSystem;
            //Reference<IModel> bsp;
            Reference<ISceneGraph> sceneGraph;
            Reference<IResourceManager> bspResMgr;

            Object<IGui> gui;
            Object<ICommandLine> commandLine;

            Camera camera;
            Reference<IFont> font;

            Reference<IPopupMenu> menu;

            int16_t mouseWheelUp, mouseWheelDown, left, right, up, down;
            bool leftPressed = false, rightPressed = false, upPressed = false, downPressed = false;
            Vector2<int> mousePos, mouseDragFrom;

        public:
            BspViewScene( IEngine* sg, const char* fileSystem, const char* fileName );
            virtual ~BspViewScene();

            void loadModel( const String& fileName );

            // StormGraph.ICommandListener
            virtual bool onCommand( const List<String>& tokens );

            // StormGraph.IGuiEventListener
            virtual void onGuiCommand( const CommandEvent& event );

            // StormGraph.IScene
            virtual void init() override;
            virtual void onKeyState( int16_t key, Key::State state, Unicode::Char character ) override;
            virtual void onMouseMoveTo( const Vector2<int>& mouse ) override;
            virtual void onRender() override;
            virtual void onUpdate( double delta ) override;
			virtual void uninit() override {}
    };

    BspViewScene::BspViewScene( IEngine* sg, const char* fileSystem, const char* fileName )
            : sg( sg ), fileName( fileName ), fileSystem( fileSystem )
    {
    }

    void BspViewScene::init()
    {
        driver = sg->getGraphicsDriver();

        IGuiDriver* guiDriver = sg->getGuiDriver();
        gui = guiDriver->createGui( Vector<>(), driver->getViewportSize() );

        IWindow* window = gui->createWindow( Vector<float>( 100.0f, 100.0f ), Vector<float>( 320.0f, 200.0f ), "Main Frame With a Really, REALLY long title herpa derpa derp herp" );
        //gui->add( panel );

        ITableLayout* layout = gui->createTableLayout( 2 );
        layout->setRowGrowable( 0, true );
        layout->setColumnGrowable( 1, true );
        window->add( layout );

        IStaticText* staticText = gui->createStaticText( Vector<>(), Vector<>(), "Hello, world!\n(i'm soo expanding)" );
        staticText->setExpand( true );
        layout->add( staticText );

        staticText = gui->createStaticText( Vector<>(), Vector<>(), "Not growing\narea around expands tho" );
        layout->add( staticText );

        ITextBox* textBox = gui->createTextBox( Vector<>(), Vector<>() );
        textBox->setExpand( true );
        layout->add( textBox );

        textBox = gui->createTextBox( Vector<>(), Vector<>( 150.0f, 40.0f ) );
        textBox->setExpand( true );
        layout->add( textBox );

        IButton* button = gui->createButton( "Menu" );
        button->setEventListener( this );
        button->setExpand( true );
        button->setName( "menu" );
        layout->add( button );

        button = gui->createButton( "Cancel" );
        layout->add( button );

        window->showModal();

        menu = gui->createPopupMenu();
        menu->setMinSize( Vector<>( 100, 0 ) );
        menu->addCommand( "Hello World!" );
        menu->addToggle( "happy", true );
        menu->addToggle( "sad", false );
        menu->addCommand( "Ok!" );

        //sg->addEventListener( gui->reference() );

        commandLine = sg->createCommandLine( gui );

        sg->registerCommandListener( this );

        if ( !fileSystem.isEmpty() )
        {
            Reference<IFileSystem> packageFs = sg->createFileSystem( fileSystem );

            if ( packageFs == nullptr )
                throw Exception( "BspViewScene.BspViewScene", "FileSystemOpenError", "Failed to open file system " + File::formatFileName( fileSystem ) );

            Reference<IUnionFileSystem> bspUnion = dynamic_cast<IUnionFileSystem*>( sg->createFileSystem( "union" ) );
            bspUnion->add( packageFs.detach() );
            bspUnion->add( sg->getFileSystem()->reference() );

            bspResMgr = sg->createResourceManager( "bspResMgr", true, bspUnion->reference() );
        }
        else
            bspResMgr = sg->createResourceManager( "bspResMgr", true, sg->getFileSystem()->reference() );

        bspResMgr->setLoadFlag( LoadFlag::useDynamicLighting, true );
        bspResMgr->setLoadFlag( LoadFlag::useShadowMapping, true );

        loadModel( fileName );

        Reference<IResourceManager> resMgr = sg->createResourceManager( "resMgr", true, sg->getFileSystem()->reference() );

        font = resMgr->getFont( "Common/Fonts/DejaVuSans.ttf", 12, IFont::bold );

        mouseWheelUp = driver->getKey( "Mouse Wheel Up" );
        mouseWheelDown = driver->getKey( "Mouse Wheel Down" );
        up = driver->getKey( "Up" );
        down = driver->getKey( "Down" );
        left = driver->getKey( "Left" );
        right = driver->getKey( "Right" );

        driver->setSceneAmbient( Colour() );
    }

    BspViewScene::~BspViewScene()
    {
        sg->unregisterCommandListener( this );
    }

    void BspViewScene::loadModel( const String& fileName )
    {
        //sceneGraph = sg->createSceneGraph( "sceneGraph" );
        sceneGraph = bspResMgr->loadSceneGraph( "_SCENEGRAPH", true );
        sceneGraph->addModel( bspResMgr->getModel( fileName ), Vector<>(), Vector<>() );

        //bsp = bspResMgr->getModel( fileName );
        camera = Camera( Vector<>(), 100, M_PI * 1.5f, M_PI_4 );

        this->fileName = fileName;
    }

    bool BspViewScene::onCommand( const List<String>& tokens )
    {
        if ( tokens[0] == "model" )
        {
            loadModel( tokens[1] );
            //sg->setVariable( "modelName", tokens[1] );

            return true;
        }

        return false;
    }

    void BspViewScene::onGuiCommand( const CommandEvent& event )
    {
        if ( String( "menu" ) == event.widget->getName() )
            menu->show( event.mousePos );
    }

    void BspViewScene::onKeyState( int16_t key, Key::State state, Unicode::Char character )
    {
        if ( key == mouseWheelUp )
            camera.zoom( -5.0f );
        else if ( key == mouseWheelDown )
            camera.zoom( 5.0f );
        else if ( key == up )
            upPressed = ( state == Key::pressed );
        else if ( key == down )
            downPressed = ( state == Key::pressed );
        else if ( key == left )
            leftPressed = ( state == Key::pressed );
        else if ( key == right )
            rightPressed = ( state == Key::pressed );
    }

    void BspViewScene::onMouseMoveTo( const Vector2<int>& mouse )
    {
        mousePos = mouse;
    }

    void BspViewScene::onRender()
    {
        sceneGraph->prerender();

        driver->set3dMode( 0.5f, 2000.0f );
        driver->setCamera( &camera );
        //bsp->render();
        sceneGraph->render();

        driver->set2dMode( -1.0f, 1.0f );

        font->renderString( 8.0f, 8.0f, "Rendering map: \\s" + fileName + "\\w in \\l" + fileSystem + " \\o(using " + sg->getEngineRelease() + ")", Colour::white(), IFont::left | IFont::top );
        driver->drawStats();
    }

    void BspViewScene::onUpdate( double delta )
    {
        if ( upPressed )
            camera.rotateXY( delta * M_PI / 2 );
        else if ( downPressed )
            camera.rotateXY( -delta * M_PI / 2 );

        if ( leftPressed )
            camera.rotateZ( -delta * M_PI / 2 );
        else if ( rightPressed )
            camera.rotateZ( delta * M_PI / 2 );
    }

    extern "C" int main( int argc, char** argv )
    {
        try
        {
            Object<IEngine> sg = Common::getCore( StormGraph_API_Version )->createEngine( "BspView", argc, argv );

            sg->addFileSystem( "native" );

            sg->setVariable( "fileSystem", sg->createStringVariable( "mox:Presets/Maps/Sandbox.mox" ), true );
            sg->setVariable( "fileName", sg->createStringVariable( "_BSP" ), true );

            sg->startup();

            IGraphicsDriver* driver = sg->getGraphicsDriver();

            DisplayMode displayMode;
            displayMode.windowTitle = "BspView";
            sg->getDefaultDisplayMode( &displayMode );
            driver->setDisplayMode( &displayMode );

            LevelOfDetail lod;
            sg->getDefaultLodSettings( &lod );
            driver->setLevelOfDetail( &lod );

            const String fileSystem = sg->getVariableValue( "fileSystem", true ).replaceAll( '\\', '/' );
            const String fileName = sg->getVariableValue( "fileName", true ).replaceAll( '\\', '/' );

            sg->run( new BspView::BspViewScene( sg, fileSystem, fileName ) );
        }
        catch ( Exception& ex )
        {
            Common::displayException( ex, false );
        }

        return 0;
    }

    class Application : public IApplication
    {
        public:
            virtual int main( int argc, char** argv )
            {
                return BspView::main( argc, argv );
            }
    };

    Sg_implementInterfaceProvider( name )
    {
        if ( strcmp( name, "StormGraph.IApplication" ) == 0 )
        {
            StormGraph::IApplication* object = new Application();

            return object;
        }
        else
            return nullptr;
    }
}
