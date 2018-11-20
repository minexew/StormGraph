/*
    Copyright (c) 2011, 2018 Xeatheran Minexew

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

#include "Internal.hpp"

#include <StormGraph/Engine.hpp>
#include <StormGraph/GeometryFactory.hpp>
#include <StormGraph/GraphicsDriver.hpp>
#include <StormGraph/GuiDriver.hpp>
#include <StormGraph/Image.hpp>
#include <StormGraph/ResourceManager.hpp>
#include <StormGraph/Scene.hpp>
#include <StormGraph/SoundDriver.hpp>

#include <littl/File.hpp>

#ifdef StormGraph_Static_GraphicsDriver
extern "C" StormGraph::IGraphicsDriver* createGraphicsDriver( const char* driverName, StormGraph::IEngine* engine );
#endif

#ifdef StormGraph_Static_GuiDriver
extern "C" StormGraph::IGuiDriver* createGuiDriver( StormGraph::IEngine* engine, const char* driverName );
#endif

namespace StormGraph
{
    template <typename T> class BoolVariable : public IVariable
    {
        protected:
            T value;

            virtual ~BoolVariable() {}

        public:
            BoolVariable( T value ) : value( value ) {}

            virtual String getValue() { return String::formatBool( value ); }
            virtual bool setValue( const String& value ) { this->value = String::toBool( value ); return true; }
    };

    class IntVariable : public IVariable
    {
        protected:
            int value;

            virtual ~IntVariable() {}

        public:
            IntVariable( int value ) : value( value ) {}

            virtual String getValue() { return String::formatInt( value ); }
            virtual bool setValue( const String& value ) { this->value = String::toInt( value ); return true; }
    };

    class NativeConsoleLineOutput : public ILineOutput
    {
        protected:
            virtual ~NativeConsoleLineOutput() {}

        public:
            virtual void writeLine( const char* text )
            {
                puts( text );
            }
    };

    class RenderFlagVariable : public IVariable
    {
        protected:
            IGraphicsDriver* driver;
            RenderFlag flag;

            virtual ~RenderFlagVariable() {}

        public:
            RenderFlagVariable( IGraphicsDriver* driver, RenderFlag flag ) : driver( driver ), flag( flag ) {}

            virtual String getValue() { return 1; }
            virtual bool setValue( const String& value ) { driver->setRenderFlag( flag, value.toBool() ); return true; }
    };

    class StringVariable : public IVariable
    {
        protected:
            String value;

            virtual ~StringVariable() {}

        public:
            StringVariable( const char* value ) : value( value ) {}

            virtual String getValue() { return value; }
            virtual bool setValue( const String& value ) { this->value = value; return true; }
    };

    class Engine : public ICommandListener, public IEngine
    {
        protected:
            String app;
            HashMap<String, Reference<IVariable>> variables;

            Object<IGraphicsDriver> graphicsDriver;
            Object<ISoundDriver> soundDriver;
            Object<IGuiDriver> guiDriver;

            Object<IImageLoader> imageLoader;

            List<RegisteredFsDriver> fsDrivers;
            Reference<IUnionFileSystem> fileSystem;
            Reference<IResourceManager> sharedResourceManager;

            cfx2_Node* configDoc, * driversDoc;
            cfx2::Document stringTable;

            Reference<IScene> scene, sceneReplacement;

            List<ICommandListener*> commandListeners;
            List<IEventListener*> eventListeners;

            Reference<ILineOutput> lineOutput;
            IProfiler* profiler;

            bool running;
            Timer deltaTimer, frameTime;

            Library *driverLibrary, *guiDriverLibrary;

        private:
            Engine( const Engine& );
            const Engine& operator = ( const Engine& );

            void loadGraphicsDriver( const char* name );

        public:
            Engine( const String& app, int argc, char** argv );
            //Engine( const String& app );
            virtual ~Engine();

            //virtual void addEventListener( IEventListener* eventListener );
            virtual bool addFileSystem( const String& fs, bool required ) override;
            virtual void addFileSystemDriver( const char* protocol, IFileSystemDriver* driver ) override;
            void addStringTable( const char* fileName );
            virtual void changeScene( IScene* newScene ) override;

            virtual void command( const String& command ) override;

            virtual IVariable* createBoolVariable( bool value ) { return new BoolVariable<bool>( value ); }
            virtual IVariable* createBoolRefVariable( bool& value ) { return new BoolVariable<bool&>( value ); }
            virtual ICommandLine* createCommandLine( IGui* gui ) override;
            virtual IFileSystem* createFileSystem( const String& fs ) override;
            virtual IVariable* createIntVariable( int value ) override { return new IntVariable( value ); }
            virtual IKeyScanner* createKeyScanner() override;
            virtual IHeightMap* createHeightMap( const Vector2<unsigned>& resolution ) override;
            virtual IOnScreenLog* createOnScreenLog( const ScreenRect& area, IFont* font ) override { return StormGraph::createOnScreenLog( this, area, font ); }
            virtual IProfiler* createProfiler() override;
            virtual IResourceManager* createResourceManager( const char* name, bool addDefaultPath, IFileSystem* fileSystem ) override;
            virtual ISceneGraph* createSceneGraph( const char* name ) override;
            virtual IVariable* createStringVariable( const char* value ) override { return new StringVariable( value ); }
            virtual IUnionFileSystem* createUnionFileSystem() override { return StormGraph::createUnionFileSystem(); }

            virtual void executeFile( const char* fileName, IFileSystem* fileSystem = nullptr ) override;
            virtual void exit() override { running = false; }

            const char* getConfig( const char* path, bool required = true ) override;
            int getConfigInt( const char* path, bool required = true );
            void getDefaultDisplayMode( DisplayMode* displayMode );
            void getDefaultLodSettings( LevelOfDetail* lod );
            String getEngineBuild();
            String getEngineRelease();
            IUnionFileSystem* getFileSystem() { return fileSystem; }
            IGuiDriver* getGuiDriver();
            IGraphicsDriver* getGraphicsDriver() { return graphicsDriver; }
            virtual IImageLoader* getImageLoader() override;
            virtual IResourceManager* getSharedResourceManager() override;
            virtual ISoundDriver* getSoundDriver() override;
            String getString( const char* key );
            virtual String getVariableValue( const char* name, bool required );

            //int isPointVisible( const Vector<float>& point );
            bool isRunning();
        	//int isSphereVisible( const Vector<float>& center, float radius );

            //virtual unsigned listDirectory( const char* path, List<DirEntry>& entries );
            virtual void listFileSystemDrivers( List<RegisteredFsDriver>& drivers );

            cfx2_Node* loadCfx2Asset( const char* fileName, bool required = true, IFileSystem* fileSystem = nullptr );
            //void loadKeyBindings( Array<unsigned short>& values, const char* fileName, const char** bindingNames, unsigned count );
            String loadTextAsset( const char* fileName, bool required = true, IFileSystem* fileSystem = nullptr );

            virtual void onCloseButton();
            virtual bool onCommand( const List<String>& tokens );
            virtual void onFrameBegin() override;
            virtual void onFrameEnd();
            /*virtual void onKeyEvent( unsigned short key, bool pressed, Utf8Char character );
            virtual void onMouseButton( int x, int y, bool right, bool pressed );
            virtual void onMouseMove( int x, int y );
            virtual void onMouseWheel( bool down );*/

            virtual void onKeyState( int16_t key, Key::State state, Unicode::Char character );
            virtual void onMouseMoveTo( const Vector2<int>& mouse );
            virtual void onRender();
            virtual void onViewportResize( const Vector2<unsigned>& dimensions );

            //SeekableInputStream* open( const char* fileName );
            static void printEventLog( OutputStream* output );

            virtual void registerCommandListener( ICommandListener* listener ) override;
            virtual void registerEventListener( IEventListener* eventListener ) override;

            void removeAllFileSystems();
            //virtual void removeEventListener( IEventListener* eventListener );
            void run( IScene* scene );

            virtual void setLineOutput( ILineOutput* lineOutput ) override { this->lineOutput = lineOutput; }
            virtual void setProfiler( IProfiler* profiler ) override { this->profiler = profiler; }

            virtual void setVariable( const char* name, IVariable* variable, bool allowOverride ) override;
            virtual bool setVariableValue( const char* name, const char* value ) override;

            virtual void startup() override;
            virtual void startupGraphics() override;

            virtual void unregisterCommandListener( ICommandListener* listener ) override;
            virtual void unregisterEventListener( IEventListener* eventListener ) override;
            virtual bool unsetVariable( const char* name ) override;
    };

    Engine::Engine( const String& app, int argc, char** argv ) : app( app ), configDoc( 0 ), driversDoc( 0 ), profiler( nullptr ),
            driverLibrary( nullptr ), guiDriverLibrary( nullptr )
    {
        for ( int i = 1; i < argc; i++ )
        {
            String arg = argv[i];

            int colon = arg.findChar( ':' );

            if ( colon > 0 )
                variables.set( arg.left( colon ), new StringVariable( arg.dropLeft( colon + 1 ) ) );
        }

        Common::logEvent( "StormGraph.Engine", "Minexew Games StormGraph Engine " + getEngineBuild() );
        Common::logEvent( "StormGraph.Engine", "Initializing app `" + app + "`" );

        lineOutput = new NativeConsoleLineOutput();

        addFileSystemDriver( "native",      StormGraph::createNativeFileSystemDriver() );
        addFileSystemDriver( "union",       StormGraph::createUnionFileSystemDriver() );
        addFileSystemDriver( "mox",         StormGraph::createMoxFileSystemDriver() );

        fileSystem = createUnionFileSystem();
        SG_assert( fileSystem != nullptr )
    }

    Engine::~Engine()
    {
        // Release current scene, unhook all event listeners
        scene.release();
        eventListeners.clear();
        lineOutput.release();

        // Release shared resources
        sharedResourceManager.release();

        // Release all variables as they may rely on some resources
        variables.clear();

        // Remove FileSystems and release FS drivers
        removeAllFileSystems();

        iterate ( fsDrivers )
            delete fsDrivers.current().driver;
        fsDrivers.clear();

        // Release Gui driver
        guiDriver.release();

        // Release graphics driver
        // At this point, ALL resources must have been released
        // (except for those owned by graphics driver itself)

        if ( graphicsDriver )
        {
            graphicsDriver->unload();
            graphicsDriver.release();
        }

        // Clean up
        cfx2_release_node( configDoc );
        cfx2_release_node( driversDoc );
    }

    void Engine::addStringTable( const char* fileName )
    {
        stringTable.merge( loadCfx2Asset( fileName ) );
    }

    bool Engine::addFileSystem( const String& fs, bool required )
    {
        Reference<IFileSystem> newFs = createFileSystem( fs );

        if ( newFs == nullptr )
        {
            if ( required )
                throw Exception( "StormGraph.Engine.addFileSystem", "FsOpenError", "Failed to open virtual filesystem `" + fs + "`. Invalid filename or protocol." );

            return false;
        }
        else
        {
            fileSystem->add( newFs.detach() );
            return true;
        }
    }

    void Engine::addFileSystemDriver( const char* protocol, IFileSystemDriver* driver )
    {
#ifdef li_GCC4
        fsDrivers.add( RegisteredFsDriver { protocol, driver } );
#else
        RegisteredFsDriver rfs = { protocol, driver };
        fsDrivers.add( ( RegisteredFsDriver&& ) rfs );
#endif
    }

    void Engine::changeScene( IScene* newScene )
    {
        sceneReplacement = newScene;

        if ( scene == nullptr )
            onFrameBegin();
    }

    void Engine::command( const String& command )
    {
        if ( command.beginsWith( ';' ) )
            return;

        List<String> tokens;

        command.parse( tokens, ' ' );

        if ( tokens.isEmpty() )
            return;

        iterate ( tokens )
            if ( tokens.current().beginsWith( '$' ) )
                tokens.current() = getVariableValue( tokens.current().dropLeftPart( 1 ), false );

        onCommand( tokens );
    }

    ICommandLine* Engine::createCommandLine( IGui* gui )
    {
        return StormGraph::createCommandLine( this, gui );
    }

    IFileSystem* Engine::createFileSystem( const String& fs )
    {
        int colon = fs.findChar( ':' );

        String protocol, name;

        protocol = ( colon < 0 ) ? fs : fs.leftPart( colon );
        name = ( colon < 0 ) ? "" : fs.dropLeftPart( colon + 1 );

        iterate ( fsDrivers )
            if ( fsDrivers.current().protocol == protocol )
                return fsDrivers.current().driver->createFileSystem( name );

        return nullptr;
    }

    IHeightMap* Engine::createHeightMap( const Vector2<unsigned>& resolution )
    {
        return StormGraph::createHeightMap( this, resolution );
    }

    IKeyScanner* Engine::createKeyScanner()
    {
        return StormGraph::createKeyScanner( this );
    }

    IProfiler* Engine::createProfiler()
    {
        return StormGraph::createProfiler( this );
    }

    IResourceManager* Engine::createResourceManager( const char* name, bool addDefaultPath, IFileSystem* fileSystem )
    {
        if ( fileSystem == nullptr )
            fileSystem = this->fileSystem->reference();

        return StormGraph::createResourceManager( this, name, addDefaultPath, fileSystem );
    }

    ISceneGraph* Engine::createSceneGraph( const char* name )
    {
        return StormGraph::createSceneGraph( this, name );
    }

    void Engine::executeFile( const char* fileName, IFileSystem* fileSystem )
    {
        if ( fileSystem == nullptr )
            fileSystem = this->fileSystem;

        Reference<InputStream> file = fileSystem->openInput( fileName );

        while ( isReadable( file ) )
            command( file->readLine() );
    }

    const char* Engine::getConfig( const char* path, bool required )
    {
        const char* value = cfx2_query_value( configDoc, path );

        if ( !value && required )
            throw Exception( "StormGraph.Engine.getConfig", "NodeQueryError",
                    ( String )"Failed to find configuration key `cfx2://" + app + "/Client.cfx2/" + path + "`.\n\nApplication configuration might be corrupted." );

        return value;
    }

    int Engine::getConfigInt( const char* path, bool required )
    {
        const char* value = getConfig( path, required );

        if ( !value )
            return 0;

        return strtol( value, 0, 0 );
    }

    void Engine::getDefaultDisplayMode( DisplayMode* displayMode )
    {
        /*displayMode->width = getConfigInt( "StormGraph/width" );
        displayMode->height = getConfigInt( "StormGraph/height" );
        displayMode->fullscreen = getConfigInt( "StormGraph/fullscreen" ) > 0;

        displayMode->vsync = getConfigInt( "StormGraph/vsync" ) > 0;
        displayMode->multisamplingLevel = getConfigInt( "StormGraph/multisample" );*/

        Vector2<unsigned> resolution( getVariableValue( "app.resolution", true ) );
        displayMode->width = resolution.x;
        displayMode->height = resolution.y;
        displayMode->fullscreen = String::toBool( getVariableValue( "app.fullscreen", false ) );

        displayMode->vsync = String::toBool( getVariableValue( "app.vsync", false ) );
        displayMode->multisamplingLevel = String::toInt( getVariableValue( "app.multisample", false ) );

        //displayMode->resizable = String::toBool( getVariableValue( "app.resizableWindow", false ) );
    }

    void Engine::getDefaultLodSettings( LevelOfDetail* lod )
    {
        lod->textureLodLevel = getConfigInt( "StormGraph/textureLod", false );
    }

    String Engine::getEngineBuild()
    {
        return StormGraph_Version " " StormGraph_Target;
    }

    String Engine::getEngineRelease()
    {
        return "StormGraph " + getEngineBuild();
    }

    IGuiDriver* Engine::getGuiDriver()
    {
        if ( guiDriver == nullptr )
        {
#ifdef StormGraph_Static_GuiDriver
            guiDriver = createGuiDriver( this, "" );
#else
            guiDriverLibrary = Common::getModule( "Gui", true );

            GuiDriverProvider provider = guiDriverLibrary->getEntry<GuiDriverProvider>( "createGuiDriver" );

            if ( provider == nullptr )
                throw Exception( "StormGraph.Engine.getGuiDriver", "EntryPointNotFound", "Failed to load GUI module" );

            guiDriver = provider( this, "" );
#endif

            if ( guiDriver == nullptr )
                throw Exception( "StormGraph.Engine.getGuiDriver", "DriverConfigurationError", "Failed to create a GUI driver instance" );
        }

        return guiDriver;
    }

    IImageLoader* Engine::getImageLoader()
    {
        if ( imageLoader == nullptr )
            imageLoader = createImageLoader( this );

        return imageLoader;
    }

    IResourceManager* Engine::getSharedResourceManager()
    {
        if ( sharedResourceManager == nullptr )
            sharedResourceManager = createResourceManager( "sharedResMgr", true, nullptr );

        return sharedResourceManager;
    }

    ISoundDriver* Engine::getSoundDriver()
    {
        if ( soundDriver == nullptr )
            soundDriver = createSoundDriver( this );

        return soundDriver;
    }

    String Engine::getString( const char* key )
    {
        return stringTable.findChild( key ).getText();
    }

    String Engine::getVariableValue( const char* name, bool required )
    {
        IVariable* variable = variables.get( name );

        if ( variable == nullptr )
        {
            if ( required )
                throw Exception( "StormGraph.Engine.getVariableValue", "VariableUndefined", ( String ) "Undefined variable `" + name + "`" );
            else
            {
                lineOutput->writeLine( ( String ) "\\rWarning: Undefined variable `" + name + "`" );
                return String();
            }
        }

        return variable->getValue();
    }

    /*int Engine::isPointVisible( const Vector<float>& point )
    {
        return frustum.pointInFrustum( point );
    }*/

    bool Engine::isRunning()
    {
        return running && scene != nullptr;
    }

    /*int Engine::isSphereVisible( const Vector<float>& center, float radius )
    {
        return frustum.sphereInFrustum( center, radius );
    }*/

    void Engine::listFileSystemDrivers( List<RegisteredFsDriver>& drivers )
    {
        iterate ( fsDrivers )
            drivers.add( fsDrivers.current() );
    }

    cfx2_Node* Engine::loadCfx2Asset( const char* fileName, bool required, IFileSystem* fileSystem )
    {
        String document = loadTextAsset( fileName, required, fileSystem );

        cfx2_Node* doc = 0;

        if ( document.isEmpty() )
            return 0;

        int error = cfx2_read_from_string( document, &doc );

        if ( !doc && required )
            throw Exception( "StormGraph.Engine.loadCfx2Asset", "DocumentLoadError",
                    ( String )"Failed to parse `" + fileName + "`: " + cfx2_get_error_desc( error ) );

        return doc;
    }

    void Engine::loadGraphicsDriver( const char* name )
    {
        cfx2_Node* driverNode = cfx2_find_child( driversDoc, name );

        if ( !driverNode )
            throw Exception( "StormGraph.Engine.loadGraphicsDriver", "DriverConfigurationError",
                    ( String ) "Unknown graphics driver `" + name + "`." );

        cfx2_Attrib* libraryName = cfx2_find_attrib( driverNode, "library" );

        if ( !libraryName || !libraryName->value )
            throw Exception( "StormGraph.Engine.loadGraphicsDriver", "DriverConfigurationError",
                    ( String ) "Incorrectly configured graphics driver `" + name + "`." );

        driverLibrary = Common::getModule( ( String ) "Driver." + libraryName->value, true );

        GraphicsDriverProvider provider = driverLibrary->getEntry<GraphicsDriverProvider>( "createGraphicsDriver" );

        if ( provider == nullptr )
            throw Exception( "StormGraph.Engine.loadGraphicsDriver", "EntryPointNotFound", "Failed to load Graphics Driver module" );

        graphicsDriver = provider( name, this );

        if ( graphicsDriver == nullptr )
            throw Exception( "StormGraph.Engine.loadGraphicsDriver", "DriverConfigurationError", "Failed to create a Graphics Driver instance" );

        Common::logEvent( "StormGraph.Engine", "Successfully loaded graphics driver " + File::formatFileName( name ) );

        setVariable( "render.culling",          new RenderFlagVariable( graphicsDriver, RenderFlag::culling ),          true );
        setVariable( "render.wireframe",        new RenderFlagVariable( graphicsDriver, RenderFlag::wireframe ),        true );
    }

    /*void Engine::loadKeyBindings( Array<unsigned short>& values, const char* fileName, const char** bindingNames, unsigned count )
    {
        cfx2_Node* doc = loadCfx2Asset( fileName );

        for ( unsigned i = 0; i < count; i++ )
            values[i] = getKey( cfx2_query_value( doc, ( String )"Controls/" + bindingNames[i] ) );

        cfx2_release_node( doc );
    }*/

    String Engine::loadTextAsset( const char* fileName, bool required, IFileSystem* fileSystem )
    {
        if ( fileSystem == nullptr )
            fileSystem = this->fileSystem;

        Reference<SeekableInputStream> input = fileSystem->openInput( fileName );

        if ( !isReadable( input ) )
        {
            if ( required )
                throw Exception( "StormGraph.Engine.loadTextAsset", "AssetOpenError",
                        ( String ) "Failed to open `" + fileName + "` for reading. The file probably doesn't exist at all." );
            else
                return String();
        }

        size_t size = ( size_t ) input->getSize();

        Array<char> buffer( size + 1 );
        input->read( buffer.getPtr(), size );
        return String( buffer.getPtr() );
    }

    void Engine::onCloseButton()
    {
        //if ( scene )
        //    running = !scene->closeButtonAction();
        running = false;
    }

    bool Engine::onCommand( const List<String>& tokens )
    {
        if ( tokens[0] == "engine.listResources" )
        {
            Resource::listResources();

            return true;
        }

        if ( tokens[0] == "engine.listVariables" )
        {
            iterate2 ( i, variables )
                lineOutput->writeLine( ( *i ).key + " \t" + i->getValue() );

            variables.printStatistics();

            return true;
        }

        if ( tokens[0] == "say" )
        {
            for ( size_t i = 1; i < tokens.getLength(); i++ )
                // printf for proper nullptr handling
                printf( "%s\n", tokens[i].c_str() );

            return true;
        }

        if ( tokens[0] == "set" )
        {
            Reference<IVariable>* var = variables.find( tokens[1] );

            if ( var != nullptr )
                (*var)->setValue( tokens[2] );
            else
                variables.set( String( tokens[1] ), new StringVariable( tokens[2] ) );

            return true;
        }

        iterate2 ( i, commandListeners )
            if ( i->onCommand( tokens ) )
                return true;

        if ( lineOutput != nullptr )
            lineOutput->writeLine( "\\rWarning: unrecognized command `" + tokens[0] + "`" );

        return false;
    }

    void Engine::onFrameBegin()
    {
        if ( sceneReplacement != nullptr )
        {
            if ( scene != nullptr )
            {
                scene->uninit();
                unregisterEventListener( scene );
            }

            scene = sceneReplacement.detach();

            if ( scene != nullptr )
            {
                registerEventListener( scene );
                scene->init();
            }
        }

        if ( scene != nullptr )
            scene->onFrameBegin();
    }

    void Engine::onFrameEnd()
    {
        /*if ( deltaTimer.isStarted() )
            timeDelta = deltaTimer.getMicros();
        else
            timeDelta = 0.0;*/

        auto dt = deltaTimer.getMicros() / 1000000.0;
        deltaTimer.start();

        if ( deltaTimer.isStarted() )
            iterate ( eventListeners )
                eventListeners.current()->onUpdate( dt );

#ifdef _DEBUG
        static int counter = 0;

        if ( ++counter == 100 )
        {
            if ( frameTime.isStarted() )
            {
                DisplayMode update;
                memset( &update, 0, sizeof( update ) );

                unsigned fps = ( unsigned )round( 100000000.0 / frameTime.getMicros() );
                unsigned timePerFrame = ( unsigned )( frameTime.getMicros() / 100000 );

                String windowTitle = app + ": " + fps + " fps (~" + timePerFrame + " ms/frame)";

                update.windowTitle = windowTitle.c_str();
                update.changeWindowTitle = true;

                graphicsDriver->changeDisplayMode( &update );
            }

            frameTime.start();

            counter = 0;
        }
#endif

        if ( soundDriver )
            soundDriver->onFrameEnd();
    }

    void Engine::onKeyState( int16_t key, Key::State state, Unicode::Char character )
    {
        iterate ( eventListeners )
            eventListeners.current()->onKeyState( key, state, character );
    }

    void Engine::onMouseMoveTo( const Vector2<int>& mouse )
    {
        iterate ( eventListeners )
            eventListeners.current()->onMouseMoveTo( mouse );
    }

    void Engine::onRender()
    {
        reverse_iterate ( eventListeners )
            eventListeners.current()->onRender();
    }

    void Engine::onViewportResize( const Vector2<unsigned>& dimensions )
    {
        if ( scene != nullptr )
            scene->onViewportResize( dimensions );
    }

    void Engine::registerCommandListener( ICommandListener* listener )
    {
        commandListeners.add( listener );
    }

    void Engine::registerEventListener( IEventListener* eventListener )
    {
        // Prevent duplicate entries
        eventListeners.removeItem( eventListener );

        // The new event listener has the highest priority
        // (except for rendering)
        eventListeners.insert( eventListener, 0 );
    }

    void Engine::removeAllFileSystems()
    {
        fileSystem->clear();
    }

    void Engine::run( IScene* defaultScene )
    {
        changeScene( defaultScene );

        running = true;

        graphicsDriver->runMainLoop( this );

        scene->uninit();
        unregisterEventListener( scene );

        scene.release();
    }

    /*void Engine::setCursor( bool enable, const char* fileName )
    {
        throw StormGraph::Exception( "StormGraph.Engine", "setCursor", "NotImplemented", "StormGraph.Engine.setCursor() is not implemented" );
    }*/

    void Engine::setVariable( const char* name, IVariable* variable, bool allowOverride )
    {
        if ( allowOverride )
        {
            Reference<IVariable>* overrideVar = variables.find( name );

            if ( overrideVar != nullptr )
            {
                variable->setValue( ( *overrideVar )->getValue() );
                *overrideVar = variable;
                return;
            }
        }

        variables.set( ( String ) name, ( IVariable*&& ) variable );
    }

    bool Engine::setVariableValue( const char* name, const char* value )
    {
        IVariable* variable = variables.get( name );

        if ( variable != nullptr )
            return variable->setValue( value );

        return false;
    }

    void Engine::startup()
    {
        Common::logEvent( "StormGraph.Engine", ( String ) "Engine startup (" + String::formatInt( fileSystem->getNumFileSystems() ) + " file systems)" );

        configDoc = loadCfx2Asset( app + "/Client.cfx2", false );
        driversDoc = loadCfx2Asset( StormGraph_Bin "/Drivers.cfx2" );

        Reference<InputStream> clientVarList = fileSystem->openInput( app + "/ClientVars.txt" );

        while ( isReadable( clientVarList ) )
        {
            String line = clientVarList->readLine();

            if ( line.isEmpty() || line.beginsWith( ';' ) )
                continue;

            int tab = line.findChar( '\t' );

            if ( tab < 0 )
                continue;

            int valueBegin = line.findDifferentChar( '\t', tab );

            if ( valueBegin < 0 )
                continue;

            variables.set( line.left( tab ), new StringVariable( line.dropLeftPart( valueBegin ) ) );
        }
    }

    void Engine::startupGraphics()
    {
#ifdef StormGraph_Static_GraphicsDriver
        graphicsDriver = createGraphicsDriver( StormGraph_Static_GraphicsDriver, this );
#else
        loadGraphicsDriver( getVariableValue( "display.driver", true ) );
#endif

        graphicsDriver->startup();
    }

    void Engine::unregisterCommandListener( ICommandListener* listener )
    {
        commandListeners.removeItem( listener );
    }

    void Engine::unregisterEventListener( IEventListener* eventListener )
    {
        eventListeners.removeItem( eventListener );
    }

    bool Engine::unsetVariable( const char* name )
    {
        Reference<IVariable>* variable = variables.find( name );

        if ( variable != nullptr )
        {
            variable->release();
            return true;
        }

        return false;
    }

    IEngine* createEngine( const char* app, int argc, char** argv )
    //IEngine* createEngine( const char* app )
    {
        return new Engine( app, argc, argv );
        //return new Engine( app );
    }
}
