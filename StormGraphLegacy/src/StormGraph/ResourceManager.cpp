
#include "Internal.hpp"

namespace StormGraph
{
    ResourceManager::ResourceManager()
    {
        texturePaths.add( "" );
    }

    ResourceManager::~ResourceManager()
    {
        iterate ( fonts )
            fonts.current().font->release();

        iterate ( models )
            models.current().model->release();

        iterate ( textures )
            textures.current().texture->release();
    }

    void ResourceManager::addModelPath( const char* path )
    {
        modelPaths.add( path );
    }

    void ResourceManager::addNamedFont( const char* name, const char* path, int size, unsigned numGlyphs )
    {
        // TODO ensure it doesnt already exist?

        NamedFont fon = { name, new Font( path, size, 0, numGlyphs ) };
        fonts.add( fon );
    }

    void ResourceManager::addTexturePath( const char* path )
    {
        texturePaths.add( path );
    }

    Model* ResourceManager::getModel( const char* name, bool allowLoading, bool essential )
    {
        iterate ( models )
            if ( models.current().name == name )
                return models.current().model->reference();

        if ( allowLoading )
        {
            iterate ( modelPaths )
            {
                Model* model = Model::tryLoad( modelPaths.current() + name, this );

                if ( model )
                {
                    LoadedModel mod = { name, model };
                    models.add( mod );
                    return model->reference();
                }
            }
        }

        if ( essential )
            throw Exception( "StormGraph.ResourceManager", "getModel", "ModelLoadError",
                    ( String )"Failed to load model `" + name + "`. It probably either doesn't exist at all or is not in any of registered model directories." );
        else
            return 0;
    }

    Font* ResourceManager::getNamedFont( const char* name, bool essential )
    {
        iterate ( fonts )
            if ( fonts.current().name == name )
                return fonts.current().font->reference();

        if ( essential )
            throw Exception( "StormGraph.ResourceManager", "getNamedFont", "FontSearchError",
                    ( String )"Failed to find named font `" + name + "`. It was probably not correctly registered." );
        else
            return 0;
    }

    Texture* ResourceManager::getTexture( const char* name, bool allowLoading, bool essential )
    {
        iterate ( textures )
            if ( textures.current().name == name )
                return textures.current().texture->reference();

        if ( allowLoading )
        {
            iterate ( texturePaths )
            {
                Texture* texture = Texture::tryLoad( texturePaths.current() + name );

                if ( texture )
                {
                    LoadedTexture tex = { name, texture };
                    textures.add( tex );
                    return texture->reference();
                }
            }
        }

        if ( essential )
            throw Exception( "StormGraph.ResourceManager", "getTexture", "TextureLoadError",
                    ( String )"Failed to load texture `" + name + "`. It probably either doesn't exist at all or is not in any of registered texture directories." );
        else
            return 0;
    }
}
