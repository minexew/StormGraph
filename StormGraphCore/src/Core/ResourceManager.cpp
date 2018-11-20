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

#include <StormGraph/Core.hpp>
#include <StormGraph/Engine.hpp>
#include <StormGraph/GraphicsDriver.hpp>
#include <StormGraph/ResourceManager.hpp>
#include <StormGraph/SceneGraph.hpp>
#include <StormGraph/SoundDriver.hpp>

#include <StormGraph/IO/Ctree2.hpp>
#include <StormGraph/IO/ModelLoader.hpp>

#include <littl/File.hpp>

namespace StormGraph
{
    class FixedLodFunction : public ILodFunction
    {
        enum { fixed } type;

        unsigned fixedValue;

        public:
            FixedLodFunction( unsigned fixedValue ) : fixedValue( fixedValue ) {}

            virtual unsigned getLod( unsigned globalLod ) { return fixedValue; }
    };

    class ResourceManager : public IResourceManager
    {
        protected:
            IEngine* engine;
            String name;

            List<String> resourcePaths;
            int loadFlags[( size_t ) LoadFlag::maxLoadFlag];

            Reference<IFileSystem> fileSystem;
            List<IResource*> resources;

            ILodFunction* getTextureLodFunction( const String& name );

            Ct2Node* loadCtree2Node( InputStream* input );
            IFont* loadFont( const char* name, unsigned size, unsigned style );
            //IModel* loadModel( IModelPreload* preload );
            void loadSceneGraphGroup( ISceneGraph* sceneGraph, InputStream* input );

            ISoundStream* openSoundStream( const String& name );

            IModelPreload* preloadModel( const char* name );
            ITexturePreload* preloadTexture( const char* name );

        public:
            li_ReferencedClass_override( ResourceManager )

            ResourceManager( IEngine* engine, const char* name, bool addDefaultPath, IFileSystem* fileSystem );
            virtual ~ResourceManager();

            virtual void addPath( const char* path ) override;

            virtual void finalizePreloads() override;

            virtual const char* getClassName() const { return "StormGraph.ResourceManager"; }
            virtual const char* getName() const { return name; }

            virtual Ct2Node* loadCtree2( const char* name, bool required ) override;
            virtual IModel* loadModel( const char* name ) override;
            virtual ISceneGraph* loadSceneGraph( const char* name, bool required ) override;
            virtual ITexture* loadTexture( const char* name ) override;

            // returns texture if available (return true), preload otherwise (return false)
            // should be only used by the graphics driver
            virtual bool getTexturePreload( const char* name, ITexture** texturePtr, ITexturePreload** texturePreloadPtr );

            virtual IFont* getFont( const char* name, unsigned size, unsigned style );
            virtual int getLoadFlag( LoadFlag flag ) override;
            virtual IMaterial* getMaterial( const char* name, bool finalized );
            virtual IModel* getModel( const char* name );
            virtual ISoundStream* getSoundStream( const char* name );
            virtual IStaticModel* getStaticModel( const char* name, bool finalized, bool required ) override;
            virtual ITexture* getTexture( const char* name );

            virtual void initializeMaterial( const MaterialStaticProperties* properties, MaterialProperties2* initialized, bool finalized );

            virtual void listResources() override;

            virtual void parseMaterial( const char* name, MaterialStaticProperties* properties ) override;

            virtual void releaseUnused() override;

            virtual void setLoadFlag( LoadFlag flag, int value ) override;
    };

    ResourceManager::ResourceManager( IEngine* engine, const char* name, bool addDefaultPath, IFileSystem* fileSystem )
            : engine( engine ), name( name ), fileSystem( fileSystem )
    {
        if ( addDefaultPath )
            resourcePaths.add( "" );

        for ( size_t i = 0; i < ( size_t ) LoadFlag::maxLoadFlag; i++ )
            loadFlags[i] = -1;

        Resource::add( this );
    }

    ResourceManager::~ResourceManager()
    {
        iterate ( resources )
            resources.current()->release();

        Resource::remove( this );
    }

    void ResourceManager::addPath( const char* path )
    {
        resourcePaths.add( path );
    }

    void ResourceManager::finalizePreloads()
    {
        reverse_iterate ( resources )
        {
            ITexturePreload* preload = dynamic_cast<ITexturePreload*>( resources.current() );

            if ( preload )
            {
                ITexture* texture = preload->getFinalized();
                preload->release();
                resources.remove( resources.iter() );

                printf( "ResourceManager: explicitly registering finalized texture `%s`\n", texture->getName() );
                resources.add( texture );
            }
        }
    }

    IFont* ResourceManager::getFont( const char* name, unsigned size, unsigned style )
    {
        iterate ( resources )
            if ( String::equals( resources.current()->getName(), name ) )
            {
                IFont* font = dynamic_cast<IFont*>( resources.current() );

                if ( font && font->getSize() == size && font->getStyle() == style )
                    return font->reference();
            }

        iterate ( resourcePaths )
        {
            IFont* font = loadFont( resourcePaths.current() + name, size, style );

            if ( font )
            {
                //printf( "ResourceManager: registering font `%s`\n", name );

                resources.add( font );
                return font->reference();
            }
        }

        throw Exception( "StormGraph.ResourceManager.getFont", "FontLoadError", ( String ) "Failed to load font `" + name + "`" );
    }

    int ResourceManager::getLoadFlag( LoadFlag flag )
    {
        SG_assert( ( size_t ) flag < ( size_t ) LoadFlag::maxLoadFlag )

        return loadFlags[( size_t ) flag];
    }

    IMaterial* ResourceManager::getMaterial( const char* name, bool finalized )
    {
        SG_assert ( finalized == true )

        iterate2 ( resource, resources )
            if ( String::equals( resource->getName(), name ) )
            {
                IMaterial* material = dynamic_cast<IMaterial*>( ( IResource* ) resource );

                if ( material != nullptr )
                    return material->reference();
            }

        /*iterate ( resourcePaths )
        {
            cfx2::Document materialDoc = Engine::getInstance()->loadCfx2Asset( resourcePaths.current() + name, false, fileSystem );

            if ( materialDoc.isOk() )
            {
                MaterialProperties2 properties;
                memset( &properties, 0, sizeof( properties ) );

                {
                    cfx2::Node material = materialDoc.findChild( "Material" );

                    if ( !material )
                        throw StormGraph::Exception( "StormGraph.ResourceManager.getMaterial", "MaterialDescriptionError", "Missing required node 'Material' in material description." );

                    properties.colour = ( String ) material.getAttrib( "colour" );

                    Object<cfx2::List> textures = material.getList( "select Texture" );

                    iterate ( *textures )
                    {
                        if ( properties.numTextures >= TEXTURES_PER_MATERIAL )
                            break;

                        properties.textures[properties.numTextures++] = getTexture( textures->current().getText() );
                    }

                    cfx2::Node dynamicLighting = materialDoc.findChild( "Lighting" );

                    if ( dynamicLighting )
                    {
                        properties.dynamicLighting = true;
                        properties.dynamicLightingResponse.ambient = ( String ) dynamicLighting.getAttrib( "ambient" );
                        properties.dynamicLightingResponse.diffuse = ( String ) dynamicLighting.getAttrib( "diffuse" );
                        properties.dynamicLightingResponse.specular = ( String ) dynamicLighting.getAttrib( "specular" );
                        properties.dynamicLightingResponse.emissive = ( String ) dynamicLighting.getAttrib( "emissive" );
                        properties.dynamicLightingResponse.shininess = String::toFloat( dynamicLighting.getAttrib( "shininess" ) );
                    }
                }

                IMaterial* material = Engine::getInstance()->getGraphicsDriver()->createMaterial( name, &properties );

                printf( "ResourceManager: registering material `%s`\n", name );

                resources.add( material );
                return material->reference();
            }
        }*/

        MaterialStaticProperties properties;
        MaterialProperties2 initialized;

        parseMaterial( name, &properties );
        initializeMaterial( &properties, &initialized, true );

        IMaterial* material = engine->getGraphicsDriver()->createMaterial( name, &initialized, true );

        printf( "ResourceManager: registering material `%s`\n", name );

        resources.add( material );
        return material->reference();
    }

    IModel* ResourceManager::getModel( const char* name )
    {
        iterate ( resources )
            if ( String::equals( resources.current()->getName(), name ) )
            {
                IModel* model = dynamic_cast<IModel*>( resources.current() );

                if ( model )
                    return model->reference();
            }

        iterate ( resources )
            if ( String::equals( resources.current()->getName(), name ) )
            {
                IModelPreload* preload = dynamic_cast<IModelPreload*>( resources.current() );

                if ( preload )
                {
                    IModel* model = preload->getFinalized();
                    preload->release();
                    resources.remove( resources.iter() );

                    printf( "ResourceManager: registering finalized model `%s`\n", name );
                    resources.add( model );
                    return model->reference();
                }
            }

        iterate ( resourcePaths )
        {
            Reference<IModel> model = loadModel( resourcePaths.current() + name );

            if ( model != nullptr )
            {
                printf( "ResourceManager: registering model `%s`\n", name );

                resources.add( model->reference() );
                return model.detach();
            }
        }

        throw Exception( "StormGraph.ResourceManager.getModel", "ModelLoadError", ( String ) "Failed to load model " + File::formatFileName( name ) + " (file not found)" );
    }

    ISoundStream* ResourceManager::getSoundStream( const char* name )
    {
        iterate ( resources )
            if ( String::equals( resources.current()->getName(), name ) )
            {
                ISoundStream* soundStream = dynamic_cast<ISoundStream*>( resources.current() );

                if ( soundStream != nullptr )
                    return soundStream->reference();
            }

        iterate ( resourcePaths )
        {
            ISoundStream* soundStream = openSoundStream( resourcePaths.current() + name );

            if ( soundStream != nullptr )
            {
                printf( "ResourceManager: registering soundStream `%s`\n", name );

                resources.add( soundStream );
                return soundStream->reference();
            }
        }

        throw Exception( "StormGraph.ResourceManager.getSoundStream", "SoundStreamOpenError",
                ( String )"Failed to open sound stream `" + name + "`." );
    }

    IStaticModel* ResourceManager::getStaticModel( const char* name, bool finalized, bool required )
    {
        iterate2 ( resource, resources )
            if ( String::equals( resource->getName(), name ) )
            {
                IStaticModel* model = dynamic_cast<IStaticModel*>( ( IResource* ) resource );

                if ( model != nullptr )
                {
                    resource = static_cast<IResource*>( model = model->finalize() );
                    return model->reference();
                }
            }

        iterate ( resourcePaths )
        {
            Reference<SeekableInputStream> input = fileSystem->openInput( resourcePaths.current() + name );

            if ( input != nullptr )
            {
                Reference<IStaticModel> model = ModelLoader::loadStaticModel( engine->getGraphicsDriver(), name, input.detach(), this, finalized );

                resources.add( model->reference() );
                return model.detach();
            }
        }

        throw Exception( "StormGraph.ResourceManager.getStaticModel", "ModelLoadError", ( String ) File::formatFileName( name ) + ": file not found" );
    }

    ITexture* ResourceManager::getTexture( const char* name )
    {
        iterate ( resources )
            if ( String::equals( resources.current()->getName(), name ) )
            {
                ITexture* texture = dynamic_cast<ITexture*>( resources.current() );

                if ( texture != nullptr )
                    return texture->reference();
            }

        iterate ( resources )
            if ( String::equals( resources.current()->getName(), name ) )
            {
                ITexturePreload* preload = dynamic_cast<ITexturePreload*>( resources.current() );

                if ( preload != nullptr )
                {
                    ITexture* texture = preload->getFinalized();
                    preload->release();
                    resources.remove( resources.iter() );

                    printf( "ResourceManager: registering finalized texture `%s`\n", name );
                    resources.add( texture );
                    return texture->reference();
                }
            }

        iterate ( resourcePaths )
        {
            ITexture* texture = loadTexture( resourcePaths.current() + name );

            if ( texture )
            {
                printf( "ResourceManager: registering texture `%s`\n", name );

                resources.add( texture );
                return texture->reference();
            }
        }

        throw Exception( "StormGraph.ResourceManager.getTexture", "TextureLoadError", ( String )"Failed to load texture `" + name + "`." );
    }

    ILodFunction* ResourceManager::getTextureLodFunction( const String& name )
    {
        cfx2_Node* textureInfo = engine->loadCfx2Asset( name + ".cfx2", false );

        if ( !textureInfo )
            return nullptr;

        cfx2_Node* fixedLod = cfx2_find_child( textureInfo, "FixedLod" );

        SG_assert3( fixedLod != nullptr && fixedLod->text != nullptr, "StormGraph.ResourceManager.getTextureLodFunction" )

        ILodFunction* func = new FixedLodFunction( strtoul( fixedLod->text, 0, 0 ) );
        cfx2_release_node( textureInfo );

        return func;
    }

    bool ResourceManager::getTexturePreload( const char* name, ITexture** texturePtr, ITexturePreload** texturePreloadPtr )
    {
        *texturePreloadPtr = 0;

        if ( texturePtr )
        {
            *texturePtr = 0;

            iterate ( resources )
                if ( String::equals( resources.current()->getName(), name ) )
                {
                    *texturePtr = dynamic_cast<ITexture*>( resources.current() );

                    if ( *texturePtr )
                        return true;
                }
        }

        iterate ( resources )
            if ( String::equals( resources.current()->getName(), name ) )
            {
                *texturePreloadPtr = dynamic_cast<ITexturePreload*>( resources.current() );

                if ( *texturePreloadPtr )
                    return false;
            }

        iterate ( resourcePaths )
        {
            ITexturePreload* preload = preloadTexture( resourcePaths.current() + name );

            if ( preload )
            {
                printf( "ResourceManager: registering texture preload `%s`\n", name );

                resources.add( preload );

                *texturePreloadPtr = preload->reference();
                return false;
            }
        }

        throw Exception( "StormGraph.ResourceManager.getTexturePreload", "TextureLoadError",
                ( String )"Failed to load texture " + File::formatFileName( name ) + " (file not found)" );
    }

    void ResourceManager::initializeMaterial( const MaterialStaticProperties* properties, MaterialProperties2* initialized, bool finalized )
    {
        memset( initialized, 0, sizeof( MaterialProperties2 ) );

        initialized->colour = properties->colour;

        initialized->numTextures = properties->numTextures;

        if ( finalized )
        {
            for ( unsigned i = 0; i < properties->numTextures; i++ )
                initialized->textures[i] = getTexture( properties->textureNames[i] );
        }
        else
        {
            for ( unsigned i = 0; i < properties->numTextures; i++ )
                getTexturePreload( properties->textureNames[i], &initialized->textures[i], &initialized->texturePreloads[i] );
        }

        initialized->dynamicLighting = properties->dynamicLighting;

        if ( properties->dynamicLighting )
            initialized->dynamicLightingResponse = properties->dynamicLightingResponse;

        initialized->lightMapping = properties->lightMapping;

        if ( properties->lightMapping )
        {
            if ( finalized )
                initialized->lightMap = getTexture( properties->lightMapName );
            else
                getTexturePreload( properties->lightMapName, &initialized->lightMap, &initialized->lightMapPreload );
        }

        initialized->castsShadows = properties->castsShadows;
        initialized->receivesShadows = properties->receivesShadows;
    }

    Ct2Node* ResourceManager::loadCtree2( const char* name, bool required )
    {
        Reference<SeekableInputStream> input = fileSystem->openInput( name );

        if ( input != nullptr )
            return loadCtree2Node( input );
        else if ( required )
            throw Exception( "StormGraph.ResourceManager.loadCtree2", "Ctree2LoadError", ( String ) "Failed to load collision tree `" + name + "`." );

        return nullptr;
    }

    Ct2Node* ResourceManager::loadCtree2Node( InputStream* input )
    {
        Object<Ct2Node> node = new Ct2Node;

        SG_assert( input->readItems<Vector2<float>>( node->bounds, 2 ) == 2 )

        uint32_t numLines;

        SG_assert( input->readItems<uint32_t>( &numLines, 1 ) == 1 )

        for ( size_t i = 0; i < numLines; i++ )
        {
            Ct2Line line;

            SG_assert( input->readItems<Vector2<>>( &line.a, 1 ) == 1 )
            SG_assert( input->readItems<Vector2<>>( &line.b, 1 ) == 1 )

            line.recalc();

            node->lines.add( line );
        }

        if ( input->read<uint8_t>() != 0 )
        {
            node->children[0] = loadCtree2Node( input );
            node->children[1] = loadCtree2Node( input );
        }

        return node.detach();
    }

    IFont* ResourceManager::loadFont( const char* name, unsigned size, unsigned style )
    {
        SeekableInputStream* input = fileSystem->openInput( name );

        if ( input )
            return engine->getGraphicsDriver()->createFontFromStream( name, input, size, style );

        return 0;
    }

    IModel* ResourceManager::loadModel( const char* name )
    {
        Reference<SeekableInputStream> input = fileSystem->openInput( name );

        if ( input != nullptr )
            return ModelLoader::load( engine->getGraphicsDriver(), name, input.detach(), this );

        return 0;
    }

    ISceneGraph* ResourceManager::loadSceneGraph( const char* name, bool required )
    {
        Reference<SeekableInputStream> input = fileSystem->openInput( name );

        if ( input != nullptr )
        {
            Object<ISceneGraph> sceneGraph = engine->createSceneGraph( name );

            Colour sceneAmbient = input->readUnsafe<Colour>();
            sceneGraph->setSceneAmbient( sceneAmbient );

            loadSceneGraphGroup( sceneGraph, input );

            return sceneGraph.detach();
        }
        else if ( required )
            throw Exception( "StormGraph.ResourceManager.loadSceneGraph", "SceneGraphLoadError", ( String )"Failed to load scene graph `" + name + "`." );

        return nullptr;
    }

    void ResourceManager::loadSceneGraphGroup( ISceneGraph* sceneGraph, InputStream* input )
    {
        while ( input->isReadable() )
        {
            uint8_t type;

            if ( !input->read( &type, 1 ) )
                throw Exception( "StormGraph.ResourceManager.loadSceneGraphGroup", "UnexpectedEof", "Unexpected end of input" );

            if ( type == 0 )
                break;

            if ( type == 1 )
            {
                //Vector<> direction = Vector<>::fromDirUint16( input->read<uint16_t>() );
                Vector<> direction = input->read<Vector<float>>();
                Colour ambient = Colour::fromRgbaUint32( input->read<uint32_t>() );
                Colour diffuse = Colour::fromRgbaUint32( input->read<uint32_t>() );
                Colour specular = Colour::fromRgbaUint32( input->read<uint32_t>() );

                DirectionalLightProperties properties = { ambient, diffuse, direction, specular };
                sceneGraph->addDirectionalLight( properties );
            }
            else if ( type == 2 )
            {
                //Vector<> direction = Vector<>::fromDirUint16( input->read<uint16_t>() );
                Vector<> pos = input->read<Vector<float>>();
                Vector<> direction = input->read<Vector<float>>();
                Colour ambient = Colour::fromRgbaUint32( input->read<uint32_t>() );
                Colour diffuse = Colour::fromRgbaUint32( input->read<uint32_t>() );
                Colour specular = Colour::fromRgbaUint32( input->read<uint32_t>() );
                float range = input->read<float>();

                float fov = input->read<float>();
                bool cubeShadowMapping = input->read<uint8_t>() != 0;
                unsigned shadowMapDetail = input->read<uint32_t>();

                PointLightProperties properties = { ambient, diffuse, direction, pos, range, specular };
                sceneGraph->addPointLight( properties, cubeShadowMapping, fov, shadowMapDetail );
            }
            else if ( type == 10 )
            {
                loadSceneGraphGroup( sceneGraph, input );
            }
            else
                throw Exception( "StormGraph.ResourceManager.loadSceneGraphGroup", "FormatError", "Invalid node id" );
        }
    }

    ITexture* ResourceManager::loadTexture( const char* name )
    {
        Reference<SeekableInputStream> input = fileSystem->openInput( name );

        if ( input != nullptr )
            return engine->getGraphicsDriver()->createTextureFromStream( input.detach(), name, getTextureLodFunction( name ) );

        return 0;
    }

    ISoundStream* SoundDriver_newVorbisSoundStream( InputStream* input, const char* name );

    ISoundStream* ResourceManager::openSoundStream( const String& name )
    {
        if ( name.endsWith( ".ogg" ) )
        {
            SeekableInputStream* input = fileSystem->openInput( name );

            if ( input )
                return SoundDriver_newVorbisSoundStream( input, name );
        }

        return nullptr;
    }

    IModelPreload* ResourceManager::preloadModel( const char* name )
    {
        SeekableInputStream* input = fileSystem->openInput( name );

        if ( input )
            return ModelLoader::preload( engine->getGraphicsDriver(), name, input, this );

        return 0;
    }

    void ResourceManager::parseMaterial( const char* name, MaterialStaticProperties* properties )
    {
        iterate ( resourcePaths )
        {
            cfx2::Document materialDoc = engine->loadCfx2Asset( resourcePaths.current() + name, false, fileSystem );

            if ( materialDoc.isOk() )
            {
                cfx2::Node material = materialDoc.findChild( "Material" );

                if ( !material )
                    throw StormGraph::Exception( "StormGraph.ResourceManager.getMaterial", "MaterialDescriptionError", "Missing required node 'Material' in material description." );

                // Clear the structure
                // Using memset() is not generally safe here as the struct contains littl Strings
                properties->numTextures = 0;
                properties->dynamicLighting = false;
                properties->lightMapping = false;
                properties->castsShadows = false;
                properties->receivesShadows = false;

                properties->colour = ( String ) material.getAttrib( "colour" );

                Object<cfx2::List> textures = material.getList( "select Texture" );

                iterate ( *textures )
                {
                    if ( properties->numTextures >= TEXTURES_PER_VERTEX )
                        break;

                    properties->textureNames[properties->numTextures++] = textures->current().getText();
                }

                cfx2::Node lighting = material.findChild( "Lighting" );

                if ( lighting )
                {
                    properties->dynamicLighting = true;
                    properties->dynamicLightingResponse.ambient = ( String ) lighting.getAttrib( "ambient" );
                    properties->dynamicLightingResponse.diffuse = ( String ) lighting.getAttrib( "diffuse" );
                    properties->dynamicLightingResponse.specular = ( String ) lighting.getAttrib( "specular" );
                    properties->dynamicLightingResponse.emissive = ( String ) lighting.getAttrib( "emissive" );
                    properties->dynamicLightingResponse.shininess = String::toFloat( lighting.getAttrib( "shininess" ) );
                }

                cfx2::Node shadows = material.findChild( "Shadows" );

                if ( shadows )
                {
                    properties->castsShadows = String::toBool( shadows.getAttrib( "casts" ) );
                    properties->receivesShadows = String::toBool( shadows.getAttrib( "receives" ) );
                }

                return;
            }
        }

        throw Exception( "StormGraph.ResourceManager.parseMaterial", "MaterialLoadError", ( String )"Failed to load material `" + name + "`." );
    }

    ITexturePreload* ResourceManager::preloadTexture( const char* name )
    {
        SeekableInputStream* input = fileSystem->openInput( name );

        if ( input )
            return engine->getGraphicsDriver()->preloadTextureFromStream( input, name, getTextureLodFunction( name ) );

        return 0;
    }

    void ResourceManager::listResources()
    {
        iterate ( resources )
            printf( " - %s `%s`\n", resources.current()->getClassName(), resources.current()->getName() );
    }

    void ResourceManager::releaseUnused()
    {
        reverse_iterate ( resources )
            if ( !resources.current()->hasOtherReferences() )
            {
                printf( "releaseUnused(): removing %s `%s`\n", resources.current()->getClassName(), resources.current()->getName() );
                resources.current()->release();
                resources.remove( resources.iter() );
            }
    }

    void ResourceManager::setLoadFlag( LoadFlag flag, int value )
    {
        SG_assert( ( size_t ) flag < ( size_t ) LoadFlag::maxLoadFlag )

        loadFlags[( size_t ) flag] = value;
    }

    IResourceManager* createResourceManager( IEngine* engine, const char* name, bool addDefaultPath, IFileSystem* fileSystem )
    {
        return new ResourceManager( engine, name, addDefaultPath, fileSystem );
    }
}
