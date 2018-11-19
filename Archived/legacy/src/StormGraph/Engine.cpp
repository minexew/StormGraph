#define __StormGraph_Engine_cpp__

#include "Internal.hpp"

//#include <ctime>
#include <SDL_image.h>

namespace StormGraph
{
    unsigned numPolysThisFrame;

    struct LoggedEvent
    {
        String className, event;
        time_t time;

        String getTime() const
        {
            struct tm * timeInfo;
            char buffer[100];

            timeInfo = gmtime( &time );
            strftime( buffer, sizeof( buffer ), "%Y.%m.%d %X %Z", timeInfo );

            return buffer;
        }
    };

    struct Key
    {
        const char* name;
        unsigned short key;
    };

    static const Key keys[] =
    {
        { "Delete", SDLK_DELETE },
        { "End", SDLK_END },
        { "Enter", SDLK_RETURN },
        { "Escape", SDLK_ESCAPE },
        { "Home", SDLK_HOME },
        { "Insert", SDLK_INSERT },
        { "Left Shift", SDLK_LSHIFT },
        { "Num Enter", SDLK_KP_ENTER },
        { "Page Down", SDLK_PAGEDOWN },
        { "Page Up", SDLK_PAGEUP },
        { "Space", ' ' },
        { 0 }
    };

    Engine* Engine::instance = 0;
    GraphicsDriver* Engine::driver = 0;

    GlFunctionTable glFs;
    List<LoggedEvent> loggedEvents;

#ifndef StormGraph_No_Helium
    static cfx2_Action iterateControlsCallback( unsigned index, cfx2_Node* child, cfx2_Node* parent, Helium::Variable* object )
    {
        object->setMember( child->name, Helium::Variable( Engine::getKey( child->text ) ) );
        return cfx2_continue;
    }
#endif

    static void setBlendMode( BlendMode bm )
    {
        switch ( bm )
        {
            case Blend_normal:
                if ( glFs.glBlendEquationSeparate )
                    glFs.glBlendEquationSeparate( GL_FUNC_ADD, GL_FUNC_ADD );

			    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
                break;

            case Blend_add:
                if ( glFs.glBlendEquationSeparate )
                    glFs.glBlendEquationSeparate( GL_FUNC_ADD, GL_FUNC_ADD );

			    glBlendFunc( GL_SRC_ALPHA, GL_ONE );
                break;

            case Blend_subtract:
                if ( glFs.glBlendEquationSeparate )
                    glFs.glBlendEquationSeparate( GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD );

			    glBlendFunc( GL_SRC_ALPHA, GL_ONE );
                break;

            case Blend_invert:
                if ( glFs.glBlendEquationSeparate )
                    glFs.glBlendEquationSeparate( GL_FUNC_ADD, GL_FUNC_ADD );

			    glBlendFunc( GL_ZERO, GL_ONE_MINUS_DST_COLOR );
                break;
        }
    }

    Engine::Engine( const String& appName ) : display( 0 ), appName( appName ), scene( 0 ), currentShader( 0 ), cursor( 0 )
    {
#ifndef Storm_Release_Build
        Engine::logEvent( "StormGraph.Engine", "-- SG DEVELOPMENT DEBUG BUILD --" );
#endif

        Engine::logEvent( "StormGraph.Engine", "initializing `" + appName + "`" );

        if ( instance )
            throw Exception( "StormGraph.Engine", "Engine", "TooManyEngines", "Only one instance of StormGraph engine per application can be created." );

        configDoc = cfx2_load_document( appName + ".cfx2" );

        instance = this;

        if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE ) == -1 )
            throw Exception( "StormGraph.Engine", "Engine", "GfxInitError", "SDL core initialization failed. No idea why." );

        SDL_EnableUNICODE( 1 );

        if ( TTF_Init() == -1 )
            throw Exception( "StormGraph.Engine", "Engine", "GfxInitError", "SDL_ttf library initialization failed. No idea why." );

        addFileSystem( "" );

        const char* driverName = getConfig( "StormGraph/driver" );
        GraphicsDriverProvider provider = loadGraphicsDriver( driverName );

        driver = provider( driverName );
    }

    Engine::~Engine()
    {
        iterate ( fileSystems )
            delete fileSystems.current();

        release( scene );

        if ( driver )
            delete driver;

        cfx2_release_node( configDoc );
        SDL_Quit();
        instance = 0;
    }

    void Engine::addFileSystem( const String& fs )
    {
        FileSystem* sys = IO::createFs( fs );

        if ( !sys )
            throw Exception( "StormGraph.Engine", "addFileSystem", "FsOpenError",
                    "Failed to open virtual filesystem `" + fs + "`. Invalid filename or protocol." );

        fileSystems.add( sys );
    }

    void Engine::assertionFail( const String& sourceUnit, int line, const String& className, const String& methodName, const String& assertion, const String& desc )
    {
        String description = sourceUnit + ":" + line + "\n\nfailed assertion `" + assertion + "`";
        throw Exception( className, methodName, "AssertionFailed", desc.isEmpty() ? description : description + "\n(" + desc + ")" );
    }

#ifndef StormGraph_No_Helium
    Helium::Variable Engine::initMembers( Helium::Variable obj )
    {
        obj.setMember( "Blend_add", Helium::Variable( Blend_add ) );
        obj.setMember( "Blend_subtract", Helium::Variable( Blend_subtract ) );
        return obj;
    }
#endif

    void Engine::changeScene( Scene* newScene )
    {
        if ( scene )
        {
            scene->release();
            scene = 0;
        }

        scene = newScene;
    }

#ifndef StormGraph_No_Helium
    void Engine::changeScene( Helium::HeVM* vm, Helium::Variable newScene )
    {
        if ( scene )
        {
            scene->release();
            scene = 0;
        }

        scene = new HeliumScene( vm, newScene );
    }
#endif

    /*void Engine::detachShader()
    {
        currentShader = 0;
        glFs.UseProgram( 0 );
    }*/

    void Engine::disableDepthTesting()
    {
        glDisable( GL_DEPTH_TEST );
    }

    void Engine::drawLine( const Vector<float>& begin, const Vector<float>& end, const Colour& colour )
    {
        const float vertices[] =
        {
            begin.x, begin.y, begin.z,
            end.x, end.y, end.z
        };

        // Set-up

		glDisable( GL_TEXTURE_2D );

        setColour( colour );

		glEnableClientState( GL_VERTEX_ARRAY );
        glVertexPointer( 3, GL_FLOAT, 0, vertices );

        // Render
        glDrawArrays( GL_LINES, 0, 2 );

        // Clean up
        glDisableClientState( GL_VERTEX_ARRAY );
    }

    void Engine::drawRect( const Vector<float>& begin, const Vector<float>& end, const Colour& colour )
    {
        const float vertices[] =
        {
            end.x, begin.y, ( begin.z + end.z ) / 2,
            begin.x, begin.y, begin.z,
            end.x, end.y, end.z,

            end.x, end.y, end.z,
            begin.x, begin.y, begin.z,
            begin.x, end.y, ( begin.z + end.z ) / 2
        };

        // Set-up

		glDisable( GL_TEXTURE_2D );

        setColour( colour );

        glEnableClientState( GL_VERTEX_ARRAY );
        glVertexPointer( 3, GL_FLOAT, 0, vertices );

        // Render
        glDrawArrays( GL_TRIANGLES, 0, 6 );
        numPolysThisFrame += 2;

        // Clean up
        glDisableClientState( GL_VERTEX_ARRAY );
    }

    void Engine::enableDepthTesting()
    {
        glEnable( GL_DEPTH_TEST );
    }

#ifndef StormGraph_No_Helium
    void Engine::exception( const char* className, const char* methodName, const char* title, const char* description )
    {
        throw Exception( className, methodName, title, description );
    }
#endif

    const char* Engine::getConfig( const char* path, bool essential )
    {
        // TODO !!!!
        return cfx2_query_value( configDoc, path );
    }

    int Engine::getConfigInt( const char* path )
    {
        String value = cfx2_query_value( configDoc, path );

        if ( value )
            return value;

        return 0;
    }

    unsigned short Engine::getKey( const char* name )
    {
        if ( !name )
            return 0;

        for ( const Key* key = keys; key->name; key++ )
            if ( strcmp( name, key->name ) == 0 )
                return key->key;

        return tolower( *name );
    }

    String Engine::getKeyName( unsigned short code )
    {
        for ( const Key* key = keys; key->name; key++ )
            if ( key->key == code )
                return key->name;

        return ( char )toupper( code );
    }

    int Engine::isPointVisible( const Vector<float>& point )
    {
        return frustum.pointInFrustum( point );
    }

    int Engine::isSphereVisible( const Vector<float>& center, float radius )
    {
        return frustum.sphereInFrustum( center, radius );
    }

#ifndef StormGraph_No_Helium
    static cfx2_Action loadCfx2_iterateCallback( unsigned index, cfx2_Node* child, cfx2_Node* parent, Helium::Variable* list )
    {
		Helium::Variable obj = Helium::Variable::newObject( 0 );
        obj.setMember( "name", Helium::Variable::newString( child->name, -1 ) );

        if ( cfx2_has_children( child ) )
        {
            Helium::Variable children = Helium::Variable::newList( 0 );
            cfx2_iterate_child_nodes( child, ( cfx2_IterateCallback )loadCfx2_iterateCallback, &children );
            obj.setMember( "children", children );
        }

        if ( child->text )
            obj.setMember( "text", Helium::Variable::newString( child->text, -1 ) );

        for ( unsigned i = 0; i < child->attributes->length; i++ )
            obj.setMember( ( ( cfx2_Attrib* )child->attributes->items[i] )->name,
                    Helium::Variable::newString( ( ( cfx2_Attrib* )child->attributes->items[i] )->value, -1 ) );

        list->addItem( obj );
        return cfx2_continue;
    }

    Helium::Variable Engine::loadCfx2( const char* fileName, bool essential )
    {
        cfx2_Node* doc = loadCfx2Asset( fileName, essential );

        if ( !doc )
            return Helium::Variable();

        Helium::Variable list = Helium::Variable::newList( 0 );
        cfx2_iterate_child_nodes( doc, ( cfx2_IterateCallback )loadCfx2_iterateCallback, &list );
        cfx2_release_node( doc );
        return list;
    }
#endif

    cfx2_Node* Engine::loadCfx2Asset( const char* fileName, bool essential )
    {
        cfx2_Node* doc = cfx2_load_document( fileName );

        if ( !doc && essential )
            throw Exception( "StormGraph.Engine", "loadCfx2Asset", "DocumentLoadError",
                    ( String )"Failed to load `" + fileName + "`. The file probably either doesn't exist at all or is not a valid cfx2 document." );

        return doc;
    }

    GraphicsDriverProvider Engine::loadGraphicsDriver( const char* name )
    {
//#ifdef __li_MSW
        // TODO: ever release the library?

        const String fileName = String( "Driver." ) + name + ".win32.gcc4.dll";

        HMODULE library = LoadLibraryA( fileName );

        if ( !library )
            throw Exception( "StormGraph.Engine", "loadGraphicsDriver", "DllLoadError",
                    ( String )"Failed to load `" + fileName + "`." );

        GraphicsDriverProvider provider = ( GraphicsDriverProvider )GetProcAddress( library, "createGraphicsDriver" );

        if ( !provider )
            throw Exception( "StormGraph.Engine", "loadGraphicsDriver", "EntryPointNotFound",
                    ( String )"Failed to link entry point `createGraphicsDriver` in `" + fileName + "`." );

        return provider;
//#endif
    }

    void Engine::loadKeyBindings( Array<unsigned short>& values, const char* fileName, const char** bindingNames, unsigned count )
    {
        cfx2_Node* doc = loadCfx2Asset( fileName );

        for ( unsigned i = 0; i < count; i++ )
            values[i] = getKey( cfx2_query_value( doc, ( String )"Controls/" + bindingNames[i] ) );

        cfx2_release_node( doc );
    }

#ifndef StormGraph_No_Helium
    Helium::Variable Engine::loadKeyBindings( const char* fileName )
    {
        cfx2_Node* doc = loadCfx2Asset( fileName );
        cfx2_Node* controls = cfx2_find_child( doc, "Controls" );

        Helium::Variable object = Helium::Variable::newObject( 0 );

        cfx2_iterate_child_nodes( controls, ( cfx2_IterateCallback )iterateControlsCallback, &object );

        cfx2_release_node( doc );
        return object;
    }
#endif

    String Engine::loadTextAsset( const char* fileName, bool essential )
    {
        SeekableInputStream* input = open( fileName );

        if ( !input || !input->isReadable() )
        {
            if ( essential )
                throw Exception( "StormGraph.Engine", "loadTextAsset", "AssetOpenError",
                        ( String )"Failed to open `" + fileName + "` for reading. The file probably doesn't exist at all." );
            else
                return String();
        }

        size_t size = input->getSize();

        Array<char> buffer( size + 1 );
        input->read( *buffer, size );
        return String( *buffer );
    }

    void Engine::logEvent( const char* className, const char* event )
    {
        LoggedEvent ev = { className, event };

        time( &ev.time );

        loggedEvents.add( ev );
    }

    SeekableInputStream* Engine::open( const char* fileName )
    {
        iterate ( fileSystems )
        {
            SeekableInputStream* input = fileSystems.current()->openInput( fileName );

            if ( input )
                return input;
        }

        return 0;
    }

    void Engine::printEventLog( OutputStream* output )
    {
        output->writeLine();

        iterate ( loggedEvents )
            output->writeLine( loggedEvents.current().getTime() + __li_lineEnd + "    " + loggedEvents.current().className + ": " + loggedEvents.current().event );

        output->release();
    }

    Engine* Engine::pushBlendMode( BlendMode bm )
    {
        setBlendMode( bm );
        blendModeStack.push( bm );

        return this;
    }

    Engine* Engine::popBlendMode()
    {
        blendModeStack.pop();
        setBlendMode( blendModeStack.top() );

        return this;
    }

    void Engine::run( Scene* defaultScene )
    {
        changeScene( defaultScene );

        unsigned counter = 0;
        isRunning = true;
        SDL_Event event;

        fps.start();

        while ( isRunning )
        {
            fps.newFrame();

            if ( SDL_PollEvent( &event ) )
            {
                switch ( event.type )
                {
                    case SDL_KEYDOWN:
                        if ( event.key.keysym.sym == SDLK_ESCAPE )
                            isRunning = !scene->closeButtonAction();

                        scene->keyStateChange( event.key.keysym.sym, true, event.key.keysym.unicode );
                        break;

                    case SDL_KEYUP:
                        scene->keyStateChange( event.key.keysym.sym, false, event.key.keysym.unicode );
                        break;

                    case SDL_MOUSEBUTTONUP:
                    case SDL_MOUSEBUTTONDOWN:
#ifdef __li_MSW
                        if ( cursor )
                            SetCursor( ( HCURSOR )cursor );
#endif

                        if ( event.button.button == SDL_BUTTON_LEFT )
                            scene->mouseButton( event.button.x, event.button.y, false, event.type == SDL_MOUSEBUTTONDOWN );
                        else if ( event.button.button == SDL_BUTTON_RIGHT )
                            scene->mouseButton( event.button.x, event.button.y, true, event.type == SDL_MOUSEBUTTONDOWN );
                        else if ( event.button.button == SDL_BUTTON_WHEELUP && event.type == SDL_MOUSEBUTTONDOWN )
                            scene->mouseWheel( false );
                        else if ( event.button.button == SDL_BUTTON_WHEELDOWN && event.type == SDL_MOUSEBUTTONDOWN )
                            scene->mouseWheel( true );
                        break;

                    case SDL_MOUSEMOTION:
#ifdef __li_MSW
                        if ( cursor )
                            SetCursor( ( HCURSOR )cursor );
#endif

                        scene->mouseMove( event.motion.x, event.motion.y );
                        break;

                    case SDL_QUIT:
                        isRunning = !scene->closeButtonAction();
                        break;
                }
            }

            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

            numPolysThisFrame = 0;
            nextLightId = 0;

            scene->render();

            SDL_GL_SwapBuffers();

            fps.endFrame();

            if ( ++counter == 30 )
            {
                SDL_WM_SetCaption( appName + " // " + fps.getRate() + " fps | " + fps.getLastMilis() + " ms | " + numPolysThisFrame + " poly", 0 );
                fps.restart();
                counter = 0;
            }
        }
    }

#ifndef StormGraph_No_Helium
    void Engine::run( Helium::HeVM* vm, Helium::Variable sceneObj )
    {
        HeliumScene* hs = new HeliumScene( vm, sceneObj );
        run( hs );
        delete scene;
    }
#endif

    void Engine::setCursor( bool enable, const char* fileName )
    {
        SDL_ShowCursor( enable ? SDL_ENABLE : SDL_DISABLE );

#ifdef __li_MSW
        if ( fileName )
        {
            SDL_Surface* cursorSurface = Texture::load( fileName );

            if ( !cursorSurface )
            {
                cursor = 0;
                return;
            }

            BITMAPV5HEADER bi;
            DWORD* destPixel;

            ZeroMemory( &bi,sizeof( BITMAPV5HEADER ) );
            bi.bV5Size = sizeof( BITMAPV5HEADER );
            bi.bV5Width = cursorSurface->w;
            bi.bV5Height = cursorSurface->h;
            bi.bV5Planes = 1;
            bi.bV5BitCount = cursorSurface->format->BitsPerPixel;
            bi.bV5Compression = BI_BITFIELDS;

            bi.bV5RedMask = 0x00FF0000;
            bi.bV5GreenMask = 0x0000FF00;
            bi.bV5BlueMask = 0x000000FF;
            bi.bV5AlphaMask = 0xFF000000;

            HDC hdc = GetDC( 0 );
            HBITMAP cursorBitmap = CreateDIBSection( hdc, ( BITMAPINFO* )&bi, DIB_RGB_COLORS, ( void** )&destPixel, 0, 0 );
            ReleaseDC( 0, hdc );

            for ( int y = 0; y < cursorSurface->h; y++ )
                for ( int x = 0; x < cursorSurface->w; x++ )
                {
                    char* sourcePixel = ( char* )cursorSurface->pixels + cursorSurface->pitch * ( cursorSurface->h - y - 1 ) + cursorSurface->format->BytesPerPixel * x;
                    Uint8* pixel = ( Uint8* )destPixel;

                    SDL_GetRGBA( *( Uint32* )( sourcePixel ), cursorSurface->format, pixel + 2, pixel + 1 , pixel, pixel + 3 );
                    destPixel++;
                }

            SDL_FreeSurface( cursorSurface );

            HBITMAP maskBitmap = CreateBitmap( cursorSurface->w, cursorSurface->h, 1, 1, 0 );

            ICONINFO ii;
            ii.fIcon = FALSE;
            ii.xHotspot = 0;
            ii.yHotspot = 0;
            ii.hbmMask = maskBitmap;
            ii.hbmColor = cursorBitmap;

            cursor = CreateIconIndirect( &ii );

            DeleteObject( cursorBitmap );
            DeleteObject( maskBitmap );
        }
#endif
    }

    void Engine::setColour( const Colour& colour )
    {
        if ( currentShader )
            currentShader->setVertexColour( colour );
        else
            glColor4f( colour.r, colour.g, colour.b, colour.a );
    }

    Engine* Engine::setMode( const char* windowTitle, unsigned width, unsigned height, int fullscreen )
    {
        int multisample = getConfigInt( "StormGraph/multisample" );
        bool vsync = ( getConfigInt( "StormGraph/vsync" ) != 0 );

		if ( multisample )
        {
            SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
            SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, multisample );
        }

        if ( !width )
            width = getConfigInt( "StormGraph/width" );

        if ( !height )
            height = getConfigInt( "StormGraph/height" );

        if ( fullscreen < 0 )
            fullscreen = getConfigInt( "StormGraph/fullscreen" );

        if ( !width || !height || fullscreen < 0 )
            throw Exception( "StormGraph.Engine", "setMode", "InvalidConfig",
                    "Invalid values for display width, height, or fullscreen (check application configuration file - `" + appName + ".cfx2`)" );

        SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, vsync );
        //SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

        window.x = width;
        window.y = height;

        Engine::logEvent( "StormGraph.Engine", ( String )"setting display mode to " + width + "x" + height + "x32 bits" );

		if ( !( display = SDL_SetVideoMode( window.x, window.y, 32, SDL_OPENGL | ( fullscreen > 0 ? SDL_FULLSCREEN : 0 ) ) ) )
            throw Exception( "StormGraph.Engine", "setMode", "GfxInitError", "SDL/OpenGL subsystem initialization failed" );

#if StormGraph_Render_Mode == StormGraph_Render_OpenGL_2_Fixed
        shadersEnabled = false;
#elif StormGraph_Render_Mode == StormGraph_Render_OpenGL_Multi || StormGraph_Render_Mode == StormGraph_Render_OpenGL_3
        shadersEnabled = true;
#else
#error Undefined for the selected render pipeline.
#endif

        //* Nice & scalable way to dynamically link GL functions
        for ( unsigned i = 0; i < sizeof( glFs.functions ) / sizeof( *glFs.functions ); i++ )
        {
            glFs.functions[i] = 0;

            if ( ( glLinkTable[i].flags & Gl_shaders ) && !shadersEnabled )
                continue;

            glFs.functions[i] = SDL_GL_GetProcAddress( glLinkTable[i].name );

            if ( !glFs.functions[i] )
            {
                if ( glLinkTable[i].flags & Gl_shaders || glLinkTable[i].flags & Gl_optional )
                {
                    printf( "StormGraph Warning: '%s' not available.\n", glLinkTable[i].name );
                    shadersEnabled = false;
                }
                else
                    throw Exception( "StormGraph.Engine", "setMode", "OpenGlLinkError",
                            ( String )"Failed to link `" + glLinkTable[i].name + "`\nupdating your graphics drivers might do the trick (OpenGL 2.0 is required)" );
            }
        }

        if ( !shadersEnabled )
        {
#ifdef __li_MSW
            MessageBoxA( 0, "StormGraph Engine failed to link the OpenGL Shader API.\n\nThis functionality requires OpenGL 2.0 or newer."
                    " Please make sure you have the latest drivers for your graphics card installed. If that doesn't help either, your graphics card"
                    " is probably too old and doesn't support the advanced functionality.\n\nStormGraph Engine will run in compatibility mode.",
                    "StormGraph Engine Warning", MB_ICONWARNING );
#else
            printf( " --- StormGraph Engine Warning ---\n\n" );
            printf( "StormGraph Engine failed to link the OpenGL Shader API.\n\nThis functionality requires OpenGL 2.0 or newer."
                    " Please make sure you have the latest drivers for your graphics card installed. If that doesn't help either, your graphics card"
                    " is probably too old and doesn't support the advanced functionality.\n\nStormGraph Engine will run in compatibility mode.\n\n" );
#endif
        }

        //* Init OpenGL
        glViewport( 0, 0, width, height );

        // Shading, lighting & other colour-related stuff
        glShadeModel( GL_SMOOTH );

        glClearColor( 0.f, 0.f, 0.f, 0.f );
        glClearDepth( 1.0f );

        //* Enable alpha blending
        glEnable( GL_BLEND );
        blendModeStack.clear();
        pushBlendMode( Blend_normal );

        glEnable( GL_ALPHA_TEST );
        glAlphaFunc( GL_GREATER, 0.1f );

        // Configure depth tesing
        glDepthFunc( GL_LEQUAL );

        //* Use nice perspective correction
        glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

        // Don't render backfaces
        glEnable( GL_CULL_FACE );

        int error = glGetError();

        if ( error != GL_NO_ERROR )
            throw Exception( "StormGraph.Engine", "setMode", "GfxInitError", ( String )"OpenGL set-up failed: error " + error );

        SDL_WM_SetCaption( windowTitle ? windowTitle : "StormGraph Alpha", 0 );

        setSceneAmbient( Colour( 0.0f, 0.0f, 0.0f, 0.0f ) );

        Engine::logEvent( "StormGraph.Engine", ( String ) "setMode() procedure completed. some details follow:\n"
                + "    OpenGL renderer: `" + ( const char* ) glGetString( GL_RENDERER ) + "` " + ( const char* ) glGetString( GL_VERSION ) + " by `" + ( const char* ) glGetString( GL_VENDOR ) + "`\n"
                + "    available shading language: GLSL " + ( const char* ) glGetString( GL_SHADING_LANGUAGE_VERSION ) );

        return this;
    }

    Engine* Engine::setOrthoProjection( double nearZ, double farZ )
    {
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();

        //* left X, right X, bottom Y, top Y, near Z, far Z
        glOrtho( 0.0, window.x, window.y, 0.0, nearZ, farZ );

        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();

        glDisable( GL_DEPTH_TEST );
        glDisable( GL_LIGHTING );

        return this;
    }

    Engine* Engine::setPerspectiveProjection( double nearZ, double farZ )
    {
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();

        //* init the perspective view with some reasonable FOV
        gluPerspective( 45.0f, ( GLfloat )( window.x ) / ( GLfloat )( window.y ), nearZ, farZ );
        frustum.setProjection( 45.0f, ( GLfloat )( window.x ) / ( GLfloat )( window.y ), nearZ, farZ );

        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();

        glEnable( GL_DEPTH_TEST );

        return this;
    }

    void Engine::setPickingId( unsigned id )
    {
        if ( currentShader )
            currentShader->setVertexColour( Colour( id & 0xFF, ( id >> 8 ) & 0xFF, ( id >> 16 ) & 0xFF, 0xFF ) );
        else
        {
            id |= 0xFF000000;
            glColor4ubv( ( const GLubyte* ) &id );
        }
    }

    void Engine::setSceneAmbient( const Colour& colour )
    {
        if ( currentShader )
            currentShader->setSceneAmbient( colour );
        else
        {
            GLfloat ambient[] = { colour.r, colour.g, colour.b, colour.a };
            glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambient );
        }
    }
}
