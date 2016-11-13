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

#pragma once

#include <StormGraph/Core.hpp>
#include <StormGraph/GeometryFactory.hpp>

// TODO: remove IRenderable

namespace StormRender
{
    class IR;
}

namespace StormGraph
{
    class BspTree;
    class Camera;
    class IEngine;
    class IGraphicsDriver;
    class IHeightMap;
    class IMaterial;
    class IResourceManager;
    class ITexture;
    class ITexturePreload;

    typedef IGraphicsDriver* ( *GraphicsDriverProvider )( const char* driverName, IEngine* engine );

    li_enum_class( MeshFormat ) { lineList, lineStrip, pointList, triangleList };
    li_enum_class( MeshLayout ) { linear, indexed };
    li_enum_class( RenderFlag ) { culling, depthTest, wireframe };

    struct DirectionalLightProperties
    {
        Colour ambient;
        Colour diffuse;
        glm::vec3 direction;
        Colour specular;
    };

    struct DisplayMode
    {
        unsigned width, height;
        const char* windowTitle;
        bool fullscreen;

        bool vsync;
        unsigned multisamplingLevel;

        bool resizable;

        bool changeWindowTitle;
    };

    struct LevelOfDetail
    {
        unsigned textureLodLevel;
    };

    struct MaterialLightingProperties
    {
        Colour ambient, diffuse, specular, emissive;
        float shininess;
    };

    struct MaterialProperties2
    {
        Colour colour;

        // Textures
        size_t numTextures;
        ITexturePreload* texturePreloads[TEXTURES_PER_VERTEX];
        ITexture* textures[TEXTURES_PER_VERTEX];

        // Dynamic Lighting Response
        bool dynamicLighting;
        MaterialLightingProperties dynamicLightingResponse;

        // Light Mapping
        bool lightMapping;
        ITexturePreload* lightMapPreload;
        ITexture* lightMap;

        // Shadow Mapping
        bool castsShadows, receivesShadows;
    };

    struct MaterialStaticProperties
    {
        Colour colour;

        // Textures
        size_t numTextures;
        String textureNames[TEXTURES_PER_VERTEX];

        // Dynamic Lighting Response
        bool dynamicLighting;
        MaterialLightingProperties dynamicLightingResponse;

        // Light Mapping
        bool lightMapping;
        String lightMapName;

        // Shadow Mapping
        bool castsShadows, receivesShadows;
    };

    struct MeshCreationInfo2
    {
        String name;

        MeshFormat format;
        MeshLayout layout;
        IMaterial* material;

        size_t numVertices, numIndices;
        const Vertex* vertices;
        const unsigned* indices;
    };

    struct MeshCreationInfo3
    {
        MeshFormat format;
        MeshLayout layout;
        IMaterial* material;

        size_t numVertices, numIndices;
        const float* coords, * normals, * uvs[TEXTURES_PER_VERTEX], * lightUvs;
        const unsigned* indices;
    };

    struct PointLightProperties
    {
        Colour ambient;
        Colour diffuse;
        glm::vec3 direction, pos;
        float range;
        Colour specular;
    };

    struct ScreenRect
    {
        Vector2<uint16_t> pos, size;
    };

    class ICubeMap : public IResource
    {
        public:
            li_ReferencedClass_override( ICubeMap )

            virtual ~ICubeMap() {}
    };

    class ILodFunction
    {
        public:
            virtual ~ILodFunction() {}

            virtual unsigned getLod( unsigned globalLod ) = 0;
    };

    class IRenderable
    {
        public:
            virtual ~IRenderable() {}

            //virtual void render( const Transform* transforms = nullptr, size_t numTransforms = 0, bool inWorldSpace = true ) = 0;
    };

    class ITexture : public IResource
    {
        public:
            li_ReferencedClass_override( ITexture )

            virtual ~ITexture() {}

            virtual Vector<unsigned> getDimensions() = 0;
    };

    /*
     *  Abstract class representing a resource for creating Texture object.
     *  Resource preloads may be created in separate threads.
     */
    class ITexturePreload : public IResource
    {
        public:
            li_ReferencedClass_override( ITexturePreload )

            virtual ~ITexturePreload() {}

            virtual ITexture* getFinalized() = 0;
    };

    /*SgClass IHeightMap : public Resource
    {
        public:
            li_ReferencedClass_override( IHeightMap )

            IHeightMap( const char* name );
            virtual ~IHeightMap();
    };*/

    /*SgCoreClass IShaderProgram : public Resource
    {
        public:
            li_ReferencedClass_override( IShaderProgram )

            IShaderProgram( const char* name );
            virtual ~IShaderProgram();

            virtual int getParamId( const char* name ) = 0;
            virtual void select() = 0;
            virtual void setColourParam( int id, const Colour& colour ) = 0;
            virtual void setTexture( ITexture* texture ) = 0;
            virtual void setFloatParam( int id, float a ) = 0;
            virtual void setVector2Param( int id, float x, float y ) = 0;
    };*/

    /*struct MaterialProperties
    {
        enum { getAmbient = 1 << 0, getDiffuse = 1 << 1, getSpecular = 1 << 2, getEmissive = 1 << 3, getColour = 1 << 4,
                getShininess = 1 << 5, getTexture = 1 << 6, getShader = 1 << 7,
                setAmbient = 1 << 16, setDiffuse = 1 << 17, setSpecular = 1 << 18, setEmissive = 1 << 19, setColour = 1 << 20,
                setShininess = 1 << 21, setTexture = 1 << 22, setShader = 1 << 23 };

        unsigned query;

        Colour ambient, diffuse, specular, emissive, colour;
        float shininess;

        ITexturePreload* texturePreload;
        ITexture* texture;
        //IShaderProgram* shader;
    };*/

    class IMaterial : public IResource
    {
        public:
            li_ReferencedClass_override( IMaterial )

            virtual ~IMaterial() {}

            //virtual void query( MaterialProperties* properties ) = 0;
    };

    class IStaticModel : public IResource
    {
        public:
            li_ReferencedClass_override( IStaticModel )

            virtual ~IStaticModel() {}

            virtual IStaticModel* finalize() = 0;
            virtual void render() = 0;
    };

    class IModel : public IStaticModel//, public IRenderable
    {
        public:
            enum Flags { fullStatic = 0, dynamicVertices = 1, dynamicIndices = 2, streamedVertices = 4, streamedIndices = 8 };

        public:
            li_ReferencedClass_override( IModel )

            virtual ~IModel() {}

            //virtual size_t getMeshCount() = 0;
            //virtual size_t getMeshVertexCount( size_t mesh ) = 0;

            virtual unsigned pick( const List<Transform>& transforms ) = 0;

            virtual void render( const List<Transform>& transforms ) = 0;
            virtual void render( const List<Transform>** transforms, size_t count ) = 0;

            // New Rendering Functions
            virtual unsigned pick( const Transform* transforms = nullptr, size_t numTransforms = 0 ) = 0;

            virtual void render() { render( nullptr, 0, true ); }

            //virtual void render( const Transform* transforms = nullptr, size_t numTransforms = 0, bool inWorldSpace = true ) = 0;
            virtual void render( const Transform* transforms, size_t numTransforms, bool inWorldSpace = true ) = 0;
            virtual void render( const Transform* transforms, size_t numTransforms, const Colour& blend ) = 0;

            //virtual bool retrieveVertices( size_t mesh, size_t offset, Vertex* vertices, size_t count ) = 0;

            // `count` specifies the number of full 3-float vertices
            //virtual bool updateVertexCoords( unsigned mesh, unsigned offset, float* coords, unsigned count ) = 0;
            //virtual bool updateVertices( size_t mesh, size_t offset, const Vertex* vertices, size_t count ) = 0;
    };

    /*
     *  Abstract class representing a resource for creating Model object.
     *  Resource preloads may be created in separate threads and should avoid
     *  synchronizing with the main thread as much as possible.
     */
    class IModelPreload : public IResource
    {
        public:
            virtual ~IModelPreload() {}

            virtual IModel* getFinalized() = 0;
    };

    class ILight : public IRenderable, public ReferencedClass
    {
        public:
            enum Type { directional, point/*, line*/ };

        public:
            virtual ~ILight() {}

            virtual void render( const Transform* transforms = nullptr, size_t numTransforms = 0, bool inWorldSpace = true ) = 0;
    };

    struct Text;

    class IFont : public IResource
    {
        public:
            enum Align { left = 0, top = 0, centered = 1, right = 2, middle = 4, bottom = 8 };
            enum Style { normal = 0, bold = 1, italic = 2 };

        public:
            li_ReferencedClass_override( IFont )

            virtual ~IFont() {}

            /**
             *  @brief Draw a text string at the specified position.
             *
             *  If the text is to be rendered more than once, using pre-formatted text object is preferred.
             *
             *  @param pos the position to render the text at
             *  @param string the text to render
             *  @param colour initial colour
             *  @param align align flags
             *
             *  @see drawText
             *  @see layoutText
             */
            virtual void drawString( const Vector2<>& pos, const String& string, const Colour& colour, unsigned short align ) = 0;

            /**
             *  @brief Draw pre-formatted text at the specified position.
             *
             *  @param pos the position to render the text at
             *  @param text the pre-formatted text object
             *  @param alpha text opacity multiplier
             *
             *  @see drawString
             *  @see layoutText
             */
            virtual void drawText( const Vector2<>& pos, const Text* text, float alpha = 1.0f ) = 0;

            virtual float getLineSkip() = 0;
            virtual Vector2<float> getTextDimensions( Text* text ) = 0;
            virtual Vector2<float> getTextSize( Text* text ) { return getTextDimensions( text ); }
            virtual unsigned getSize() = 0;
            virtual unsigned getStyle() = 0;

            virtual Text* layoutText( const String& text, const Colour& colour, unsigned short align ) = 0;
            virtual void releaseText( Text* text ) = 0;
            virtual void renderString( float x, float y, const String& string, const Colour& colour, unsigned short align ) = 0;
            virtual void renderText( float x, float y, const Text* text ) = 0;
    };

    class IProjectionInfoBuffer
    {
        public:
            virtual ~IProjectionInfoBuffer() {}
    };

    class IRenderBuffer : public ReferencedClass
    {
        public:
            virtual ~IRenderBuffer() {}

            virtual ITexture* getTexture() = 0;
    };

    /**
     *  @brief Asynchronous rendering queue
     *
     *  All add* functions are thread-safe.
     */
    class IRenderQueue
    {
        public:
            virtual ~IRenderQueue() {}

            /**
             *  @brief Add model to the render queue.
             *
             *  @param model the model to render
             *  @param transforms array of transformations to perform on the model
             *  @param numTransforms number of items in transforms
             */
            virtual void add( IModel* model, const Transform* transforms, size_t numTransforms ) = 0;

            // TODO: virtual void add( IBspModel* model ) = 0;

            /**
             *  @brief Add a light to the scene.
             *
             *  @param light the light to add
             *  @param transforms array of transformations to perform on the model
             *  @param numTransforms number of items in transforms
             */
            //virtual void addLight( ILight* light, const Transform* transforms, size_t numTransforms ) = 0;

            /**
             *  @brief Start the rendering.
             */
            virtual void render() = 0;
    };

    SgStruct CuboidCreationInfo
    {
        const Vector<> dimensions, origin;
        bool withNormals, withUvs, wireframe;

        IMaterial* material;
        bool visibleFromInside;

        CuboidCreationInfo();
        CuboidCreationInfo( const Vector<>& dimensions, const Vector<>& origin, bool withNormals, bool withUvs, bool wireframe, IMaterial* material,
                bool visibleFromInside = false );
    };

    /*struct MeshCreationInfo
    {
        MeshFormat format;
        MeshLayout layout;
        IMaterial* material;

        unsigned numVertices, numIndices;
        const float* coords, * normals, * uvs;
        const unsigned* indices;
    };*/

    struct PlaneCreationInfo
    {
        Vector2<> dimensions;
        Vector<> origin;
        Vector2<> uv0, uv1;
        bool withNormals, withUvs;

        IMaterial* material;

        PlaneCreationInfo() : withNormals( false ), withUvs( false ), material( nullptr )
        {
        }

        PlaneCreationInfo( const Vector2<>& dimensions, const Vector<>& origin, const Vector2<>& uv0, const Vector2<>& uv1, bool withNormals, bool withUvs, IMaterial* material )
                : dimensions( dimensions ), origin( origin ), uv0( uv0 ), uv1( uv1 ), withNormals( withNormals ), withUvs( withUvs ), material( material )
        {
        }
    };

    SgStruct TerrainCreationInfo
    {
        IHeightMap* heightMap;

        Vector<> dimensions;
        Vector<> origin;
        Vector2<unsigned> resolution;
        Vector2<> uv0, uv1;
        bool withNormals, withUvs, wireframe;

        IMaterial* material;

        TerrainCreationInfo();
        TerrainCreationInfo( IHeightMap* heightMap, const Vector<>& dimensions, const Vector<>& origin, const Vector2<unsigned>& resolution,
                const Vector2<>& uv0, const Vector2<>& uv1, bool withNormals, bool withUvs, bool wireframe, IMaterial* material );
    };

    class IGraphicsDriver
    {
        public:
            enum BlendMode { normal, additive, subtractive, invert };

            struct Info
            {
                String release, copyright, renderer;
            };

        public:
            virtual ~IGraphicsDriver() {}

            // WIP features
            virtual void setPointLightShadowMap( unsigned index, ICubeMap* depthMap, const glm::mat4& shadowMapMatrix ) {}
            virtual void setPointLightShadowMap( unsigned index, ITexture* depthMap, const glm::mat4& shadowMapMatrix ) = 0;

            virtual int addDirectionalLight( const DirectionalLightProperties& properties, bool inWorldSpace ) = 0;
            virtual int addPointLight( const PointLightProperties& properties, bool inWorldSpace ) = 0;
            virtual void beginDepthRendering() = 0;
            virtual void beginPicking() = 0;
            virtual void beginShadowMapping( ITexture* shadowTexture, const glm::mat4& textureMatrix ) = 0;
            virtual void changeDisplayMode( DisplayMode* displayMode ) = 0;
            virtual void clear() = 0;
            virtual void clearLights() = 0;
            virtual IModel* createCuboid( const char* name, CuboidCreationInfo* hexahedron ) = 0;
            virtual IModel* createCuboid( const char* name, const CuboidCreationInfo2& creationInfo, IMaterial* material, unsigned flags = IModel::fullStatic ) = 0;
            virtual ICubeMap* createDepthCubeMap( const char* name, const Vector2<unsigned>& resolution ) { return nullptr; }
            virtual ITexture* createDepthTexture( const char* name, const Vector2<unsigned>& resolution ) = 0;
            virtual ILight* createDirectionalLight( const Vector<float>& direction, const Colour& ambient, const Colour& diffuse, const Colour& specular ) = 0;
            //virtual IMaterial* createCustomMaterial( const char* name, IShader* shader ) = 0;
            //virtual IShader* createCustomShader( const char* base, const char* name ) = 0;
            virtual IFont* createFontFromStream( const char* name, SeekableInputStream* input, unsigned size, unsigned style ) = 0;
            //virtual IHeightMap* createHeightMapFromStream( SeekableInputStream* input, const char* name ) = 0;
            virtual IMaterial* createMaterial( const char* name, const MaterialProperties2* material, bool finalized ) = 0;
            //virtual IModel* createModelFromBsp( const char* name, BspTree* bsp, IResourceManager* resMgr ) = 0;
            virtual IModel* createModelFromMemory( const char* name, MeshCreationInfo2* meshes, size_t count, unsigned flags = IModel::fullStatic ) = 0;
            virtual IModel* createModelFromMemory( const char* name, MeshCreationInfo2** meshes, size_t count, unsigned flags = IModel::fullStatic ) = 0;
            virtual IModel* createModelFromMemory( const char* name, MeshCreationInfo3* meshes, size_t count, unsigned flags = IModel::fullStatic ) = 0;
            virtual IModel* createModelFromMemory( const char* name, MeshCreationInfo3** meshes, size_t count, unsigned flags = IModel::fullStatic ) = 0;
            //virtual IModel* createModelFromStream( SeekableInputStream* input, const char* name, ResourceManager* resMgr ) = 0;
            virtual IModel* createPlane( const char* name, PlaneCreationInfo* plane ) = 0;
            virtual ILight* createPointLight( float range, const Colour& ambient, const Colour& diffuse, const Colour& specular ) = 0;
            virtual IProjectionInfoBuffer* createProjectionInfoBuffer() = 0;
            virtual IRenderBuffer* createRenderBuffer( const Vector<unsigned>& dimensions, bool withDepthBuffer ) = 0;
            virtual IRenderBuffer* createRenderBuffer( ICubeMap* depthCubeMap ) { return nullptr; }
            virtual IRenderBuffer* createRenderBuffer( ITexture* depthTexture ) = 0;
            virtual IRenderQueue* createRenderQueue() = 0;
            virtual IMaterial* createSolidMaterial( const char* name, const Colour& colour, ITexture* texture ) = 0;
            virtual ITexture* createSolidTexture( const char* name, const Colour& colour ) = 0;
            virtual IStaticModel* createStaticModelFromBsp( const char* name, BspTree* bsp, IResourceManager* resMgr, bool finalized ) = 0;
            virtual IModel* createTerrain( const char* name, TerrainCreationInfo* terrain, unsigned modelFlags ) = 0;
            //virtual IMaterial* createTexturedMaterial( const char* name, MaterialProperties* material ) = 0;
            virtual ITexture* createTextureFromStream( SeekableInputStream* input, const char* name, ILodFunction* lodFunction ) = 0;
            virtual void drawLine( const Vector<float>& a, const Vector<float>& b, const Colour& blend ) = 0;
            virtual void drawRectangle( const Vector<float>& pos, const Vector2<float>& size, const Colour& blend, ITexture* texture ) = 0;
            virtual void drawRectangleOutline( const Vector<float>& pos, const Vector2<float>& size, const Colour& blend, ITexture* texture ) = 0;
            virtual void drawStats() = 0;
            virtual void endDepthRendering() = 0;
            virtual unsigned endPicking( const Vector2<unsigned>& samplePos ) = 0;
            virtual void endShadowMapping() = 0;
            virtual void getDriverInfo( Info* info ) = 0;
            virtual IEventListener* getEventListener() = 0;
            virtual int16_t getKey( const char* name ) = 0;
            virtual String getKeyName( int16_t code ) = 0;
            virtual const glm::mat4& getModelView() = 0;
            virtual void getProjectionInfo( IProjectionInfoBuffer* projectionInfoBuffer ) = 0;
            virtual IMaterial* getSolidMaterial() = 0;
            virtual Vector2<unsigned> getViewportSize() = 0;
            virtual Vector2<unsigned> getWindowSize() = 0;
            virtual ITexturePreload* preloadTextureFromStream( SeekableInputStream* input, const char* name, ILodFunction* lodFunction ) = 0;
            virtual IModelPreload* preloadModelFromMemory( const char* name, MeshCreationInfo2* meshes, size_t count, unsigned flags = IModel::fullStatic ) = 0;
            virtual IModelPreload* preloadModelFromMemory( const char* name, MeshCreationInfo2** meshes, size_t count, unsigned flags = IModel::fullStatic ) = 0;
            //virtual void renderPlane2d( const Vector<>& origin, const Vector2<>& dimensions, const Vector2<>& uv0, const Vector2<>& uv1, IMaterial* material ) = 0;
            virtual void runMainLoop( IEventListener* eventListener ) = 0;
            virtual void popBlendMode() = 0;
            virtual void popClippingRect() = 0;
            virtual void popProjection() = 0;
            virtual void popRenderBuffer() = 0;
            virtual void pushBlendMode( BlendMode blendMode ) = 0;
            virtual void pushClippingRect( const ScreenRect& clippingRect ) = 0;
            virtual void pushProjection() = 0;
            virtual void pushRenderBuffer( IRenderBuffer* renderBuffer ) = 0;
            virtual void set2dMode( float nearZ, float farZ ) = 0;
            virtual void set3dMode( float nearZ, float farZ ) = 0;
            virtual void setCamera( const Camera* camera ) = 0;
            virtual void setCamera( const Vector<float>& eye, const Vector<float>& center, const Vector<float>& up ) = 0;
            virtual void setClearColour( const Colour& colour ) = 0;
            virtual void setDisplayMode( DisplayMode* displayMode ) = 0;
            virtual void setEventSource( IEventSource* eventSource ) = 0;
            virtual void setLevelOfDetail( LevelOfDetail* lod ) = 0;
            virtual void setOrthoProjection( const Vector2<float>& leftRight, const Vector2<float>& topBottom, const Vector2<float>& nearFar ) = 0;
            virtual void setPerspectiveProjection( float nearZ = 1.0f, float farZ = 1000.0f, float fov = 45.0f ) = 0;
            virtual void setProjection( const glm::mat4& projection ) = 0;
            virtual void setRenderFlag( RenderFlag flag, bool value ) = 0;
            virtual void setSceneAmbient( const Colour& colour ) = 0;
            virtual void setViewport( const Vector2<int>& pos, const Vector2<unsigned>& size, const Vector2<unsigned>& windowSize ) = 0;
            virtual void setViewTransform( const glm::mat4& viewTransform ) = 0;
            virtual void startup() = 0;
            virtual void unload() = 0;
            virtual bool unproject( const Vector2<float>& windowSpace, Vector<float>& worldSpace, IProjectionInfoBuffer* projectionInfoBuffer = nullptr ) = 0;
            virtual Vector<float> unproject( const Vector<>& windowSpace, IProjectionInfoBuffer* projectionInfoBuffer = nullptr ) = 0;

            // 2012 new stuff
            //virtual void draw2d( ITexture* texture, const Vector<float>& pos, const Vector2<float> size, const Colour& blend ) = 0;
            virtual void draw2dCenteredRotated( ITexture* texture, float scale, float angle, const Colour& blend ) = 0;
            virtual StormRender::IR* getR() = 0;
    };
}
