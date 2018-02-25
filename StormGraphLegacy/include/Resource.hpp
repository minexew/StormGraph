#pragma once

#include <StormGraph/StormGraph.hpp>

namespace StormGraph
{
    class ResourceManager;

    class Texture : public ReferencedClass
    {
        GLuint texture;

        unsigned width, height;
        Vector<float> origin;

        static GLenum getSurfaceFormat( const SDL_Surface* surface );
        void init( SDL_Surface* surface );

        public:
            li_ReferencedClass_override( Texture )

            Texture( const char* fileName );
            Texture( SDL_Surface* surface );
            Texture( unsigned width, unsigned height );
            virtual ~Texture();
            static Texture* tryLoad( const char* fileName );

#ifndef StormGraph_No_Helium
            Helium::Variable initMembers( Helium::Variable object );
#endif

            void blitSurface( SDL_Surface* surface, int x, int y );
            void centerOrigin();
            unsigned getHeight() const { return height; }
            unsigned getWidth() const { return width; }
            static SDL_Surface* load( const char* fileName );
            void render2D( float x, float y );
            void render2D( float x, float y, float w, float h, float u0 = 0.0f, float v0 = 0.0f, float u1 = 1.0f, float v1 = 1.0f, const Colour& blend = Colour( 1.0f, 1.0f, 1.0f, 1.0f ) );
            void renderBillboard( const Vector<float>& center, float width, float height, float angle = 0.0f, float alpha = 1.0f );
            void renderBillboard2( const Vector<float>& center, float width, float height, float angle = 0.0f, float alpha = 1.0f );
            void renderPlanar( const Vector<float>& center, float a, float angle );
            void renderQuad( const Vector<float>& a, const Vector<float>& b, const Vector<float>& c, const Vector<float>& d );
            void setOrigin( float x, float y );

            friend class Material;
    };

    class Material : public ReferencedClass
    {
        public:
            String name;
            Colour ambient, diffuse, specular, emissive;
            ITexture* texture;
            float alpha, shininess;

            li_ReferencedClass_override( Material )

            Material();
            virtual ~Material();

            void apply();
            static Material* createSimple( const char* texture );
    };

    class HeightMap : public ReferencedClass
    {
        unsigned width, height;
        Vector<float> dimensions;

        float* data, ** heights;

        public:
            HeightMap( const char* fileName, const Vector<float>& dimensions, float heightMin );
            ~HeightMap();

            Vector<float> getDimensions() const { return dimensions; }
            Vector<unsigned> getSize() const { return Vector<unsigned>( width, height ); }

            float get( unsigned x, unsigned y );
            float get( float x, float y );
    };

    struct Vertex;

    class Mesh : public ReferencedClass
    {
        MeshType type;
        String name;
        Material* material;

        GLuint vbo, ibo;
        unsigned numVertices, numIndices, indexLength, indexFormat;

        void buildVBO( const List<Vertex>& vertices );
        void buildVBO( const List<float>& coords, const List<float>& normals, const List<float>& uvs );
        void buildIBO( const List<unsigned short>& indices );
        void buildIBO( const List<unsigned>& indices );
        void createTerrain( HeightMap* heightMap, const Vector<float>& uvRepeat );
        void doRender( bool simple );

        public:
            li_ReferencedClass_override( Mesh )

            friend class Model;
            friend class ParserMs3d;

            Mesh( const String& name, Material* material );
            Mesh( const String& name, Material* material, const List<float>& coords, const List<float>& normals, const List<float>& uvs );
            Mesh( const String& name, Material* material, const List<float>& coords, const List<float>& normals, const List<float>& uvs, const List<unsigned short>& indices );
            virtual ~Mesh();

            void pickingRender( Picking* picking, unsigned id );
            void render();
            void renderTerrain( const Vector<float>& location, HeightMap* heightMap );
    };

    class Model : public ReferencedClass
    {
        List<Mesh*> meshes;

        Model();

        static InputStream* open( const char* uri );

        public:
            li_ReferencedClass_override( Model )

            Model( const char* fileName, ResourceManager* resourceMgr );
            Model( InputStream* input, ResourceManager* resourceMgr );
            virtual ~Model();
            static Model* tryLoad( const char* fileName, ResourceManager* resourceMgr );

            static Model* createTerrain( HeightMap* heightMap, const Vector<float>& uvRepeat, Material* material );

            void renderBegin();
            void renderEnd();
            unsigned pickingRender( Picking* picking );
            void render();
            void renderTerrain( const Vector<float>& location, HeightMap* heightMap );

            void rotate( float angle, const Vector<float>& vec );
            void scale( const Vector<float>& vec );
            void translate( const Vector<float>& vec );

#ifndef StormGraph_No_Helium
            void rotate( double angle, const Helium::VectorFixedObject* vec )
            {
                rotate( ( float )angle, Vector<double>( vec->x, vec->y, vec->z ) );
            }

            void scale( const Helium::VectorFixedObject* vec )
            {
                scale( Vector<double>( vec->x, vec->y, vec->z ) );
            }

            void translate( const Helium::VectorFixedObject* vec )
            {
                translate( Vector<double>( vec->x, vec->y, vec->z ) );
            }
#endif
    };

    class Shader : public ReferencedClass
    {
        protected:
            GLuint shader;

        public:
            Shader() : shader( 0 )
            {
            }

            virtual ~Shader();

        friend class Program;
    };

    class PixelShader : public Shader
    {
        public:
            li_ReferencedClass_override( PixelShader )

            PixelShader( const char* source );
            virtual ~PixelShader();
    };

    class VertexShader : public Shader
    {
        public:
            li_ReferencedClass_override( VertexShader )

            VertexShader( const char* source );
            virtual ~VertexShader();
    };

    class Program : public ReferencedClass
    {
        protected:
            PixelShader* pixel;
            VertexShader* vertex;

            GLuint program;
            GLint sceneAmbient, lightAmbient[maxLights], lightDiffuse[maxLights], lightPos[maxLights], lightRange[maxLights], lightSpecular[maxLights],
                    lightTransform[maxLights], materialAmbient, materialDiffuse, materialShininess, materialSpecular, vertexColour;

        private:
            void init( PixelShader* pixel, VertexShader* vertex );

        public:
            Program( const String& path );
            Program( PixelShader* pixel, VertexShader* vertex );
            virtual ~Program();

#ifndef StormGraph_No_Helium
            static Program* create( PixelShader* pixel, VertexShader* vertex )
            {
                return new Program( pixel->reference(), vertex->reference() );
            }
#endif

            void setSceneAmbient( const Colour& colour );
            void setLightAmbient( unsigned i, const Colour& colour );
            void setLightDiffuse( unsigned i, const Colour& colour );
            void setLightPos( unsigned i, const Vector<float>& pos, float* modelView );
            void setLightRange( unsigned i, float range );
            void setLightSpecular( unsigned i, const Colour& colour );
            void setMaterialAmbient( const Colour& colour );
            void setMaterialDiffuse( const Colour& colour );
            void setMaterialShininess( float shininess );
            void setMaterialSpecular( const Colour& colour );
            void setVertexColour( const Colour& colour );

            void use();
    };

    struct Align
    {
        enum
        {
            left = 0,
            top = 0,
            centered = 1,
            right = 2,
            middle = 4,
            bottom = 8
        };
    };

    struct Text
    {
        // public
        unsigned width, height;
        Colour colour;
        float size;

        // private
        String string;
        float x, y;
    };

    class Font : public ReferencedClass
    {
        struct Glyph
        {
            bool defined;
            float u[2], v[2], width;
        };

        struct TextDim
        {
            unsigned w, h;

            TextDim( unsigned w, unsigned h ) : w( w ), h( h )
            {
            }
        };

        TTF_Font* font;
        unsigned lineSkip, nominalSize;

        float heightCorrection;
        List<Glyph> glyphs;
        Texture* fontTexture;

        float getCharWidth( Utf8Char c, float size );
        TextDim layoutString( float x0, float y0, const char* text, intptr_t numBytes, float size, Colour colour, bool render );
        static TTF_Font* load( const char* fileName, int size );
        TextDim wrapString( float x0, float y0, const char* text, intptr_t numBytes, float size, Colour colour, bool render, unsigned width );

        public:
            li_ReferencedClass_override( Font )

            Font( const char* fileName, int size, unsigned cpMin = 0, unsigned cpMax = 256 );
            virtual ~Font();

            unsigned getLineSkip();
            float getLineSkip( float size );
            unsigned getNominalSize() const { return nominalSize; }

            Texture* render( const char* text, Colour color );

            float renderChar( float x, float y, Utf8Char c, float size, const Colour& colour );
            TextDim render( float x, float y, const char* text, intptr_t numBytes, float size, Colour colour = Colour( 1.0f, 1.0f, 1.0f, 1.0f ), unsigned align = 0 );
            TextDim render( float x, float y, const String& text, float size, Colour colour = Colour( 1.0f, 1.0f, 1.0f, 1.0f ), unsigned align = 0 );
            TextDim render( float x, float y, const String& text, Colour colour = Colour( 1.0f, 1.0f, 1.0f, 1.0f ), unsigned align = 0 );
            void render( float x, float y, const Text& text );

            Text layout( const String& text, float size, Colour colour = Colour( 1.0f, 1.0f, 1.0f, 1.0f ), unsigned align = 0 );
            Text layout( const String& text, Colour colour = Colour( 1.0f, 1.0f, 1.0f, 1.0f ), unsigned align = 0 );
    };

    class ResourceManager : public Resource
    {
        List<String> modelPaths, texturePaths;

        struct NamedFont
        {
            String name;
            Font* font;
        };

        struct LoadedModel
        {
            String name;
            Model* model;
        };

        struct LoadedTexture
        {
            String name;
            Texture* texture;
        };

        List<NamedFont> fonts;
        List<LoadedModel> models;
        List<LoadedTexture> textures;

        public:
            ResourceManager();
            virtual ~ResourceManager();

            void addModelPath( const char* path );
            void addNamedFont( const char* name, const char* path, int size, unsigned numGlyphs = 0 );
            void addTexturePath( const char* path );
            Model* getModel( const char* name, bool allowLoading = true, bool essential = true );
            Font* getNamedFont( const char* name, bool essential = true );
            Texture* getTexture( const char* name, bool allowLoading = true, bool essential = true );
    };
}
