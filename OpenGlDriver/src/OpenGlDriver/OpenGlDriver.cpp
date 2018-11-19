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

#include "OpenGlDriver.hpp"

#include <StormGraph/Engine.hpp>
#include <StormGraph/GeometryFactory.hpp>
#include <StormGraph/ResourceManager.hpp>
#include <StormGraph/Scene.hpp>

#include <littl/FileName.hpp>

#include <glm/gtc/matrix_transform.hpp>

#ifdef OpenGlDriver_With_GLX
#include <GL/glx.h>
#endif

#define GL_TEXTURE_FREE_MEMORY_ATI  0x87FC

namespace OpenGlDriver
{
    static const char* version = "TRUNK";

    class FloatRefVariable : public IVariable
    {
        protected:
            float& value;

            virtual ~FloatRefVariable() {}

        public:
            FloatRefVariable( float& value ) : value( value ) {}

            virtual String getValue() { return String::formatFloat( value ); }
            virtual bool setValue( const String& value ) { this->value = String::toFloat( value ); return true; }
    };

    class SizeRefVariable : public IVariable
    {
        protected:
            size_t& value;

            virtual ~SizeRefVariable() {}

        public:
            SizeRefVariable( size_t& value ) : value( value ) {}

            virtual String getValue() { return String::formatInt( value ); }
            virtual bool setValue( const String& value ) { this->value = String::toInt( value ); return true; }
    };

    class SdlEventSource : public IEventSource, public IEventInput
    {
        SDL_Surface* display;
        OpenGlDriver::Profiling& profiling;

        public:
            SdlEventSource( SDL_Surface* display, OpenGlDriver::Profiling& profiling );
            virtual ~SdlEventSource();

            virtual void processEvents( IEventListener* listener );

            virtual void ReceiveEvents( IEventReceiver* receiver );
    };

    enum
    {
        Gl_always = 1,
        Gl_optional = 2,
        Gl_vbos = 4,
        Gl_shaders = 8,
        Gl_renderbuffers = 16,
        Gl_vaos = 32
    };

    struct GlFunction
    {
        const char* name;
        unsigned flags;
    };

    struct Key
    {
        const static int16_t anyMouse = -1, leftMouse = -2, rightMouse = -3, mouseWheelUp = -4, mouseWheelDown = -5;

        const char* name;
        int16_t key;
    };

    static const GlFunction glLinkTable[] =
    {
        { "glActiveTexture",                Gl_always },
        { "glAttachShader",                 Gl_shaders },
        { "glBindBuffer",                   Gl_vbos },
        { "glBindFramebufferEXT",           Gl_renderbuffers },
        { "glBindRenderbufferEXT",          Gl_renderbuffers },
        { "glBlendEquationSeparate",        Gl_always | Gl_optional },
        { "glBufferData",                   Gl_vbos },
        { "glBufferSubData",                Gl_vbos },
        { "glCheckFramebufferStatus",       Gl_renderbuffers },
        { "glClientActiveTexture",          Gl_always },
        { "glCompileShader",                Gl_shaders },
        { "glCompressedTexImage2DARB",      Gl_always | Gl_optional },
        { "glCreateProgram",                Gl_shaders },
        { "glCreateShader",                 Gl_shaders },
        { "glDeleteBuffers",                Gl_renderbuffers },
        { "glDeleteFramebuffersEXT",        Gl_renderbuffers },
        { "glDeleteProgram",                Gl_shaders },
        { "glDeleteShader",                 Gl_shaders },
        { "glDetachShader",                 Gl_shaders },
        { "glFramebufferRenderbufferEXT",   Gl_renderbuffers },
        { "glFramebufferTexture2DEXT",      Gl_renderbuffers },
        { "glGenBuffers",                   Gl_vbos },
        { "glGenFramebuffersEXT",           Gl_renderbuffers },
        { "glGenRenderbuffersEXT",          Gl_renderbuffers },
        { "glGetAttribLocation",            Gl_shaders },
        { "glGetProgramiv",                 Gl_shaders },
        { "glGetProgramInfoLog",            Gl_shaders },
        { "glGetShaderiv",                  Gl_shaders },
        { "glGetShaderInfoLog",             Gl_shaders },
        { "glGetUniformLocation",           Gl_shaders },
        { "glLinkProgram",                  Gl_shaders },
        { "glMapBuffer",                    Gl_vbos },
        { "glGetBufferSubData",             Gl_vbos },
        { "glRenderbufferStorageEXT",       Gl_renderbuffers },
        { "glShaderSource",                 Gl_shaders },
        { "glUniform1i",                    Gl_shaders },
        { "glUniform1f",                    Gl_shaders },
        { "glUniform2f",                    Gl_shaders },
        { "glUniform3f",                    Gl_shaders },
        { "glUniform4f",                    Gl_shaders },
        { "glUniform4fv",                   Gl_shaders },
        { "glUniformMatrix4fv",             Gl_shaders },
        { "glUnmapBuffer",                  Gl_vbos },
        { "glUseProgram",                   Gl_shaders },
        { "glVertexAttrib4f",               Gl_shaders }
    };

    static const Key keys[] =
    {
        // Virtual (negative id) keys
        { "Any Mouse Button",       Key::anyMouse },
        { "Left Mouse Button",      Key::leftMouse },
        { "Right Mouse Button",     Key::rightMouse },
        { "Mouse Wheel Up",         Key::mouseWheelUp },
        { "Mouse Wheel Down",       Key::mouseWheelDown },

        // Real (mostly) keys
        { "Backspace",              SDLK_BACKSPACE },
        { "Delete",                 SDLK_DELETE },
        { "Down",                   SDLK_DOWN },
        { "End",                    SDLK_END },
        { "Enter",                  SDLK_RETURN },
        { "Escape",                 SDLK_ESCAPE },
        { "Home",                   SDLK_HOME },
        { "Insert",                 SDLK_INSERT },
        { "Left",                   SDLK_LEFT },
        { "Left Shift",             SDLK_LSHIFT },
        { "Num Enter",              SDLK_KP_ENTER },
        { "Page Down",              SDLK_PAGEDOWN },
        { "Page Up",                SDLK_PAGEUP },
        { "Right",                  SDLK_RIGHT },
        { "Space",                  ' ' },
        { "Up",                     SDLK_UP },
        { 0 }
    };

    static bool haveExtension( const char* extension )
    {
        // Not a perfect check, but our engine (i mean, driver ofc) is smart enough to figure out when something ain't alright
        // Caveat: when looking for extension ABC, will return true even if ABC isn't present but ABCDEF is.

        return strstr( ( const char* ) glGetString( GL_EXTENSIONS ), extension ) != 0;
    }

    inline Event_t synthVKeyEvent( int flags, int vk, int x = 0, int y = 0 )
    {
        Event_t ev;
        ev.type = EV_VKEY;
        ev.vkey.flags = flags;
        ev.vkey.vk = vk;
        ev.vkey.x = x;
        ev.vkey.y = y;
        return ev;
    }

    OpenGlDriver::OpenGlDriver( IEngine* engine )
            : engine( engine ), eventListener( nullptr )
    {
        forceNoShaders = false;
        forceNoVbo = false;
        rendererOnly = false;

        profiling.profiler = engine->createProfiler();
        profiling.frame = profiling.profiler->enter( "frame#" );
        profiling.event = profiling.profiler->enter( "event#" );
        profiling.render = profiling.profiler->enter( "render#" );
        profiling.swap = profiling.profiler->enter( "swap#" );

        profiling.interval = 1;

        globalState.dynamicLightingEnabled = true;
        globalState.fontBatchingEnabled = true;
        globalState.fontBatchSize = 100;
        globalState.shadowPcfEnabled = true;
        globalState.shadowPcfDist = 0.008f;
        globalState.softShadows = true;

        features.gl3PlusOnly = 0;

        engine->setVariable( "driver.dynamicLightingEnabled",               engine->createBoolRefVariable( globalState.dynamicLightingEnabled ),        true );
        engine->setVariable( "driver.fontBatching",                         engine->createBoolRefVariable( globalState.fontBatchingEnabled ),           true );
        engine->setVariable( "driver.fontBatchSize",                        new SizeRefVariable( globalState.fontBatchSize ),                           true );
        engine->setVariable( "driver.forceNoShaders",                       engine->createBoolRefVariable( forceNoShaders ),                            true );
        engine->setVariable( "driver.forceNoVbo",                           engine->createBoolRefVariable( forceNoVbo ),                                true );
        engine->setVariable( "driver.shadowPcfEnabled",                     engine->createBoolRefVariable( globalState.shadowPcfEnabled ),              true );
        engine->setVariable( "driver.shadowPcfDist",                        new FloatRefVariable( globalState.shadowPcfDist ),                          true );
        engine->setVariable( "driver.softShadows",                          engine->createBoolRefVariable( globalState.softShadows ),                   true );

        engine->setVariable( "display.rendererOnly",                        engine->createBoolRefVariable( rendererOnly ),                              true );
    }

    OpenGlDriver::~OpenGlDriver()
    {
        // Release only memory structures here
        // All resources must be released in this->unload() instead
        // (before the GL context is destroyed by SDL_Quit)
    }

    int OpenGlDriver::addDirectionalLight( const DirectionalLightProperties& light, bool inWorldSpace )
    {
        unsigned index;

        if ( driverShared.useShaders )
        {
            if ( globalState.numDirectionalLights >= MAX_DIRECTIONAL_LIGHTS )
                return -1;

            index = globalState.numDirectionalLights++;
            DirectionalLightProperties& local = globalState.directionalLights[index];

            local = light;

            if ( inWorldSpace )
            {
                glm::mat4 transform;
                glGetFloatv( GL_MODELVIEW_MATRIX, &transform[0][0] );
                local.direction = glm::normalize( glm::vec3( transform * glm::vec4( -light.direction, 0.0f ) ) );
            }
            else
                local.direction = -light.direction;

            if ( renderState.currentShaderProgram != nullptr )
                renderState.currentShaderProgram->setDirectionalLight( index, local );

            return index;
        }
        else
        {
            if ( nextLightId >= ( unsigned ) driverShared.maxFixedLights )
                return -1;

            index = nextLightId++;

            if ( !inWorldSpace )
            {
                glPushMatrix();
                glLoadIdentity();
            }

            glEnable( GL_LIGHT0 + index );

#define COMPONENTS_OF( property_ ) { light.property_.r, light.property_.g, light.property_.b, light.property_.a }

            GLfloat ambient[] = COMPONENTS_OF( ambient );
            GLfloat diffuse[] = COMPONENTS_OF( diffuse );
            GLfloat specular[] = COMPONENTS_OF( specular );

#undef COMPONENTS_OF

            glLightfv( GL_LIGHT0 + index, GL_AMBIENT, ambient );
            glLightfv( GL_LIGHT0 + index, GL_DIFFUSE, diffuse );
            glLightfv( GL_LIGHT0 + index, GL_SPECULAR, specular );

            // included is also a little hack because of bugs in OpenGL
            GLfloat dir[] = { -light.direction.x - 0.0001f, -light.direction.y - 0.0001f, -light.direction.z - 0.0001f, 0.0f };
            glLightfv( GL_LIGHT0 + index, GL_POSITION, dir );

            if ( !inWorldSpace )
                glPopMatrix();
        }

        stats.numDirectionalLights++;

        return index;
    }

    int OpenGlDriver::addPointLight( const PointLightProperties& light, bool inWorldSpace )
    {
        unsigned index;

        if ( driverShared.useShaders )
        {
            if ( globalState.numPointLights >= MAX_POINT_LIGHTS )
                return -1;

            index = globalState.numPointLights++;
            PointLightProperties& local = globalState.pointLights[index];

            local = light;

            if ( inWorldSpace )
            {
                glm::mat4 transform;
                glGetFloatv( GL_MODELVIEW_MATRIX, &transform[0][0] );
                local.pos = glm::vec3( transform * glm::vec4( light.pos, 1.0f ) );
            }

            if ( renderState.currentShaderProgram != nullptr )
                renderState.currentShaderProgram->setPointLight( index, local );
        }
        else
        {
            if ( nextLightId >= ( unsigned ) driverShared.maxFixedLights )
                return -1;

            index = nextLightId++;

            if ( !inWorldSpace )
            {
                glPushMatrix();
                glLoadIdentity();
            }

            glEnable( GL_LIGHT0 + index );

#define COMPONENTS_OF( property_ ) { light.property_.r, light.property_.g, light.property_.b, light.property_.a }

            GLfloat ambient[] = COMPONENTS_OF( ambient );
            GLfloat diffuse[] = COMPONENTS_OF( diffuse );
            GLfloat specular[] = COMPONENTS_OF( specular );

#undef COMPONENTS_OF

            glLightfv( GL_LIGHT0 + index, GL_AMBIENT, ambient );
            glLightfv( GL_LIGHT0 + index, GL_DIFFUSE, diffuse );
            glLightfv( GL_LIGHT0 + index, GL_SPECULAR, specular );

            GLfloat position[] = { light.pos.x, light.pos.y, light.pos.z, 1.0f };
            glLightfv( GL_LIGHT0 + index, GL_POSITION, position );

            glLightf( GL_LIGHT0 + index, GL_CONSTANT_ATTENUATION, 0.0f );
            glLightf( GL_LIGHT0 + index, GL_LINEAR_ATTENUATION, 2.0f / light.range );
            glLightf( GL_LIGHT0 + index, GL_QUADRATIC_ATTENUATION, 0.0f );

            if ( !inWorldSpace )
                glPopMatrix();
        }

        stats.numPointLights++;

        return index;
    }

    void OpenGlDriver::applyTransforms( const List<Transform>& transforms )
    {
        applyTransforms( transforms.getPtrUnsafe(), transforms.getLength() );
    }

    glm::mat4 OpenGlDriver::applyTransforms( const Transform* transforms, unsigned numTransforms )
    {
        // OpenGL does it the reverse way
        // reversed reverse = correct! yay!

        glm::mat4 matrix;

        for ( int i = numTransforms - 1; i >= 0; i-- )
        {
            const Transform& current = transforms[i];

            switch ( current.operation )
            {
                case Transform::translate:
                    //glTranslatef( current.vector.x, current.vector.y, current.vector.z );
                    matrix = glm::translate( matrix, current.vector.operator glm::vec3() );
                    break;

                case Transform::rotate:
                    //glRotatef( ( GLfloat )( -current.angle * 180.0f / M_PI ), current.vector.x, current.vector.y, current.vector.z );
                    matrix = glm::rotate( matrix, ( float )( -current.angle * 180.0f / M_PI ), ( glm::vec3 ) current.vector.operator glm::vec3() );
                    break;

                case Transform::scale:
                    //glScalef( current.vector.x, current.vector.y, current.vector.z );
                    matrix = glm::scale( matrix, current.vector.operator glm::vec3() );
                    break;

                case Transform::matrix:
                    //glMultMatrixf( &current.transformation[0][0] );
                    matrix = matrix * current.transformation;
                    break;
            }
        }

        glMultMatrixf( &matrix[0][0] );
        return matrix;
    }

    void OpenGlDriver::beginDepthRendering()
    {
        // TODO: ;;;

        /*if ( globalState.shadersEnabled )
        {
            //SG_assert( false )


        }
        else*/
        {
            glCullFace( GL_FRONT );

            glShadeModel( GL_FLAT );
            glColorMask( 0, 0, 0, 0 );
        }
    }

    void OpenGlDriver::beginPicking()
    {
        glDisable( GL_BLEND );
        glDisable( GL_DITHER );
        glDisable( GL_FOG );
        glDisable( GL_TEXTURE_1D );
        glDisable( GL_TEXTURE_2D );
        glDisable( GL_TEXTURE_3D );
        glShadeModel( GL_FLAT );

        nextPickingId = 1;

        if ( driverShared.useShaders )
            getPickingShaderProgram()->select();
        else
            glDisable( GL_LIGHTING );

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    }

    void OpenGlDriver::changeDisplayMode( DisplayMode* displayMode )
    {
        if ( displayMode->changeWindowTitle && !rendererOnly )
            SDL_WM_SetCaption( displayMode->windowTitle, 0 );
    }

    void OpenGlDriver::checkErrors( const char* caller )
    {
        int error = glGetError();

        if ( error != GL_NO_ERROR )
            throw Exception( caller, "OpenGlError", "OpenGL runtime error: " + String::formatInt( error, -1, String::hexadecimal ) + "h" );
    }

    void OpenGlDriver::clear()
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    }

    void OpenGlDriver::clearLights()
    {
        if ( driverShared.useShaders )
        {
            globalState.numDirectionalLights = 0;
            globalState.numPointLights = 0;

            renderState.currentShaderProgram = nullptr;
        }
        else
        {
            for ( int i = 0; i < driverShared.maxFixedLights; i++ )
                glDisable( GL_LIGHT0 + i );

            nextLightId = 0;
        }
    }

    IModel* OpenGlDriver::createCuboid( const char* name, CuboidCreationInfo* cuboid )
    {
        Mesh* mesh = Mesh::createCuboid( this, cuboid, IModel::fullStatic );

        return new Model( this, name, &mesh, 1, true );
    }

    IModel* OpenGlDriver::createCuboid( const char* name, const CuboidCreationInfo2& creationInfo, IMaterial* material, unsigned flags )
    {
        Reference<IMaterial> materialGuard( material );

        List<Vertex> vertices;
        List<uint32_t> indices;

        GeometryFactory::createCuboidTriangles( creationInfo, vertices, indices );

        MeshCreationInfo2 meshCreationInfo = { String(), MeshFormat::triangleList, MeshLayout::indexed, materialGuard.detach(),
                vertices.getLength(), indices.getLength(), vertices.getPtr(), indices.getPtr() };

        return createModelFromMemory( name, &meshCreationInfo, 1, IModel::fullStatic );
    }

    /*IMaterial* OpenGlDriver::createCustomMaterial( const char* name, IShader* shader )
    {
        if ( !driverShared.useShaders )
            return 0;

        return new Material( this, name, ( Program* ) shader );
    }

    IShader* OpenGlDriver::createCustomShader( const char* base, const char* name )
    {
        if ( !driverShared.useShaders )
            return 0;

        return new Program( this, base, name );
    }*/

    ITexture* OpenGlDriver::createDepthTexture( const char* name, const Vector2<unsigned>& resolution )
    {
        return new Texture( this, name, resolution );
    }

    ILight* OpenGlDriver::createDirectionalLight( const Vector<float>& direction, const Colour& ambient, const Colour& diffuse, const Colour& specular )
    {
        DirectionalLightProperties properties;

        properties.ambient = ambient;
        properties.diffuse = diffuse;
        properties.direction = direction;
        properties.specular = specular;

        return new Light( this, &properties );
    }

    IMaterial* OpenGlDriver::createMaterial( const char* name, const MaterialProperties2* material, bool finalized )
    {
        return new Material( this, name, material, finalized );
    }

    IFont* OpenGlDriver::createFontFromStream( const char* name, SeekableInputStream* input, unsigned size, unsigned style )
    {
        return new Font( this, name, input, size, style );
    }

    Image* OpenGlDriver::createImageFromStream( SeekableInputStream* input )
    {
        Reference<> inputGuard( input );

        return engine->getImageLoader()->load( input );
    }

    /*IModel* OpenGlDriver::createModelFromMemory( const char* name, List<MeshCreationInfo*>& meshes, unsigned flags )
    {
        return new Model( this, name, meshes, flags );
    }*/

    IModel* OpenGlDriver::createModelFromMemory( const char* name, MeshCreationInfo2* meshes, size_t count, unsigned flags )
    {
        Reference<IModelPreload> preload = preloadModelFromMemory( name, meshes, count, flags );

        return preload->getFinalized();
    }

    IModel* OpenGlDriver::createModelFromMemory( const char* name, MeshCreationInfo2** meshes, size_t count, unsigned flags )
    {
        Reference<IModelPreload> preload = preloadModelFromMemory( name, meshes, count, flags );

        return preload->getFinalized();
    }

    IModel* OpenGlDriver::createModelFromMemory( const char* name, MeshCreationInfo3* meshes, size_t count, unsigned flags )
    {
        return new Model( this, name, meshes, count, flags, true );
    }

    IModel* OpenGlDriver::createModelFromMemory( const char* name, MeshCreationInfo3** meshes, size_t count, unsigned flags )
    {
        return new Model( this, name, meshes, count, flags, true );
    }

    IModel* OpenGlDriver::createPlane( const char* name, PlaneCreationInfo* plane )
    {
        Mesh* mesh = Mesh::createPlane( this, plane, IModel::fullStatic );

        return new Model( this, name, &mesh, 1, true );
    }

    ILight* OpenGlDriver::createPointLight( float range, const Colour& ambient, const Colour& diffuse, const Colour& specular )
    {
        PointLightProperties properties;

        properties.ambient = ambient;
        properties.diffuse = diffuse;
        properties.range = range;
        properties.specular = specular;

        return new Light( this, &properties );
    }

    IProjectionInfoBuffer* OpenGlDriver::createProjectionInfoBuffer()
    {
        return new ProjectionInfoBuffer;
    }

    IRenderBuffer* OpenGlDriver::createRenderBuffer( const Vector<unsigned>& dimensions, bool withDepthBuffer )
    {
        if ( driverShared.haveRenderBuffers )
            return new RenderBuffer( this, dimensions, withDepthBuffer );
        else
            return nullptr;
    }

    IRenderBuffer* OpenGlDriver::createRenderBuffer( ITexture* depthTexture )
    {
        if ( driverShared.haveRenderBuffers && depthTexture != nullptr )
            return new RenderBuffer( this, static_cast<Texture*>( depthTexture ) );
        else
            return nullptr;
    }

    IRenderQueue* OpenGlDriver::createRenderQueue()
    {
        return new RenderQueue( this );
    }

    IMaterial* OpenGlDriver::createSolidMaterial( const char* name, const Colour& colour, ITexture* texture )
    {
        MaterialProperties2 material;
        memset( &material, 0, sizeof( material ) );

        material.colour = colour;

        if ( texture != nullptr )
        {
            material.numTextures = 1;
            material.textures[0] = texture;
        }

        return new Material( this, name, &material, true );
    }

    ITexture* OpenGlDriver::createSolidTexture( const char* name, const Colour& colour )
    {
        return new Texture( this, name, colour );
    }

    IStaticModel* OpenGlDriver::createStaticModelFromBsp( const char* name, BspTree* bsp, IResourceManager* resMgr, bool finalized )
    {
        // TODO: inline
        return new BspModel( this, name, bsp, resMgr, finalized );
    }

    IModel* OpenGlDriver::createTerrain( const char* name, TerrainCreationInfo* terrain, unsigned flags )
    {
        Mesh* mesh = Mesh::createFromHeightMap( this, terrain, flags, true );

        return new Model( this, name, &mesh, 1, true );
    }

    ITexture* OpenGlDriver::createTextureFromStream( SeekableInputStream* input, const char* name, ILodFunction* lodFunction )
    {
        Object<Image> image = createImageFromStream( input );

        if ( image == nullptr )
            throw StormGraph::Exception( "OpenGlDriver.OpenGlDriver.createTextureFromStream", "GraphicsLoadError",
                    ( String ) "Failed to load texture " + FileName::format( name ) + ". File format was not recognized." );

        return new Texture( this, name, image, lodFunction );
    }

    void OpenGlDriver::draw2dCenteredRotated( ITexture* texture, float scale, float angle, const Colour& blend )
    {
        Transform transforms[4];

        const Vector2<> size = texture->getDimensions().getXy();

        transforms[0] = Transform( Transform::translate, Vector<>( -0.5f, -0.5f ) );
        transforms[1] = Transform( Transform::rotate, Vector<>( 0.0f, 0.0f, 1.0f ), angle );
        transforms[2] = Transform( Transform::scale, size * scale );
        transforms[3] = Transform( Transform::translate, viewport / 2 );

        glPushMatrix();

        applyTransforms( transforms, 4 );

        if ( texture == nullptr )
            plane1x1uv->render( solidMaterial, blend );
        else
            plane1x1uv->render( texturedMaterial, blend, static_cast<Texture*>( texture ) );

        glPopMatrix();
    }

    void OpenGlDriver::drawLine( const Vector<float>& a, const Vector<float>& b, const Colour& blend )
    {
        Transform transforms[2];

        transforms[0] = Transform( Transform::scale, b - a );
        transforms[1] = Transform( Transform::translate, a );

        glPushMatrix();

        applyTransforms( transforms, 2 );

        line->render( solidMaterial, blend );

        glPopMatrix();
    }

    void OpenGlDriver::drawRectangle( const Vector<float>& pos, const Vector2<float>& size, const Colour& blend, ITexture* texture )
    {
        Transform transforms[2];

        transforms[0] = Transform( Transform::scale, size );
        transforms[1] = Transform( Transform::translate, pos );

        glPushMatrix();

        applyTransforms( transforms, 2 );

        if ( texture == nullptr )
            plane1x1uv->render( solidMaterial, blend );
        else
            plane1x1uv->render( texturedMaterial, blend, static_cast<Texture*>( texture ) );

        glPopMatrix();
    }

    void OpenGlDriver::drawRectangleOutline( const Vector<float>& pos, const Vector2<float>& size, const Colour& blend, ITexture* texture )
    {
        Transform transforms[2];

        transforms[0] = Transform( Transform::scale, size );
        transforms[1] = Transform( Transform::translate, pos );

        glPushMatrix();

        applyTransforms( transforms, 2 );

        if ( texture == nullptr )
            rect1x1->render( solidMaterial, blend );
        else
            rect1x1->render( texturedMaterial, blend, static_cast<Texture*>( texture ) );

        glPopMatrix();
    }

    void OpenGlDriver::drawStats()
    {
        if ( statsFont == nullptr )
            statsFont = engine->getSharedResourceManager()->getFont( "Common/Fonts/DejaVuSansMono.ttf", 10, IFont::normal );

        String info = ( String ) "OpenGlDriver " + version + " " StormGraph_Target + "\n";
        info += String::formatInt( stats.numPolys ) + " rendered polys\n";
        info += String::formatInt( stats.numTextures ) + " texture switches\n";
        info += String::formatInt( stats.numRenderCalls ) + " render calls\n";
        info += String::formatInt( stats.numDirectionalLights ) + " dyn directional\n";
        info += String::formatInt( stats.numPointLights ) + " dyn point\n";
        info += "est " + String::formatInt( gpuStats.bytesInTextures ) + " B in textures\n";
        info += "est " + String::formatInt( gpuStats.bytesInMeshes ) + " B in meshes\n";

        if ( driverShared.haveAtiMeminfo )
        {
            GLint freeTexMem[4];
            glGetIntegerv( GL_TEXTURE_FREE_MEMORY_ATI, freeTexMem );

            info += String::formatInt( freeTexMem[0] / 1024 ) + " MiB ATi tex pool free\n";
            //info += String::formatInt( freeTexMem[1] / 1024 ) + " MiB ATi tex pool blk\n";
            info += String::formatInt( freeTexMem[2] / 1024 ) + " MiB ATi tex aux free\n";
            //info += String::formatInt( freeTexMem[3] / 1024 ) + " MiB ATi tex aux blk\n";
        }

        info += "\n";
        info += "total  " + String::formatInt( ( int ) profiling.frameTime ) + " us\n";
        info += "event  " + String::formatInt( ( int ) profiling.eventTime ) + " us\n";
        info += "scene  " + String::formatInt( ( int ) profiling.renderTime ) + " us\n";
        info += "swap   " + String::formatInt( ( int ) profiling.swapTime ) + " us\n";

        //statsFont->renderString( viewport.x - 12.0f, 12.0f, info, Colour::white(), IFont::right | IFont::top );
        statsFont->drawString( Vector2<>( viewport.x - 252.0f, 12.0f ), info, Colour::white(), IFont::left | IFont::top );
    }

    void OpenGlDriver::endDepthRendering()
    {
        // TODO: auch!

        /*if ( globalState.shadersEnabled )
        {
            SG_assert( false )
        }
        else*/
        {
            glCullFace( GL_BACK );
            glShadeModel( GL_SMOOTH );
            glColorMask( 1, 1, 1, 1 );
        }
    }

    unsigned OpenGlDriver::endPicking( const Vector2<unsigned>& samplePos )
    {
        unsigned id = 0;

        glReadPixels( samplePos.x, windowSize.y - samplePos.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &id );

        glEnable( GL_BLEND );
        glShadeModel( GL_SMOOTH );

        printf( "%u,%u->%08X\n", samplePos.x, samplePos.y, id );
        return id & 0xFFFFFF;
    }

    void OpenGlDriver::endShadowMapping()
    {
        //Disable textures and texgen
        glDisable(GL_TEXTURE_2D);

        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        glDisable(GL_TEXTURE_GEN_R);
        glDisable(GL_TEXTURE_GEN_Q);

        //Restore other states
        glDisable(GL_LIGHTING);
        glDisable(GL_ALPHA_TEST);
    }

    void OpenGlDriver::getDriverInfo( Info* info )
    {
        info->release = ( String ) "OpenGlDriver " + version + " " StormGraph_Target;
        info->renderer = ( const char* ) glGetString( GL_RENDERER );
    }

    IEventListener* OpenGlDriver::getEventListener()
    {
        return static_cast<IEventListener*>( this );
    }

    int16_t OpenGlDriver::getKey( const char* name )
    {
        if ( !name )
            return 0;

        for ( const Key* key = keys; key->name; key++ )
            if ( strcmp( name, key->name ) == 0 )
                return key->key;

        return tolower( *name );
    }

    String OpenGlDriver::getKeyName( int16_t code )
    {
        for ( const Key* key = keys; key->name; key++ )
            if ( key->key == code )
                return key->name;

        return ( char ) toupper( code );
    }

    Colour OpenGlDriver::getPickingColour( unsigned id )
    {
        int r = id & 0xFF, g = ( id >> 8 ) & 0xFF, b = ( id >> 16 ) & 0xFF;

        return Colour( r, g, b );
    }

    unsigned OpenGlDriver::getPickingId()
    {
        return nextPickingId++;
    }

    ShaderProgram* OpenGlDriver::getPickingShaderProgram()
    {
        if ( assets.pickingShaderProgram == nullptr )
        {
            ShaderProgramProperties properties;
            memset( &properties, 0, sizeof( properties ) );

            assets.pickingShaderProgram = new ShaderProgram( this, &properties );
        }

        return assets.pickingShaderProgram;
    }

    void OpenGlDriver::getProjectionInfo( IProjectionInfoBuffer* projectionInfoBuffer )
    {
        auto buffer = static_cast<ProjectionInfoBuffer*>( projectionInfoBuffer );

        glGetDoublev( GL_MODELVIEW_MATRIX, &buffer->modelView[0][0] );
        glGetDoublev( GL_PROJECTION_MATRIX, &buffer->projection[0][0] );
    }

    ShaderProgramSet* OpenGlDriver::getShaderProgramSet( ShaderProgramSetProperties* properties )
    {
        iterate ( shaderProgramSets )
            if ( shaderProgramSets.current()->matches( properties ) )
                return shaderProgramSets.current()->reference();

#ifdef li_MSW
        LARGE_INTEGER freq, begin, end;
        QueryPerformanceFrequency( &freq );

        QueryPerformanceCounter( &begin );
#endif

        Reference<ShaderProgramSet> set = new ShaderProgramSet( this, properties );
        set->init();

#ifdef li_MSW
        QueryPerformanceCounter( &end );
        printf( "in %g ms\n", ( end.QuadPart - begin.QuadPart ) * 1000.0 / ( double )freq.QuadPart );
#endif

        shaderProgramSets.add( set->reference() );
        return set.detach();
    }

    IMaterial* OpenGlDriver::getSolidMaterial()
    {
        /*if ( solidMaterial == nullptr )
        {
            MaterialProperties2 materialProperties;
            memset( &materialProperties, 0, sizeof( materialProperties ) );

            materialProperties.colour = Colour::white();

            solidMaterial = new Material( this, "OpenGlDriver/solidMaterial", &materialProperties );
        }
*/
        return solidMaterial->reference();
    }

    /*Texture* OpenGlDriver::getSolidTexture()
    {
        if ( solidTexture == nullptr )
            solidTexture = new Texture( "OpenGlDriver/solidTexture", Colour::white() );

        return solidTexture->reference();
    }*/

    Vector2<unsigned> OpenGlDriver::getViewportSize()
    {
        return viewport;
    }

    void OpenGlDriver::initOpenGl()
    {
        // TODO: how aboooooout.... some checks as we might be destorying an existing OpenGL context here ffs?

        globalState.currentCoordSource = nullptr;

        for ( size_t i = 0; i < MAX_UVS_PER_VERTEX; i++ )
            globalState.currentUvSource[i] = nullptr;

        gpuStats.bytesInTextures = 0;
        gpuStats.bytesInMeshes = 0;

        // Link OpenGl procedures

        String failedFor, missingEntries;

        // Various functionality
        driverShared.haveAtiMeminfo = haveExtension( "GL_ATI_meminfo" );
        driverShared.haveRenderBuffers = haveExtension( "GL_EXT_framebuffer_object" );
        driverShared.haveS3tc = haveExtension( "GL_EXT_texture_compression_s3tc" );
        driverShared.useShaders = haveExtension( "GL_ARB_shader_objects" ) && haveExtension( "GL_ARB_vertex_shader" ) && haveExtension( "GL_ARB_fragment_shader" ) && haveExtension( "GL_ARB_shading_language_100" );
        driverShared.useVaos = haveExtension( "GL_ARB_vertex_array_object" );
        //driverShared.useVaos = true;
        driverShared.useVertexBuffers = haveExtension( "GL_ARB_vertex_buffer_object" );
        features.depthTextures = haveExtension( "GL_ARB_depth_texture" );

        if ( forceNoShaders )
            driverShared.useShaders = false;

        if ( forceNoVbo )
            driverShared.useVertexBuffers = false;

        // Textures
        GLint maxTexture;
        glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maxTexture );

        driverShared.maxPo2Upscale = 512;
        driverShared.maxTextureSize = maxTexture;
        driverShared.requirePo2Textures = !haveExtension( "GL_ARB_texture_non_power_of_two" );
        driverShared.textureLod = 0;

        // Fragment processing (aka "shading")
        //driverShared.numLights = cfg_num_lights.isEmpty() ? -1 : ( int ) cfg_num_lights;

        // We did our best. But sometimes it's just not enough...
        if ( strstr( ( const char* ) glGetString( GL_RENDERER ), "RADEON 9600" ) != 0 )
        {
            driverShared.useShaders = false;            // pretends to support shaders, but fails to link them
            driverShared.requirePo2Textures = true;     // pretends to support NPOT, but horribly destroys those
        }

        for ( unsigned i = 0; i < sizeof( glLinkTable ) / sizeof( *glLinkTable ); i++ )
        {
            glApi.pointers[i] = 0;

            bool wanted = ( glLinkTable[i].flags & Gl_always )
                    || ( driverShared.useShaders && ( glLinkTable[i].flags & Gl_shaders ) )
                    || ( driverShared.useVaos && ( glLinkTable[i].flags & Gl_vaos ) )
                    || ( driverShared.useVertexBuffers && ( glLinkTable[i].flags & Gl_vbos ) )
                    || ( driverShared.haveRenderBuffers && ( glLinkTable[i].flags & Gl_renderbuffers ) );

            if ( !wanted )
                continue;

            // TODO: this definitely shouldn't be decided at runtime
            if ( !rendererOnly )
                glApi.pointers[i] = ( void ( * )() ) SDL_GL_GetProcAddress( glLinkTable[i].name );
            else {
#ifdef __li_MSW
                glApi.pointers[i] = ( void ( * )() ) wglGetProcAddress( glLinkTable[i].name );
#elif OpenGlDriver_With_GLX
                glApi.pointers[i] = ( void ( * )() ) glXGetProcAddress(
                        reinterpret_cast<const GLubyte*>( glLinkTable[i].name ) );
#else
                SG_assert4(!rendererOnly, "Renderer-only mode is currently not supported on this platform.");
#endif
            }

            if ( !glApi.pointers[i] )
            {
                missingEntries += ( String ) "<li>" + glLinkTable[i].name + "</li>\n";

                if ( glLinkTable[i].flags & Gl_optional )
                {
                    printf( "OpenGlDriver Warning: '%s' not available.\n", glLinkTable[i].name );
                    continue;
                }
                else if ( glLinkTable[i].flags & Gl_vbos )
                    driverShared.useVertexBuffers = false;
                else if ( glLinkTable[i].flags & Gl_renderbuffers )
                    driverShared.haveRenderBuffers = false;
                else if ( glLinkTable[i].flags & Gl_shaders )
                    driverShared.useShaders = false;

                if ( failedFor.isEmpty() )
                    failedFor = glLinkTable[i].name;
            }
        }

        if ( !failedFor.isEmpty() ) {
#ifdef __li_MSW
            MessageBoxA( 0, "Failed to link OpenGL entry `" + failedFor + "`!\n\nSome functionality might be disabled."
                    " You might try to update your graphics drivers to fix this problem (OpenGL 2.0 is required). See the application log for more details.", "StormGraph Engine", MB_ICONWARNING );
#endif
        }

        if ( !missingEntries.isEmpty() )
            Common::logEvent( "OpenGlDriver.OpenGlDriver", "Failed to link one or more OpenGL entries:\n<ul>" + missingEntries + "</ul>" );

        if ( glApi.functions.glCompressedTexImage2DARB == nullptr )
            driverShared.haveS3tc = false;

        #define test( value_ ) ( ( value_ ) ? "<span style=\"color: #080\">yes</span>" : "<b style=\"color: #f00\">no</b>" )

        Common::logEvent( "OpenGlDriver.OpenGlDriver", ( String ) "Initializing OpenGlDriver!\n"
                + "&nbsp;&nbsp;<b>OpenGL renderer</b>: " + ( const char* ) glGetString( GL_RENDERER ) + "\n"
                + "&nbsp;&nbsp;<b>OpenGL version</b>: "+ ( const char* ) glGetString( GL_VERSION ) + "\n"
                + "&nbsp;&nbsp;<b>Renderer vendor</b>: " + ( const char* ) glGetString( GL_VENDOR ) + "\n"
                + "&nbsp;&nbsp;<b>GLSL version</b>: " + ( const char* ) glGetString( GL_SHADING_LANGUAGE_VERSION ) + "\n"
                + "&nbsp;&nbsp;<b>Have Shader Programs</b>: " + test( driverShared.useShaders ) + "\n"
                + "&nbsp;&nbsp;<b>Have Render Buffers</b>: " + test( driverShared.haveRenderBuffers ) + "\n"
                + "&nbsp;&nbsp;<b>Have S3 Texture Compression</b>: " + test( driverShared.haveS3tc ) + "\n"
                + "&nbsp;&nbsp;<b>Have GL_ATI_meminfo</b>: " + test( driverShared.haveAtiMeminfo ) + "\n"
                + "&nbsp;&nbsp;<b>Have Vertex Array Objects</b>: " + test( driverShared.useVaos ) + "\n"
                + "&nbsp;&nbsp;<b>Have Vertex Buffers</b>: " + test( driverShared.useVertexBuffers ) + "\n"
                + "&nbsp;&nbsp;<b>NPOT textures supported</b>: " + test( !driverShared.requirePo2Textures ) + "\n"
                + "&nbsp;&nbsp;<b>Depth textures supported</b>: " + test( features.depthTextures ) + "\n"
                + "&nbsp;&nbsp;<b>Max texture dimension</b>: " + driverShared.maxTextureSize + " px\n" );

        glGetIntegerv( GL_MAX_LIGHTS, &driverShared.maxFixedLights );

        memset( &renderState, 0, sizeof( renderState ) );
        currentRenderBuffer = nullptr;

        setSceneAmbient( Colour( 1.0f, 1.0f, 1.0f ) );

        features.shaders = driverShared.useShaders;
        globalState.shadersEnabled = features.shaders;

        // Sum stock obj's
        {
            MaterialProperties2 materialProperties;
            memset( &materialProperties, 0, sizeof( materialProperties ) );

            materialProperties.colour = Colour::white();

            solidMaterial = new Material( this, "OpenGlDriver.solidMaterial", &materialProperties, true );
        }

        {
            MaterialProperties2 materialProperties;
            memset( &materialProperties, 0, sizeof( materialProperties ) );

            materialProperties.colour = Colour::white();
            materialProperties.numTextures = 1;

            texturedMaterial = new Material( this, "OpenGlDriver.texturedMaterial", &materialProperties, true );
        }

        // 1x1 texture plane
        PlaneCreationInfo plane( Vector2<>( 1.0f, 1.0f ), Vector<>(), Vector2<>(), Vector2<>( 1.0f, 1.0f ), false, true, /*planeRenderMaterial->reference()*/ nullptr );
        plane1x1uv = Mesh::createPlane( this, &plane, IModel::fullStatic );

        // Line
        const float lineCoords[] = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };
        MeshCreationInfo3 lineCreationInfo = { MeshFormat::lineList, MeshLayout::linear, nullptr, 2, 0, lineCoords, nullptr, {}, nullptr, nullptr };
        line = new Mesh( this, &lineCreationInfo, IModel::fullStatic, true );

        // Rectangle Outline
        const float rectCoords[] = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        MeshCreationInfo3 rectCreationInfo = { MeshFormat::lineStrip, MeshLayout::linear, nullptr, 5, 0, rectCoords, nullptr, {}, nullptr, nullptr };
        rect1x1 = new Mesh( this, &rectCreationInfo, IModel::fullStatic, true );

        // We're done here, thanks for everything
        Common::logEvent( "OpenGlDriver.OpenGlDriver", "OpenGL initialization successful." );

        driverShared.isGlInit = true;
    }

    bool OpenGlDriver::isRunning()
    {
        SG_assert3( eventListener != nullptr, "OpenGlDriver.OpenGlDriver.isRunning" )

        return eventListener->isRunning();
    }

    void OpenGlDriver::onCloseButton()
    {
        if ( eventListener )
            eventListener->onCloseButton();
    }

    void OpenGlDriver::onFrameBegin()
    {
        if ( profiling.interval == 0 )
            profiling.profiler->reenter( profiling.frame );

        globalState.numDirectionalLights = 0;
        globalState.numPointLights = 0;
        nextLightId = 0;

        memset( &stats, 0, sizeof( stats ) );

        setRenderBuffer( nullptr );
        clear();

        if ( eventListener )
            eventListener->onFrameBegin();
    }

    void OpenGlDriver::onFrameEnd()
    {
        if ( eventListener )
            eventListener->onFrameEnd();

        //checkErrors( "onFrameEnd" );
    }

    void OpenGlDriver::onKeyState( int16_t key, StormGraph::Key::State state, Unicode::Char character )
    {
        if ( eventListener )
            eventListener->onKeyState( key, state, character );
    }

    void OpenGlDriver::onMouseMoveTo( const Vector2<int>& mouse )
    {
        if ( eventListener )
            eventListener->onMouseMoveTo( mouse );
    }

    void OpenGlDriver::onRender()
    {
        if ( eventListener )
            eventListener->onRender();
    }

    /*void OpenGlDriver::onSelectProgram( ShaderProgram* program )
    {
        currentShader = program;
    }*/

    void OpenGlDriver::onViewportResize( const Vector2<unsigned>& dimensions )
    {
        viewport = dimensions;

        if ( currentRenderBuffer == nullptr )
            glViewport( viewportPos.x, windowSize.y - viewportPos.y - viewport.y, viewport.x, viewport.y );

        if ( eventListener )
            eventListener->onViewportResize( dimensions );
    }

    void OpenGlDriver::popBlendMode()
    {
        setBlendMode( blendModes.pop() );
    }

    void OpenGlDriver::popClippingRect()
    {
        clippingRects.pop();

        if ( !clippingRects.isEmpty() )
            setClippingRect( clippingRects.top() );
        else
            glDisable( GL_SCISSOR_TEST );
    }

    void OpenGlDriver::popProjection()
    {
        glMatrixMode( GL_PROJECTION );
        glPopMatrix();

        glMatrixMode( GL_MODELVIEW );
        glPopMatrix();
    }

    void OpenGlDriver::popRenderBuffer()
    {
        setRenderBuffer( renderBuffers.pop() );
    }

    ITexturePreload* OpenGlDriver::preloadTextureFromStream( SeekableInputStream* input, const char* name, ILodFunction* lodFunction )
    {
        Object<Image> image = createImageFromStream( input );

        if ( image == nullptr )
            throw StormGraph::Exception( "OpenGlDriver.OpenGlDriver.preloadTextureFromStream", "TextureLoadError",
                    ( String ) "Failed to preload texture `" + name + "`. The file is not a well-formed image." );

        return new TexturePreload( this, name, image.detach(), lodFunction );
    }

    IModelPreload* OpenGlDriver::preloadModelFromMemory( const char* name, MeshCreationInfo2* meshes, size_t count, unsigned flags )
    {
        IModelPreload* preload = ModelPreload::createFromMemory( this, name, meshes, count, flags );

        SG_assert3( preload != nullptr, "OpenGlDriver.OpenGlDriver.preloadModelFromMemory" )

        return preload;
    }

    IModelPreload* OpenGlDriver::preloadModelFromMemory( const char* name, MeshCreationInfo2** meshes, size_t count, unsigned flags )
    {
        IModelPreload* preload = ModelPreload::createFromMemory( this, name, meshes, count, flags );

        SG_assert3( preload != nullptr, "OpenGlDriver.OpenGlDriver.preloadModelFromMemory" )

        return preload;
    }

    void OpenGlDriver::pushBlendMode( BlendMode blendMode )
    {
        blendModes.push( currentBlendMode );

        setBlendMode( blendMode );
    }

    void OpenGlDriver::pushClippingRect( const ScreenRect& clippingRect )
    {
        setClippingRect( clippingRect );

        clippingRects.push( clippingRect );
    }

    void OpenGlDriver::pushRenderBuffer( IRenderBuffer* renderBuffer )
    {
        renderBuffers.push( currentRenderBuffer );

        setRenderBuffer( ( RenderBuffer* ) renderBuffer );
    }

    void OpenGlDriver::pushProjection()
    {
        glMatrixMode( GL_PROJECTION );
        glPushMatrix();

        glMatrixMode( GL_MODELVIEW );
        glPushMatrix();
    }

    void OpenGlDriver::render2d( Texture* texture, float x, float y, float w, float h, float u0, float v0, float u1, float v1, const Colour& blend )
    {
        //printf( "render2d( %p, %g, %g, %g, %g, %g, %g, %g, %g );\n", texture, x, y, w, h, u0, v0, u1, v1 );

        /*planeRenderMaterial->setColour( blend );
        plane1x1uv->setMaterial( planeRenderMaterial->reference() );

        glPushMatrix();
        glTranslatef( x, y, 0.0f );
        glScalef( w, h, 1.0f );

        glMatrixMode( GL_TEXTURE );
        glPushMatrix();
        glTranslatef( u0, v0, 0.0f );
        glScalef( u1 - u0, v1 - v0, 1.0f );

        plane1x1uv->render();

        glPopMatrix();

        glMatrixMode( GL_MODELVIEW );
        glPopMatrix();*/
    }

    /*void OpenGlDriver::renderPlane2d( const Vector<>& origin, const Vector2<>& dimensions, const Vector2<>& uv0, const Vector2<>& uv1, IMaterial* material )
    {
        plane1x1uv->setMaterial( ( Material* ) material->reference() );

        glPushMatrix();
        glTranslatef( origin.x, origin.y, origin.z );
        glScalef( dimensions.x, dimensions.y, 1.0f );

        glMatrixMode( GL_TEXTURE );
        glPushMatrix();
        glTranslatef( uv0.x, uv0.y, 0.0f );
        glScalef( uv1.x - uv0.x, uv1.y - uv0.y, 1.0f );

        plane1x1uv->render();

        glPopMatrix();

        glMatrixMode( GL_MODELVIEW );
        glPopMatrix();
    }*/

    void OpenGlDriver::runMainLoop( IEventListener* eventListener )
    {
        SG_assert3( eventSource != nullptr, "OpenGlDriver.OpenGlDriver.runMainLoop" )

        this->eventListener = eventListener;

        while ( eventListener->isRunning() )
            eventSource->processEvents( this );

        this->eventListener = 0;
    }

    void OpenGlDriver::set2dMode( float nearZ, float farZ )
    {
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();

        if ( !currentRenderBuffer )
            glOrtho( 0.0, viewport.x, viewport.y, 0.0, nearZ, farZ );
        else
            glOrtho( 0.0, currentRenderBuffer->getWidth(), currentRenderBuffer->getHeight(), 0.0, nearZ, farZ );

        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();
    }

    void OpenGlDriver::set3dMode( float nearZ, float farZ )
    {
        /*glMatrixMode( GL_PROJECTION );
        glLoadIdentity();

        const GLfloat viewportRatio = ( currentRenderBuffer == nullptr )
                ? ( GLfloat )( viewport.x ) / ( GLfloat )( viewport.y )
                : ( GLfloat )( currentRenderBuffer->getWidth() ) / ( GLfloat )( currentRenderBuffer->getHeight() );

        gluPerspective( 45.0f, viewportRatio, nearZ, farZ );
        frustum.setProjection( 45.0f, viewportRatio, nearZ, farZ );

        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();*/

        setPerspectiveProjection( nearZ, farZ, 45.0f );
    }

    void OpenGlDriver::setBlendMode( BlendMode blendMode )
    {
        currentBlendMode = blendMode;

        switch ( blendMode )
        {
            case normal:
                if ( glApi.functions.glBlendEquationSeparate )
                    glApi.functions.glBlendEquationSeparate( GL_FUNC_ADD, GL_MAX );

			    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
                break;

            case additive:
                if ( glApi.functions.glBlendEquationSeparate )
                    glApi.functions.glBlendEquationSeparate( GL_FUNC_ADD, GL_FUNC_ADD );

			    glBlendFunc( GL_SRC_ALPHA, GL_ONE );
                break;

            case subtractive:
                if ( glApi.functions.glBlendEquationSeparate )
                    glApi.functions.glBlendEquationSeparate( GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD );

			    glBlendFunc( GL_SRC_ALPHA, GL_ONE );
                break;

            /*case Blend_invert:
                if ( glFs.glBlendEquationSeparate )
                    glFs.glBlendEquationSeparate( GL_FUNC_ADD, GL_FUNC_ADD );

			    glBlendFunc( GL_ZERO, GL_ONE_MINUS_DST_COLOR );
                break;*/

            default:
                ;
        }
    }

    void OpenGlDriver::setCamera( const Camera* camera )
    {
        //setCamera( camera->getEyePos(), camera->getCenterPos(), camera->getUpVector() );
        setCamera( camera->eye, camera->center, camera->up );
    }

    void OpenGlDriver::setCamera( const Vector<float>& eye, const Vector<float>& center, const Vector<float>& up )
    {
        currentCamera = eye;
        currentUp = up;

        // Our glorious hack to get D3D-like coordinates
        // Thank me later
        // Source: ubuntu forums actually
        //glLoadIdentity();
        //gluLookAt( -eye.x, eye.y, eye.z, -center.x, center.y, center.z, -up.x, up.y, up.z );
        //glScalef( -1.f, 1.f, 1.f );

        //glm::mat4 modelView = glm::lookAt( glm::vec3( -eye.x, eye.y, eye.z ), glm::vec3( -center.x, center.y, center.z ), glm::vec3( -up.x, up.y, up.z ) );
        //modelView = glm::scale( modelView, glm::vec3( -1.0f, 1.0f, 1.0f ) );

        modelView = glm::lookAt( glm::vec3( eye.x, -eye.y, eye.z ), glm::vec3( center.x, -center.y, center.z ), glm::vec3( up.x, -up.y, up.z ) );
        modelView = glm::scale( modelView, glm::vec3( 1.0f, -1.0f, 1.0f ) );

        glLoadMatrixf( &modelView[0][0] );

        frustum.setView( eye, center, up );
    }

    void OpenGlDriver::setClearColour( const Colour& colour )
    {
        glClearColor( colour.r, colour.g, colour.b, colour.a );
    }

    void OpenGlDriver::setClippingRect( const ScreenRect& clippingRect )
    {
        glEnable( GL_SCISSOR_TEST );
        glScissor( clippingRect.pos.x, windowSize.y - clippingRect.pos.y - clippingRect.size.y, clippingRect.size.x, clippingRect.size.y );
    }

    void OpenGlDriver::setDisplayMode( DisplayMode* displayMode )
    {
        if ( !rendererOnly )
        {
            SG_assert( displayMode->width > 0 )
            SG_assert( displayMode->height > 0 )

            SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
            SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
            SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );

            SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );

#ifndef __linux__
            if ( displayMode->multisamplingLevel )
            {
                SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
                SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, displayMode->multisamplingLevel );
            }
#endif

            SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, displayMode->vsync );
            //SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

            viewport.x = displayMode->width;
            viewport.y = displayMode->height;

            windowSize = viewport;

            unsigned flags = SDL_OPENGL;

            if ( displayMode->fullscreen )
                flags |= SDL_FULLSCREEN;
            else if ( displayMode->resizable )
                flags |= SDL_RESIZABLE;

            SDL_Surface* display;

            SDL_WM_SetCaption( displayMode->windowTitle, "A" );

            if ( !( display = SDL_SetVideoMode( viewport.x, viewport.y, 32, flags ) ) )
                throw Exception( "OpenGlDriver.OpenGlDriver.setDisplayMode", "GfxInitError", "SDL/OpenGL subsystem initialization failed: " + (String)SDL_GetError() );

            auto se = new SdlEventSource( display, profiling );
            eventSource = se;
            eventInput = se;

            //engine->setVariable( "displayResolution", viewport.toString() );
        }

        if ( !driverShared.isGlInit )
            initOpenGl();

        // Enable blending
        glEnable( GL_BLEND );

        blendModes.clear();
        setBlendMode( normal );

        glEnable( GL_ALPHA_TEST );
        glAlphaFunc( GL_GREATER, 0.01f );

        // Configure depth tesing
        glDepthFunc( GL_LEQUAL );

        // Use nice perspective correction
        glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

        setRenderFlag( RenderFlag::culling, true );
        setRenderFlag( RenderFlag::depthTest, true );
        //setRenderFlag( RenderFlag::wireframe, false ); -- by default

        int error = glGetError();

        if ( error != GL_NO_ERROR )
            throw Exception( "OpenGlDriver.OpenGlDriver.setDisplayMode", "GfxInitError", ( String )"OpenGL set-up failed: error " + error );

        //if ( !driverShared.useShaders )
        //    glEnable( GL_LIGHTING );
    }

    void OpenGlDriver::setEventSource( IEventSource* eventSource )
    {
        this->eventSource = eventSource;
    }

    void OpenGlDriver::setLevelOfDetail( LevelOfDetail* lod )
    {
        driverShared.textureLod = lod->textureLodLevel;
    }

    void OpenGlDriver::setOrthoProjection( const Vector2<float>& leftRight, const Vector2<float>& topBottom, const Vector2<float>& nearFar )
    {
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();

        glOrtho( leftRight.x, leftRight.y, topBottom.x, topBottom.y, nearFar.x, nearFar.y );

        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();
    }

    /*void OpenGlDriver::setParameter( const String& name, const String& value )
    {
        if ( name == "rendererOnly" )
            rendererOnly = true;
        else if ( name == "forceNoShaders" )
            forceNoShaders = value.toBool();
        else if ( name == "forceNoVbo" )
            forceNoVbo = value.toBool();

        if ( mode == Mode_default )
        {
            if ( name == "lights" )
                driverShared.numLights = value;
        }
        else if ( mode == Mode_legacy )
        {
            if ( name == "maxtex" )
                driverShared.maxTextureSize = value;
            else if ( name == "npot" )
                driverShared.requirePo2Textures = ! ( unsigned ) value;
            else if ( name == "npotmax" )
                driverShared.maxPo2Upscale = value;
        }
    }*/

    void OpenGlDriver::setPerspectiveProjection( float nearZ, float farZ, float fov )
    {
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();

        const GLfloat viewportRatio = ( currentRenderBuffer == nullptr )
                ? ( GLfloat )( viewport.x ) / ( GLfloat )( viewport.y )
                : ( GLfloat )( currentRenderBuffer->getWidth() ) / ( GLfloat )( currentRenderBuffer->getHeight() );

        gluPerspective( fov, viewportRatio, nearZ, farZ );
        frustum.setProjection( fov, viewportRatio, nearZ, farZ );

        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();
    }

    void OpenGlDriver::setPointLightShadowMap( unsigned index, ITexture* depthMap, const glm::mat4& shadowMapMatrix )
    {
        globalState.pointShadowMap[index] = static_cast<Texture*>( depthMap );
        globalState.pointShadowMatrix[index] = shadowMapMatrix;

        if ( renderState.currentShaderProgram != nullptr )
            renderState.currentShaderProgram->setPointLightShadowMap( index, static_cast<Texture*>( depthMap )->texture, shadowMapMatrix );
    }

    void OpenGlDriver::setProjection( const glm::mat4& projection )
    {
        glMatrixMode( GL_PROJECTION );
        glLoadMatrixf( &projection[0][0] );
        glMatrixMode( GL_MODELVIEW );
    }

    void OpenGlDriver::setRenderBuffer( RenderBuffer* renderBuffer )
    {
        if ( currentRenderBuffer == renderBuffer )
            return;

        if ( renderBuffer )
        {
            glApi.functions.glBindFramebuffer( GL_FRAMEBUFFER_EXT, renderBuffer->buffer );

            if ( renderBuffer->isSetUp() )
                glViewport( 0, 0, renderBuffer->getWidth(), renderBuffer->getHeight() );
        }
        else
        {
            glApi.functions.glBindFramebuffer( GL_FRAMEBUFFER_EXT, 0 );
            glViewport( viewportPos.x, windowSize.y - viewportPos.y - viewport.y, viewport.x, viewport.y );
        }

        currentRenderBuffer = renderBuffer;
    }

    void OpenGlDriver::setRenderFlag( RenderFlag flag, bool value )
    {
        switch ( flag )
        {
            case RenderFlag::culling:
                if ( value == true )
                    glEnable( GL_CULL_FACE );
                else
                    glDisable( GL_CULL_FACE );
                break;

            case RenderFlag::depthTest:
                if ( value == true )
                    glEnable( GL_DEPTH_TEST );
                else
                    glDisable( GL_DEPTH_TEST );
                break;

            case RenderFlag::wireframe:
                if ( value == true )
                    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
                else
                    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
                break;

            default:
                ;
        }
    }

    void OpenGlDriver::setSceneAmbient( const Colour& colour )
    {
        if ( driverShared.useShaders )
        {
            globalState.sceneAmbient = colour;

            if ( renderState.currentShaderProgram != nullptr )
                renderState.currentShaderProgram->setSceneAmbient( globalState.sceneAmbient );
        }
        else
        {
            GLfloat ambient[] = { colour.r, colour.g, colour.b, colour.a };
            glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambient );
        }
    }

    void OpenGlDriver::beginShadowMapping( ITexture* shadowTexture, const glm::mat4& textureMatrix )
    {
        glApi.functions.glActiveTexture( GL_TEXTURE2 );
        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, static_cast<Texture*>( shadowTexture )->texture );

        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
        glTexGenfv(GL_S, GL_EYE_PLANE, &textureMatrix[0][0] );
        glEnable(GL_TEXTURE_GEN_S);

        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
        glTexGenfv(GL_T, GL_EYE_PLANE, &textureMatrix[1][0] );
        glEnable(GL_TEXTURE_GEN_T);

        glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
        glTexGenfv(GL_R, GL_EYE_PLANE, &textureMatrix[2][0] );
        glEnable(GL_TEXTURE_GEN_R);

        glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
        glTexGenfv(GL_Q, GL_EYE_PLANE, &textureMatrix[3][0] );
        glEnable(GL_TEXTURE_GEN_Q);

        //Enable shadow comparison
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE);

        //Shadow comparison should be true (ie not in shadow) if r<=texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);

        //Shadow comparison should generate an INTENSITY result
        glTexParameteri( GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_INTENSITY );

        glAlphaFunc( GL_GEQUAL, 0.99f );
        glEnable( GL_ALPHA_TEST );
    }

    void OpenGlDriver::setViewport( const Vector2<int>& pos, const Vector2<unsigned>& size, const Vector2<unsigned>& windowSize )
    {
        //printf( "Will SW @ [%s] of [%s] in [%s] window\n", pos.toString().c_str(), size.toString().c_str(), windowSize.toString().c_str() );

        viewportPos = pos;
        this->windowSize = windowSize;

        glViewport( pos.x, windowSize.y - pos.y - size.y, size.x, size.y );

        onViewportResize( size );
    }

    void OpenGlDriver::setViewTransform( const glm::mat4& viewTransform )
    {
        glLoadMatrixf( &viewTransform[0][0] );
    }

    void OpenGlDriver::startup()
    {
        if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE ) == -1 )
            throw Exception( "OpenGlDriver.OpenGlDriver.startup", "GfxInitError", "SDL core initialization failed. No idea why." );

        SDL_EnableUNICODE( 1 );

#ifdef Use_Sdl_Ttf
        if ( TTF_Init() == -1 )
            throw Exception( "OpenGlDriver.OpenGlDriver.startup", "GfxInitError", "SDL_ttf library initialization failed. No idea why." );
#endif

        driverShared.isGlInit = 0;
    }

    void OpenGlDriver::unload()
    {
        assets.pickingShaderProgram.release();

        plane1x1uv.release();
        line.release();
        rect1x1.release();

        solidMaterial.release();
        texturedMaterial.release();
        statsFont.release();

        Font::exitFreeType();
        SDL_Quit();

        memset( &renderState, 0, sizeof( renderState ) );
    }

    bool OpenGlDriver::unproject( const Vector2<float>& windowSpace, Vector<float>& worldSpace, IProjectionInfoBuffer* projectionInfoBuffer )
    {
        Vector<double> pos;

        if ( projectionInfoBuffer == nullptr )
        {
            GLint viewport[4];
            GLdouble modelView[16], projection[16];

            glGetDoublev( GL_MODELVIEW_MATRIX, modelView );
            glGetDoublev( GL_PROJECTION_MATRIX, projection );
            glGetIntegerv( GL_VIEWPORT, viewport );

            Vector<float> win( windowSpace.x, ( float ) viewport[3] - windowSpace.y, 0.0f );

            glReadPixels( ( int ) win.x, ( int ) win.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &win.z );

            if ( win.z >= 1.0 )
                return false;

            if ( gluUnProject( win.x, win.y, win.z, modelView, projection, viewport, &pos.x, &pos.y, &pos.z ) != GL_TRUE )
                return false;
        }
        else
        {
            auto buffer = static_cast<ProjectionInfoBuffer*>( projectionInfoBuffer );

            Vector<float> win( windowSpace.x, ( float ) buffer->viewport[3] - windowSpace.y, 0.0f );

            glReadPixels( ( int ) win.x, ( int ) win.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &win.z );

            if ( win.z >= 1.0 )
                return false;

            if ( gluUnProject( win.x, win.y, win.z, &buffer->modelView[0][0], &buffer->projection[0][0], buffer->viewport, &pos.x, &pos.y, &pos.z ) != GL_TRUE )
                return false;
        }

        worldSpace = pos;

        return true;
    }

    Vector<float> OpenGlDriver::unproject( const Vector<>& windowSpace, IProjectionInfoBuffer* projectionInfoBuffer )
    {
        Vector<double> pos;

        if ( projectionInfoBuffer == nullptr )
        {
            GLint viewport[4];
            GLdouble modelView[16], projection[16];

            glGetDoublev( GL_MODELVIEW_MATRIX, modelView );
            glGetDoublev( GL_PROJECTION_MATRIX, projection );
            glGetIntegerv( GL_VIEWPORT, viewport );

            gluUnProject2( windowSpace.x, -windowSpace.y, windowSpace.z, modelView, projection, viewport, &pos.x, &pos.y, &pos.z );
        }
        else
        {
            auto buffer = static_cast<ProjectionInfoBuffer*>( projectionInfoBuffer );

            gluUnProject2( windowSpace.x, -windowSpace.y, windowSpace.z, &buffer->modelView[0][0], &buffer->projection[0][0], buffer->viewport, &pos.x, &pos.y, &pos.z );
        }

        return pos;
    }

    SdlEventSource::SdlEventSource( SDL_Surface* display, OpenGlDriver::Profiling& profiling ) : display( display ), profiling( profiling )
    {
    }

    SdlEventSource::~SdlEventSource()
    {
    }

    void SdlEventSource::processEvents( IEventListener* eventListener )
    {
        SDL_Event event;

        eventListener->onFrameBegin();

        if ( profiling.interval == 0 )
            profiling.profiler->reenter( profiling.event );

        if ( SDL_PollEvent( &event ) )
        {
            switch ( event.type )
            {
                case SDL_KEYDOWN:
                    if ( event.key.keysym.sym == SDLK_ESCAPE )
                        eventListener->onCloseButton();

                    eventListener->onKeyState( event.key.keysym.sym, StormGraph::Key::pressed, event.key.keysym.unicode );
                    break;

                case SDL_KEYUP:
                    eventListener->onKeyState( event.key.keysym.sym, StormGraph::Key::released, event.key.keysym.unicode );
                    break;

                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEBUTTONDOWN:
                {
                    StormGraph::Key::State state = ( event.type == SDL_MOUSEBUTTONDOWN ) ? StormGraph::Key::pressed : StormGraph::Key::released;

                    if ( event.button.button == SDL_BUTTON_LEFT )
                    {
                        eventListener->onKeyState( Key::anyMouse, state, 0 );
                        eventListener->onKeyState( Key::leftMouse, state, 0 );
                    }
                    else if ( event.button.button == SDL_BUTTON_RIGHT )
                    {
                        eventListener->onKeyState( Key::anyMouse, state, 0 );
                        eventListener->onKeyState( Key::rightMouse, state, 0 );
                    }
                    else if ( event.button.button == SDL_BUTTON_WHEELUP && event.type == SDL_MOUSEBUTTONDOWN )
                        eventListener->onKeyState( Key::mouseWheelUp, state, 0 );
                    else if ( event.button.button == SDL_BUTTON_WHEELDOWN && event.type == SDL_MOUSEBUTTONDOWN )
                        eventListener->onKeyState( Key::mouseWheelDown, state, 0 );
                    break;
                }

                case SDL_MOUSEMOTION:
                    eventListener->onMouseMoveTo( Vector2<unsigned>( event.motion.x, event.motion.y ) );
                    break;

                case SDL_VIDEORESIZE:
                    eventListener->onViewportResize( Vector2<unsigned>( event.resize.w, event.resize.h ) );
                    break;

                case SDL_QUIT:
                    eventListener->onCloseButton();
                    break;
            }
        }

        if ( profiling.interval == 0 )
        {
            profiling.profiler->leave( profiling.event );
            profiling.eventTime = profiling.profiler->getDelta( profiling.event );

            profiling.profiler->reenter( profiling.render );
            eventListener->onRender();
            profiling.profiler->leave( profiling.render );
            profiling.renderTime = profiling.profiler->getDelta( profiling.render );

            eventListener->onFrameEnd();

            profiling.profiler->reenter( profiling.swap );
            SDL_GL_SwapBuffers();
            SDL_Delay(16);
            profiling.profiler->leave( profiling.swap );
            profiling.swapTime = profiling.profiler->getDelta( profiling.swap );

            profiling.profiler->leave( profiling.frame );
            profiling.frameTime = profiling.profiler->getDelta( profiling.frame );

            printf( "scene  %u\n", unsigned( profiling.renderTime ) );
            profiling.interval = 1000;
        }
        else
        {
            eventListener->onRender();

            eventListener->onFrameEnd();

            SDL_GL_SwapBuffers();
            SDL_Delay(16);
        }

        profiling.interval--;
    }

    void SdlEventSource::ReceiveEvents( IEventReceiver* receiver )
    {
        SDL_Event event;

        if ( SDL_PollEvent( &event ) )
        {
            switch ( event.type )
            {
                case SDL_KEYDOWN:
                    /*if ( event.key.keysym.sym == SDLK_ESCAPE )
                        eventListener->onCloseButton();

                    eventListener->onKeyState( event.key.keysym.sym, StormGraph::Key::pressed, event.key.keysym.unicode );*/
                    break;

                case SDL_KEYUP:
                    //eventListener->onKeyState( event.key.keysym.sym, StormGraph::Key::released, event.key.keysym.unicode );
                    break;

                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEBUTTONDOWN:
                {
                    /*StormGraph::Key::State state = ( event.type == SDL_MOUSEBUTTONDOWN ) ? StormGraph::Key::pressed : StormGraph::Key::released;

                    if ( event.button.button == SDL_BUTTON_LEFT )
                    {
                        eventListener->onKeyState( Key::anyMouse, state, 0 );
                        eventListener->onKeyState( Key::leftMouse, state, 0 );
                    }
                    else if ( event.button.button == SDL_BUTTON_RIGHT )
                    {
                        eventListener->onKeyState( Key::anyMouse, state, 0 );
                        eventListener->onKeyState( Key::rightMouse, state, 0 );
                    }
                    else if ( event.button.button == SDL_BUTTON_WHEELUP && event.type == SDL_MOUSEBUTTONDOWN )
                        eventListener->onKeyState( Key::mouseWheelUp, state, 0 );
                    else if ( event.button.button == SDL_BUTTON_WHEELDOWN && event.type == SDL_MOUSEBUTTONDOWN )
                        eventListener->onKeyState( Key::mouseWheelDown, state, 0 );*/
                    break;
                }

                case SDL_MOUSEMOTION:
                    //eventListener->onMouseMoveTo( Vector2<unsigned>( event.motion.x, event.motion.y ) );
                    break;

                case SDL_VIDEORESIZE:
                    //eventListener->onViewportResize( Vector2<unsigned>( event.resize.w, event.resize.h ) );
                    break;

                case SDL_QUIT:
                    receiver->ReceiveEvent( synthVKeyEvent( VKEY_TRIG, V_CLOSE ) );
                    break;
            }
        }
    }

    void OpenGlDriver::beginFrame()
    {
        eventInput->ReceiveEvents( this );

        clear();
    }

    void OpenGlDriver::endFrame()
    {
        SDL_GL_SwapBuffers();
    }

    Event_t* OpenGlDriver::getEvent()
    {
        if ( !eventQueue.isEmpty() )
            return eventQueue.popPtr();
        else
            return nullptr;
    }

    void OpenGlDriver::ReceiveEvent( const Event_t& event )
    {
        eventQueue.push( event );
    }
}
