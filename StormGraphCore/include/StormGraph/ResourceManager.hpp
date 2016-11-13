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

namespace StormGraph
{
    class Ct2Node;
    class IFont;
    class IMaterial;
    class IModel;
    class ISceneGraph;
    class ISoundStream;
    class IStaticModel;
    class ITexture;
    class ITexturePreload;

    struct MaterialProperties2;
    struct MaterialStaticProperties;

    li_enum_class( LoadFlag ) { useDynamicLighting, useLightMapping, useShadowMapping, maxLoadFlag };

    class IResourceManager : public IResource
    {
        public:
            li_ReferencedClass_override( IResourceManager )

            virtual ~IResourceManager() {}

            virtual void addPath( const char* path ) = 0;

            virtual void finalizePreloads() = 0;

            virtual Ct2Node* loadCtree2( const char* name, bool required ) = 0;
            virtual IModel* loadModel( const char* name ) = 0;
            virtual ISceneGraph* loadSceneGraph( const char* name, bool required ) = 0;
            virtual ITexture* loadTexture( const char* name ) = 0;

            // returns texture if available (return true), preload otherwise (return false)
            // should be only used by the graphics driver
            virtual bool getTexturePreload( const char* name, ITexture** texturePtr, ITexturePreload** texturePreloadPtr ) = 0;

            virtual IFont* getFont( const char* name, unsigned size, unsigned style ) = 0;
            virtual int getLoadFlag( LoadFlag flag ) = 0;
            virtual IMaterial* getMaterial( const char* name, bool finalized ) = 0;
            virtual IModel* getModel( const char* name ) = 0;
            virtual ISoundStream* getSoundStream( const char* name ) = 0;
            virtual IStaticModel* getStaticModel( const char* name, bool finalized, bool required = true ) = 0;
            virtual ITexture* getTexture( const char* name ) = 0;

            virtual void initializeMaterial( const MaterialStaticProperties* properties, MaterialProperties2* initialized, bool finalized ) = 0;

            virtual void listResources() = 0;

            virtual void parseMaterial( const char* name, MaterialStaticProperties* properties ) = 0;

            virtual void releaseUnused() = 0;

            /**
             *  Set a boolean or unsigned load flag.
             *
             *  @param flag the load flag to set
             *  @param value new value of the specified flag (-1 to use default where applicable)
             */
            virtual void setLoadFlag( LoadFlag flag, int value ) = 0;
    };
}
