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

#include <StormGraph/GraphicsDriver.hpp>
#include <StormGraph/Profiler.hpp>
#include <StormGraph/IO/Bsp.hpp>

#include <StormGraph/VisualInterface.hpp>

#include <SDL.h>
#include <SDL_opengl.h>

#ifdef Use_Sdl_Ttf
#include <SDL_ttf.h>
#else
// TODO: remove
struct FtFont;
#endif

#include <StormGraph/Image.hpp>

#include <glm/glm.hpp>

#include <littl/Stack.hpp>

#ifdef _MSC_VER
#pragma warning ( disable : 4200 )
#endif

// TODO: remove MeshPreload

namespace OpenGlDriver
{
    using namespace StormGraph;
    using namespace StormRender;

    static const size_t MAX_DIRECTIONAL_LIGHTS = 1;
    static const size_t MAX_POINT_LIGHTS = 4;

    static const size_t MAX_UVS_PER_VERTEX = TEXTURES_PER_VERTEX + 1;

    class Mesh;
    class OpenGlDriver;
    class Texture;

    union OpenGlApi
    {
        struct
        {
            PFNGLACTIVETEXTUREPROC glActiveTexture;
            PFNGLATTACHSHADERPROC glAttachShader;
            PFNGLBINDBUFFERPROC glBindBuffer;
            PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebuffer;
            PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbuffer;
            PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate;
            PFNGLBUFFERDATAPROC glBufferData;
            PFNGLBUFFERSUBDATAPROC glBufferSubData;
            PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatus;
            PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
            PFNGLCOMPILESHADERPROC glCompileShader;
            PFNGLCOMPRESSEDTEXIMAGE2DARBPROC glCompressedTexImage2DARB;
            PFNGLCREATEPROGRAMPROC glCreateProgram;
            PFNGLCREATESHADERPROC glCreateShader;
            PFNGLDELETEBUFFERSPROC glDeleteBuffers;
            PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffers;
            PFNGLDELETEPROGRAMPROC glDeleteProgram;
            PFNGLDELETESHADERPROC glDeleteShader;
            PFNGLDETACHSHADERPROC glDetachShader;
            PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbuffer;
            PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2D;
            PFNGLGENBUFFERSPROC glGenBuffers;
            PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffers;
            PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffers;
            PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
            PFNGLGETPROGRAMIVPROC glGetProgramiv;
            PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
            PFNGLGETSHADERIVPROC glGetShaderiv;
            PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
            PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
            PFNGLLINKPROGRAMPROC glLinkProgram;
            PFNGLMAPBUFFERPROC glMapBuffer;
            PFNGLGETBUFFERSUBDATAPROC glGetBufferSubData;
            PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorage;
            PFNGLSHADERSOURCEPROC glShaderSource;
            PFNGLUNIFORM1IPROC glUniform1i;
            PFNGLUNIFORM1FPROC glUniform1f;
            PFNGLUNIFORM2FPROC glUniform2f;
            PFNGLUNIFORM3FPROC glUniform3f;
            PFNGLUNIFORM4FPROC glUniform4f;
            PFNGLUNIFORM4FVPROC glUniform4fv;
            PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
            PFNGLUNMAPBUFFERPROC glUnmapBuffer;
            PFNGLUSEPROGRAMPROC glUseProgram;
            PFNGLVERTEXATTRIB4FPROC glVertexAttrib4f;
        }
        functions;

        void ( *pointers[sizeof( functions ) / sizeof( void ( * )() )] )();
        //void ( *pointers[] )();
    };

    struct ConstVertices
    {
        unsigned count;

        const float* coords;
        const float* normals;
        const float* uvs[TEXTURES_PER_VERTEX];
        const float* lightUvs;
    };

    struct FrameStats
    {
        unsigned numDirectionalLights, numPointLights, numPolys, numTextures, numRenderCalls;
    };

    struct GpuStats
    {
        size_t bytesInTextures, bytesInMeshes;
    };

    struct MatEntry
    {
        IMaterial* material;
        Mesh* first, * last;
    };

    class Queueable
    {
        public:
            virtual void render( const glm::mat4& transform ) = 0;
    };

    struct ShaderProgramSetProperties
    {
        unsigned numTextures;
        bool dynamicLighting, lightMapping, receivesShadows;
    };

    struct ShaderProgramProperties
    {
        ShaderProgramSetProperties common;

        unsigned numDirectionalLights, numPointLights;
    };

    struct Shared
    {
        bool haveAtiMeminfo, haveRenderBuffers, haveS3tc, isGlInit, requirePo2Textures, useShaders, useVaos, useVertexBuffers;
        int maxFixedLights;
        unsigned textureLod, maxPo2Upscale, maxTextureSize;
    };

    extern OpenGlApi glApi;
    extern Shared driverShared;
    extern FrameStats stats;

    SDL_RWops* getRwOps( SeekableInputStream* stream );

    class TexturePreload : public ITexturePreload
    {
        public:
            OpenGlDriver* driver;
            String name;

            Object<Image> image;

            Reference<Texture> finalized;
            Object<ILodFunction> lodFunction;

        public:
            TexturePreload( OpenGlDriver* driver, const char* name, Image* image, ILodFunction* lodFunction );
            virtual ~TexturePreload();

            //static TexturePreload* createFromStream( SeekableInputStream* input, const char* name, LodFunction* lodFunction );
            //static Image* load( SeekableInputStream* input );

            virtual const char* getClassName() const { return "OpenGlDriver.TexturePreload"; }
            virtual ITexture* getFinalized();
            virtual const char* getName() const { return name; }
    };

    class Shader : public ReferencedClass
    {
        protected:
            GLuint shader;

            void compile( const char* name, GLuint shaderType, const char* source );

        public:
            Shader();
            virtual ~Shader();

        friend class ShaderProgram;
    };

    class PixelShader : public Shader
    {
        public:
            li_ReferencedClass_override( PixelShader )

            PixelShader( const char* name, const char* source );
            virtual ~PixelShader();
    };

    class VertexShader : public Shader
    {
        public:
            li_ReferencedClass_override( VertexShader )

            VertexShader( const char* name, const char* source );
            virtual ~VertexShader();
    };

    class ShaderProgram //: public IShaderProgram
    {
        protected:
            OpenGlDriver* driver;
            unsigned numTextures;

            Reference<VertexShader> vertexShader;
            Reference<PixelShader> pixelShader;

            GLuint program;

            int textureIndices[TEXTURES_PER_VERTEX], lightMapIndex, pointShadowMapIndices[MAX_POINT_LIGHTS];

            GLint blendColour, lightMap, materialAmbient, materialDiffuse, materialEmissive, materialShininess, materialSpecular, sceneAmbient,
                    directionalAmbient[MAX_DIRECTIONAL_LIGHTS], directionalDiffuse[MAX_DIRECTIONAL_LIGHTS], directionalDir[MAX_DIRECTIONAL_LIGHTS], directionalSpecular[MAX_DIRECTIONAL_LIGHTS],
                    pointAmbient[MAX_POINT_LIGHTS], pointDiffuse[MAX_POINT_LIGHTS], pointPos[MAX_POINT_LIGHTS], pointRange[MAX_POINT_LIGHTS], pointSpecular[MAX_POINT_LIGHTS],
                    pointShadowMap[MAX_POINT_LIGHTS], pointShadowMatrix[MAX_POINT_LIGHTS], localToWorld,
                    textures[TEXTURES_PER_VERTEX];

        private:
            void init( PixelShader* pixel, VertexShader* vertex );

        public:
            ShaderProgram( OpenGlDriver* driver, ShaderProgramProperties* properties );
            virtual ~ShaderProgram();

            //virtual int getParamId( const char* name );
            virtual void select();
            /*virtual void setColourParam( int id, const Colour& colour );
            virtual void setTexture( ITexture* texture );
            virtual void setFloatParam( int id, float a );
            virtual void setVector2Param( int id, float x, float y );*/

            void setBlendColour( const Colour& colour );
            void setDirectionalLight( unsigned index, const DirectionalLightProperties& light );
            void setLightMap( GLuint texture );
            void setLocalToWorld( const glm::mat4& matrix );
            void setMaterialLighting( const MaterialLightingProperties& material );
            void setPointLight( unsigned index, const PointLightProperties& light );
            void setPointLightShadowMap( unsigned index, GLuint texture, const glm::mat4& matrix );
            void setSceneAmbient( const Colour& colour );
            void setTexture( unsigned index, GLuint texture );
    };

    class ShaderProgramSet : public ReferencedClass
    {
        OpenGlDriver* driver;
        ShaderProgramSetProperties properties;

        unsigned dirLightVars, pointLightVars;
        ShaderProgram** programs;

        public:
            li_ReferencedClass_override( ShaderProgramSet )

            ShaderProgramSet( OpenGlDriver* driver, ShaderProgramSetProperties* properties );
            virtual ~ShaderProgramSet();

            ShaderProgram* getShaderProgram( bool dynamicLighting );
            void init();
            bool matches( const ShaderProgramSetProperties* properties );
    };

    class Texture : public ITexture
    {
        public:
            OpenGlDriver* driver;
            String name;

            GLuint texture;
            size_t bytesAlloc;

            Vector2<unsigned> size;

            void init( Image* image, ILodFunction* lodFunction );
            void init( SDL_Surface* surface, ILodFunction* lodFunction );

        public:
            li_ReferencedClass_override( Texture )

            // From preload (already contains all the parameters we need)
            Texture( TexturePreload* preload );

            // From Image*
            Texture( OpenGlDriver* driver, const char* name, Image* image, ILodFunction* lodFunction );

            // From SDL_Surface (Font still does this)
            Texture( OpenGlDriver* driver, const char* name, SDL_Surface* surface, ILodFunction* lodFunction );

            // 2x2 solid colour
            Texture( OpenGlDriver* driver, const char* name, const Colour& colour );

            // Allocate only
            Texture( OpenGlDriver* driver, const char* name, unsigned width, unsigned height );

            // Depth Map
            Texture( OpenGlDriver* driver, const char* name, const Vector2<unsigned>& size );

            virtual ~Texture();

            virtual const char* getClassName() const { return "OpenGlDriver.Texture"; }
            virtual Vector<unsigned> getDimensions();
            virtual const char* getName() const { return name; }
    };

    class Material : public IMaterial
    {
        friend class RenderQueue;

        public:
            enum QueryFlags
            {
                getColour = 1,
                getNumTextures = 2
            };

        private:
            OpenGlDriver* driver;
            String name;

            Reference<ShaderProgramSet> shaderProgramSet;

            Colour colour;

            // Textures
            size_t numTextures;
            Reference<TexturePreload> texturePreloads[TEXTURES_PER_VERTEX];
            Reference<Texture> textures[TEXTURES_PER_VERTEX];

            // Dynamic Lighting Response
            bool dynamicLighting;
            MaterialLightingProperties dynamicLightingResponse;

            // Light Mapping
            bool lightMapping;
            Reference<TexturePreload> lightMapPreload;
            Reference<Texture> lightMap;

            // Shadow Mapping
            bool castsShadows, receivesShadows;

            MatEntry* queueEntry;

            void getShaderSet();

        public:
            li_ReferencedClass_override( Material )

            Material( OpenGlDriver* driver, const char* name, const MaterialProperties2* properties, bool finalized );
            virtual ~Material();

            void apply();
            void apply( const Colour& blend );
            void apply( const Colour& blend, Texture* texture0 );
            Material* finalize();
            virtual const char* getClassName() const { return "OpenGlDriver.Material"; }
            virtual const char* getName() const { return name; }
            virtual void query( unsigned flags, MaterialProperties2* properties );
            void setColour( const Colour& colour );
    };

    struct MeshPreload
    {
        // After finalizing, this structure is NOT DESTROYED
        // The ownership is transfered to the final Mesh object.
        // Saves us a lot of work :)

        String name;
        GLenum renderMode;
        MeshLayout layout;

        List<float> coords, normals, uvs[TEXTURES_PER_VERTEX], lightUvs;
        List<unsigned> indices;

        Material* material;
    };

    class Mesh/* : public Queueable*/
    {
        friend class RenderQueue;

        struct RemoteData
        {
            GLenum renderMode;
            MeshLayout layout;

            Material* material;

            GLuint vbo, ibo;
            GLenum indexFormat;
            uint8_t indexSize;

            size_t numVertices, numIndices, vboCapacity, iboCapacity;

            VertexProperties vertexProperties;
            unsigned vertexSize, normalOffset, uvOffset[TEXTURES_PER_VERTEX], lightUvOffset;
        };

        protected:
            OpenGlDriver* driver;

            Object<MeshPreload> localData;
            Object<RemoteData> remoteData;

            Queueable* nextQueued;
            unsigned queuedOffset, queuedCount;

            //void createVBO( unsigned flags, unsigned numVertices, const float* coords, const float* normals, const float* uvs );
            //void createIBO( unsigned flags, unsigned numIndices, const unsigned* indices );

            //void doRenderLegacy( bool flat );
            //void doRender( bool flat );

            void fixLocalUvs();

            void initStructs( GLenum renderMode, MeshLayout layout, Material* material );

            //void loadToBuffers( unsigned flags, unsigned numVertices, unsigned numIndices, const float* coords, const float* normals, const float* uvs, const unsigned* indices );

            void loadIndices( unsigned flags, size_t count, const unsigned* indices );
            void loadVertices( unsigned flags, const ConstVertices* vertices, bool fixUvs );

            // Assumes data != nullptr
            //void loadToLocal( unsigned numVertices, unsigned numIndices, const float* coords, const float* normals, const float* uvs, const unsigned* indices );

        public:
            Mesh( OpenGlDriver* driver, MeshPreload* preload, bool finalized );
            //Mesh( MeshCreationInfo* mesh, unsigned flags = IModel::fullStatic );
            Mesh( OpenGlDriver* driver, const char* name, GLenum renderMode, MeshLayout layout, bool isSimple, unsigned numVertices, unsigned numIndices, bool finalized );
            Mesh( OpenGlDriver* driver, MeshCreationInfo3* creationInfo, unsigned flags, bool finalized );
            ~Mesh();

            static Mesh* createCuboid( OpenGlDriver* driver, CuboidCreationInfo* info, unsigned flags );
            static Mesh* createFromHeightMap( OpenGlDriver* driver, TerrainCreationInfo* terrain, unsigned flags, bool finalized );
            static Mesh* createPlane( OpenGlDriver* driver, PlaneCreationInfo* plane, unsigned flags );

            // Rendering
            void beginRender( bool flat );
            void doRenderAll();
            void doRenderRange( size_t offset, size_t count );
            void endRender( bool flat );

            Mesh* finalize();

            size_t getNumVertices();
            static GLenum getRenderMode( MeshFormat format );

            void pick();
            void render( const glm::mat4& localToWorld, Material* material = nullptr );
            void render( Material* material, const Colour& blend );
            void render( Material* material, const Colour& blend, Texture* texture0 );
            void renderRange( size_t offset, size_t count, Material* material = nullptr );

            // TODO: wat
            //virtual void render( const glm::mat4& transform ) { render( transform, nullptr ); }

            //bool retrieveVertices( unsigned offset, Vertex* vertices, unsigned count );

            void setMaterial( Material* material );

            bool updateIndices( size_t offset, const unsigned* indices, size_t count );
            /*bool updateVertexCoords( unsigned offset, float* coords, unsigned count );
            bool updateVertices( unsigned offset, const Vertex* vertices, unsigned count );*/
    };

    class ModelPreload : public IModelPreload
    {
        public:
            OpenGlDriver* driver;
            String name;

            List<MeshPreload*> meshes;

            Reference<IModel> finalized;

        public:
            ModelPreload( OpenGlDriver* driver, const char* name );
            virtual ~ModelPreload();

            static ModelPreload* createFromMemory( OpenGlDriver* driver, const char* name, MeshCreationInfo2* meshes, size_t count, unsigned flags );
            static ModelPreload* createFromMemory( OpenGlDriver* driver, const char* name, MeshCreationInfo2** meshes, size_t count, unsigned flags );
            static ModelPreload* createFromMemory( OpenGlDriver* driver, const char* name, MeshCreationInfo3** meshes, size_t count, unsigned flags );
            virtual const char* getClassName() const { return "OpenGlDriver.ModelPreload"; }
            virtual const char* getName() const { return name; }
            virtual IModel* getFinalized();
    };

    class Model : public IModel
    {
        OpenGlDriver* driver;
        String name;

        List<Mesh*> meshes;

        public:
            Model( OpenGlDriver* driver, ModelPreload* preload, bool finalized );
            //Model( OpenGlDriver* driver, const char* name, const List<MeshCreationInfo*>& meshes, unsigned flags );
            Model( OpenGlDriver* driver, const char* name, MeshCreationInfo3* meshes, size_t count, unsigned flags, bool finalized );
            Model( OpenGlDriver* driver, const char* name, MeshCreationInfo3** meshes, size_t count, unsigned flags, bool finalized );
            //Model( OpenGlDriver* driver, const char* name, List<Mesh*>& meshes );
            Model( OpenGlDriver* driver, const char* name, Mesh** meshes, size_t count, bool finalized );
            virtual ~Model();

            virtual Model* finalize();

            virtual const char* getClassName() const override { return "OpenGlDriver.Model"; }
            virtual size_t getMeshCount();
            virtual size_t getMeshVertexCount( size_t mesh );
            virtual const char* getName() const override { return name; }

            virtual unsigned pick( const List<Transform>& transforms ) override;
            virtual unsigned pick( const Transform* transforms, size_t numTransforms ) override;

            virtual void render( const List<Transform>& transforms ) override;
            virtual void render( const List<Transform>** transforms, size_t count ) override;

            virtual void render( const Transform* transforms, size_t numTransforms, bool inWorldSpace ) override;
            virtual void render( const Transform* transforms, size_t numTransforms, const Colour& blend ) override;

            //virtual bool retrieveVertices( size_t mesh, size_t offset, Vertex* vertices, size_t count ) override;

            //virtual bool updateVertexCoords( unsigned mesh, unsigned offset, float* coords, unsigned count ) override;
            //virtual bool updateVertices( size_t mesh, size_t offset, const Vertex* vertices, size_t count ) override;
    };

    class BspModel : public IStaticModel
    {
        struct BspRenderMesh
        {
            unsigned material, indexOffset, indexCount;
        };

        class BspRenderNode
        {
            public:
                Vector<float> bounds[2];
                List<BspRenderMesh*> meshes;
                Object<BspRenderNode> children[2];

                ~BspRenderNode()
                {
                    iterate ( meshes )
                        delete meshes.current();
                }
        };

        OpenGlDriver* driver;
        String name;

        Object<BspRenderNode> root;
        List<Mesh*> materialGroups;

        BspRenderNode* create( BspNode* node );

        Array<unsigned> currentOffset;

        public:
            BspModel( OpenGlDriver* driver, const char* name, BspTree* bsp, IResourceManager* resMgr, bool finalized );
            ~BspModel();

            virtual BspModel* finalize() override;

            virtual const char* getClassName() const override { return "OpenGlDriver.BspModel"; }
            virtual size_t getMeshCount();
            virtual size_t getMeshVertexCount( size_t mesh );
            virtual const char* getName() const override { return name; }

            virtual void render() override;

            /*virtual unsigned pick( const List<Transform>& transforms ) override;
            virtual unsigned pick( const Transform* transforms, size_t numTransforms ) override;

            virtual void render() override { render( nullptr, 0 ); }

            virtual void render( const List<Transform>& transforms ) override;
            virtual void render( const List<Transform>** transforms, size_t count ) override;

            virtual void render( const Transform* transforms, size_t numTransforms, bool inWorldSpace ) override;
            virtual void render( const Transform* transforms, size_t numTransforms, const Colour& blend ) override;*/

            void renderNode( BspRenderNode* node );

            //virtual bool retrieveVertices( size_t mesh, size_t offset, Vertex* vertices, size_t count ) override;

            //virtual bool updateVertexCoords( unsigned mesh, unsigned offset, float* coords, unsigned count ) override;
            //virtual bool updateVertices( size_t mesh, size_t offset, const Vertex* vertices, size_t count ) override;
    };

    class Light : public ILight
    {
        protected:
            OpenGlDriver* driver;
            Type type;

            Object<DirectionalLightProperties> directional;
            Object<PointLightProperties> point;

        public:
            Light( OpenGlDriver* driver, const DirectionalLightProperties* properties );
            Light( OpenGlDriver* driver, const PointLightProperties* properties );
            virtual ~Light();

            //virtual void render( const List<Transform>& transforms ) override;
            virtual void render( const Transform* transforms, size_t count, bool inWorldSpace ) override;
            //Light* setCutoff( float angle );
    };

    class Font : public IFont
    {
        struct Batch
        {
            size_t size, used;
            GLuint vbo;

            float* vertices, * uvs[1];
        };

        struct Glyph
        {
            bool defined;
            float u[2], v[2], width, advance, offset;
        };

        struct Layout
        {
            Vector2<unsigned> dimensions;
            Colour colour;

            String string;
            float x, y;
        };

        OpenGlDriver* driver;
        String name;

#ifdef Use_Sdl_Ttf
        TTF_Font* font;
#else
        FtFont* font;
#endif
        unsigned lineSkip, size, style;

        float heightCorrection;
        List<Glyph> glyphs;

        Object<Mesh> plane;
        Reference<Material> material;

        Object<Batch> batch;

        /*float getCharWidth( Utf8Char c, float size );
        TextDim layoutString( float x0, float y0, const char* text, intptr_t numBytes, float size, Colour colour, bool render );
        TextDim wrapString( float x0, float y0, const char* text, intptr_t numBytes, float size, Colour colour, bool render, unsigned width );*/

        float getCharWidth( Unicode::Char c );
        Vector2<float> layoutString( float x0, float y0, const char* text, intptr_t numBytes, Colour colour, bool render );
        float renderChar( float x, float y, Unicode::Char c, const Colour& colour );

        public:
            Font( OpenGlDriver* driver, const char* name, SeekableInputStream* input, unsigned size, unsigned style );
            virtual ~Font();

            void batchFlush( const Colour& colour );
            float batchGlyph( float x, float y, Unicode::Char c, const Colour& colour );
            void batchString( float x0, float y0, const char* text, intptr_t numBytes, Colour colour, bool shadow );

            virtual void drawString( const Vector2<>& pos, const String& string, const Colour& colour, unsigned short align ) override;
            virtual void drawText( const Vector2<>& pos, const Text* text, float alpha = 1.0f ) override;

            static void exitFreeType();

            virtual float getLineSkip() override { return ( float ) lineSkip; }
            /*float getLineSkip( float size );*/

            virtual const char* getClassName() const { return "OpenGlDriver.Font"; }
            virtual const char* getName() const { return name; }
            virtual Vector2<float> getTextDimensions( Text* text );
            virtual unsigned getSize() override;
            virtual unsigned getStyle() override;

            //Texture* render( const char* text, Colour color );

            /*float renderChar( float x, float y, Utf8Char c, float size, const Colour& colour );
            TextDim render( float x, float y, const char* text, intptr_t numBytes, float size, Colour colour = Colour( 1.0f, 1.0f, 1.0f, 1.0f ), unsigned align = 0 );
            TextDim render( float x, float y, const String& text, float size, Colour colour = Colour( 1.0f, 1.0f, 1.0f, 1.0f ), unsigned align = 0 );
            TextDim render( float x, float y, const String& text, Colour colour = Colour( 1.0f, 1.0f, 1.0f, 1.0f ), unsigned align = 0 );*/

            virtual Text* layoutText( const String& text, const Colour& colour, unsigned short align ) override;
            virtual void releaseText( Text* text );
            virtual void renderString( float x, float y, const String& string, const Colour& colour, unsigned short align );
            virtual void renderText( float x, float y, const Text* text );
    };

    class ProjectionInfoBuffer : public IProjectionInfoBuffer
    {
        public:
            GLint viewport[4];
            glm::dmat4 modelView, projection;

        public:
            virtual ~ProjectionInfoBuffer()
            {
            }
    };

    class RenderBuffer : public IRenderBuffer
    {
        public:
            GLuint buffer, depth;
            Reference<Texture> texture;

        public:
            RenderBuffer( OpenGlDriver* driver, const Vector<unsigned>& size, bool withDepthBuffer );

            // Initialize with a depth texture
            RenderBuffer( OpenGlDriver* driver, Texture* depthTexture );

            virtual ~RenderBuffer();

            unsigned getHeight();
            virtual ITexture* getTexture();
            unsigned getWidth();
            bool isSetUp();
    };

    class RenderQueue : public IRenderQueue
    {
        public:
            OpenGlDriver* driver;

            List<MatEntry> materials;

        public:
            RenderQueue( OpenGlDriver* driver );
            virtual ~RenderQueue() {}

            void enqueue( Mesh* mesh, const glm::mat4& transform );

            virtual void add( IModel* model, const Transform* transforms, size_t numTransforms ) override;
            /*virtual void addLight( ILight* light, const Transform* transforms, size_t numTransforms ) override;*/

            virtual void render() override;
    };

    class ViewFrustum
    {
    	enum { top = 0, bottom, left, right, nearClip, farClip, numPlanes };

        public:
            enum TestResult { outside, intersect, inside };

        	Plane planes[numPlanes];

        	Vector<float> ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr;
        	float nearD, farD, ratio, angle,tang;
        	float nw, nh, fw, fh;

        	ViewFrustum();
        	~ViewFrustum();

            void setProjection( float angle, float ratio, float nearD, float farD );
            void setView( const Vector<float>& eye, const Vector<float>& center, const Vector<float>& up );
            TestResult pointInFrustum( const Vector<float>& point );
            TestResult sphereInFrustum( const Vector<float>& center, float radius );
            TestResult boxInFrustum( const Vector<float>& min, const Vector<float>& max );
    };

    int gluInvertMatrixd( const GLdouble m[16], GLdouble invOut[16] );
    void gluLookAt( GLdouble eyex, GLdouble eyey, GLdouble eyez, GLdouble centerx, GLdouble centery, GLdouble centerz, GLdouble upx, GLdouble upy, GLdouble upz );
    void gluMultMatrixVecd( const GLdouble matrix[16], const GLdouble in[4], GLdouble out[4] );
    void gluPerspective( float fov, float aspect, float zNear, float zFar );
    GLint gluUnProject( GLdouble winx, GLdouble winy, GLdouble winz, const GLdouble modelMatrix[16], const GLdouble projMatrix[16],
            const GLint viewport[4], GLdouble* objx, GLdouble* objy, GLdouble* objz );
    GLint gluUnProject2( GLdouble winx, GLdouble winy, GLdouble winz, const GLdouble modelMatrix[16], const GLdouble projMatrix[16],
            const GLint viewport[4], GLdouble* objx, GLdouble* objy, GLdouble* objz );

    class OpenGlDriver : public IEventListener, public IGraphicsDriver, public IR, public IEventReceiver
    {
        public:
            //enum ShaderType { lit, solid, picking, numShaders };

            GpuStats gpuStats;

        public:
            struct Features
            {
                bool depthTextures, gl3PlusOnly, shaders;
            };

            struct GlobalState
            {
                bool dynamicLightingEnabled, shadersEnabled, fontBatchingEnabled;
                size_t fontBatchSize;

                bool softShadows;
                bool shadowPcfEnabled;
                float shadowPcfDist;

                // State Caching
                const void* currentCoordSource, * currentUvSource[MAX_UVS_PER_VERTEX];

                unsigned numDirectionalLights, numPointLights;

                Colour sceneAmbient;

                DirectionalLightProperties directionalLights[MAX_DIRECTIONAL_LIGHTS];
                PointLightProperties pointLights[MAX_POINT_LIGHTS];

                Texture* pointShadowMap[MAX_POINT_LIGHTS];
                glm::mat4 pointShadowMatrix[MAX_POINT_LIGHTS];
            };

            struct Profiling
            {
                Object<IProfiler> profiler;
                int frame, event, render, swap;
                uint64_t frameTime, eventTime, renderTime, swapTime;
                unsigned interval;
            };

            struct RenderState
            {
                ShaderProgram* currentShaderProgram;
                Material *currentMaterialColour, *currentMaterialLighting, *currentMaterialTexture;
                Mesh* currentMesh;
            };

            struct Assets
            {
                Object<ShaderProgram> pickingShaderProgram;
            };

            IEngine* engine;

            // Configuration
            bool forceNoShaders, forceNoVbo, rendererOnly;

            // Window Management
            Object<IEventSource> eventSource;
            IEventListener* eventListener;

            Vector2<unsigned> viewport, windowSize;
            Vector2<int> viewportPos;

            // OpenGL
            Features features;
            RenderState renderState;
            GlobalState globalState;
            Profiling profiling;

            unsigned nextLightId, nextPickingId;

            // Camera
            Vector<float> currentCamera, currentUp;
            ViewFrustum frustum;

            glm::mat4 modelView;

            // Rendering
            ReferenceList<ShaderProgramSet> shaderProgramSets;
            Stack<ScreenRect> clippingRects;

            // RenderBuffers
            RenderBuffer* currentRenderBuffer;
            Stack<RenderBuffer*> renderBuffers;

            BlendMode currentBlendMode;
            Stack<BlendMode> blendModes;

            // Various stock objects
            Assets assets;
            Reference<Material> solidMaterial, texturedMaterial;
            Object<Mesh> plane1x1uv, line, rect1x1;
            Reference<IFont> statsFont;

            // 2012 new
            IEventInput* eventInput;
            Stack<Event_t> eventQueue;

        private:
            void initOpenGl();

            void setBlendMode( BlendMode blendMode );
            void setClippingRect( const ScreenRect& clippingRect );
            void setRenderBuffer( RenderBuffer* renderBuffer );

        public:
            OpenGlDriver( IEngine* engine );
            virtual ~OpenGlDriver();

            void render2d( Texture* texture, float x, float y, float w, float h, float u0, float v0, float u1, float v1, const Colour& blend );

            // OpenGlDriver.OpenGlDriver - Various
            void checkErrors( const char* caller );
            virtual Image* createImageFromStream( SeekableInputStream* input );
            ShaderProgramSet* getShaderProgramSet( ShaderProgramSetProperties* properties );

            // OpenGlDriver.OpenGlDriver - Picking
            Colour getPickingColour( unsigned id );
            unsigned getPickingId();
            ShaderProgram* getPickingShaderProgram();

            // OpenGlDriver.OpenGlDriver - Transformation
            void applyTransforms( const List<Transform>& transforms );
            static glm::mat4 applyTransforms( const Transform* transforms, unsigned numTransforms );

            // TEMPORARY
            virtual void setPointLightShadowMap( unsigned index, ITexture* depthMap, const glm::mat4& shadowMapMatrix ) override;

            // StormGraph.IEventListener
            virtual bool isRunning();
            virtual void onCloseButton();
            virtual void onFrameBegin();
            virtual void onFrameEnd();
            virtual void onKeyState( int16_t key, Key::State state, Unicode::Char character );
            virtual void onMouseMoveTo( const Vector2<int>& mouse );
            virtual void onRender();
            virtual void onViewportResize( const Vector2<unsigned>& dimensions );

            // StormGraph.IGraphicsDriver
            virtual int addDirectionalLight( const DirectionalLightProperties& properties, bool inWorldSpace ) override;
            virtual int addPointLight( const PointLightProperties& properties, bool inWorldSpace ) override;
            virtual void beginPicking() override;
            virtual void beginDepthRendering() override;
            virtual void beginShadowMapping( ITexture* shadowTexture, const glm::mat4& textureMatrix ) override;
            virtual void changeDisplayMode( DisplayMode* displayMode );
            virtual void clear();
            virtual void clearLights(); // TODO replace
            virtual IModel* createCuboid( const char* name, CuboidCreationInfo* cuboid ) override;
            virtual IModel* createCuboid( const char* name, const CuboidCreationInfo2& creationInfo, IMaterial* material, unsigned flags = IModel::fullStatic ) override;
            //virtual IMaterial* createCustomMaterial( const char* name, IShader* shader );
            //virtual IShader* createCustomShader( const char* base, const char* name );
            virtual ITexture* createDepthTexture( const char* name, const Vector2<unsigned>& resolution ) override;
            virtual ILight* createDirectionalLight( const Vector<float>& direction, const Colour& ambient, const Colour& diffuse, const Colour& specular ) override;
            virtual IFont* createFontFromStream( const char* name, SeekableInputStream* input, unsigned size, unsigned style ) override;
            //virtual ILight* createLight( ILight::Type type, const Vector<float>& direction, const Colour& ambient, const Colour& diffuse, float range );
            virtual IMaterial* createMaterial( const char* name, const MaterialProperties2* material, bool finalized ) override;
            //virtual IModel* createModelFromMemory( const char* name, List<MeshCreationInfo*>& meshes, unsigned flags ) override;
            virtual IModel* createModelFromMemory( const char* name, MeshCreationInfo2* meshes, size_t count, unsigned flags ) override;
            virtual IModel* createModelFromMemory( const char* name, MeshCreationInfo2** meshes, size_t count, unsigned flags ) override;
            virtual IModel* createModelFromMemory( const char* name, MeshCreationInfo3* meshes, size_t count, unsigned flags ) override;
            virtual IModel* createModelFromMemory( const char* name, MeshCreationInfo3** meshes, size_t count, unsigned flags ) override;
            virtual IModel* createPlane( const char* name, PlaneCreationInfo* plane );
            virtual ILight* createPointLight( float range, const Colour& ambient, const Colour& diffuse, const Colour& specular ) override;
            virtual IProjectionInfoBuffer* createProjectionInfoBuffer() override;
            virtual IRenderBuffer* createRenderBuffer( const Vector<unsigned>& dimensions, bool withDepthBuffer ) override;
            virtual IRenderBuffer* createRenderBuffer( ITexture* depthTexture ) override;
            virtual IRenderQueue* createRenderQueue() override;
            virtual IMaterial* createSolidMaterial( const char* name, const Colour& colour, ITexture* texture );
            virtual ITexture* createSolidTexture( const char* name, const Colour& colour );
            virtual IStaticModel* createStaticModelFromBsp( const char* name, BspTree* bsp, IResourceManager* resMgr, bool finalized ) override;
            virtual IModel* createTerrain( const char* name, TerrainCreationInfo* terrain, unsigned modelFlags );
            virtual ITexture* createTextureFromStream( SeekableInputStream* input, const char* name, ILodFunction* lodFunction );
            virtual void drawLine( const Vector<float>& a, const Vector<float>& b, const Colour& blend ) override;
            virtual void drawRectangle( const Vector<float>& pos, const Vector2<float>& size, const Colour& blend, ITexture* texture ) override;
            virtual void drawRectangleOutline( const Vector<float>& pos, const Vector2<float>& size, const Colour& blend, ITexture* texture ) override;
            virtual void drawStats() override;
            virtual void endDepthRendering() override;
            virtual unsigned endPicking( const Vector2<unsigned>& samplePos ) override;
            virtual void endShadowMapping() override;
            virtual void getDriverInfo( Info* info );
            virtual IEventListener* getEventListener();
            virtual int16_t getKey( const char* name );
            virtual String getKeyName( int16_t code );
            virtual const glm::mat4& getModelView() override { return modelView; }
            virtual void getProjectionInfo( IProjectionInfoBuffer* projectionInfoBuffer ) override;
            virtual IMaterial* getSolidMaterial();
            //virtual Texture* getSolidTexture();
            virtual Vector2<unsigned> getViewportSize() override;
            virtual Vector2<unsigned> getWindowSize() override { return windowSize; }
            virtual void popBlendMode();
            virtual void popClippingRect();
            virtual void popProjection();
            virtual void popRenderBuffer();
            virtual ITexturePreload* preloadTextureFromStream( SeekableInputStream* input, const char* name, ILodFunction* lodFunction );
            virtual IModelPreload* preloadModelFromMemory( const char* name, MeshCreationInfo2* meshes, size_t count, unsigned flags ) override;
            virtual IModelPreload* preloadModelFromMemory( const char* name, MeshCreationInfo2** meshes, size_t count, unsigned flags ) override;
            virtual void pushBlendMode( BlendMode blendMode );
            virtual void pushClippingRect( const ScreenRect& clippingRect );
            virtual void pushProjection();
            virtual void pushRenderBuffer( IRenderBuffer* renderBuffer );
            //virtual void renderPlane2d( const Vector<>& origin, const Vector2<>& dimensions, const Vector2<>& uv0, const Vector2<>& uv1, IMaterial* material );
            virtual void runMainLoop( IEventListener* eventListener );
            virtual void set2dMode( float nearZ, float farZ );
            virtual void set3dMode( float nearZ, float farZ );
            virtual void setCamera( const Camera* camera );
            virtual void setCamera( const Vector<float>& eye, const Vector<float>& center, const Vector<float>& up );
            virtual void setClearColour( const Colour& colour );
            virtual void setDisplayMode( DisplayMode* displayMode );
            virtual void setEventSource( IEventSource* eventSource );
            virtual void setLevelOfDetail( LevelOfDetail* lod );
            virtual void setOrthoProjection( const Vector2<float>& leftRight, const Vector2<float>& topBottom, const Vector2<float>& nearFar ) override;
            virtual void setPerspectiveProjection( float nearZ, float farZ, float fov ) override;
            //virtual void setParameter( const String& name, const String& value ) override;
            virtual void setProjection( const glm::mat4& projection ) override;
            virtual void setRenderFlag( RenderFlag flag, bool value ) override;
            virtual void setSceneAmbient( const Colour& colour ) override;
            virtual void setViewport( const Vector2<int>& pos, const Vector2<unsigned>& size, const Vector2<unsigned>& windowSize ) override;
            virtual void setViewTransform( const glm::mat4& viewTransform ) override;
            virtual void startup();
            virtual void unload();
            virtual bool unproject( const Vector2<float>& windowSpace, Vector<float>& worldSpace, IProjectionInfoBuffer* projectionInfoBuffer ) override;
            virtual Vector<float> unproject( const Vector<>& windowSpace, IProjectionInfoBuffer* projectionInfoBuffer ) override;

            // 2012 new stuff
            //virtual void draw2d( ITexture* texture, const Vector<float>& pos, const Vector2<float> size, const Colour& blend ) override;
            virtual void draw2dCenteredRotated( ITexture* texture, float scale, float alpha, const Colour& blend ) override;
            virtual StormRender::IR* getR() override { return this; }

            virtual void beginFrame() override;
            virtual void endFrame() override;
            virtual Event_t* getEvent() override;
            virtual void ReceiveEvent( const Event_t& event ) override;
    };
}
