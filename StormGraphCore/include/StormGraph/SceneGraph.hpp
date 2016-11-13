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
#include <StormGraph/GraphicsDriver.hpp>

namespace StormGraph
{
    class IDirectionalLightNode
    {
        public:
            virtual ~IDirectionalLightNode() {}
    };

    class IPointLightNode
    {
        public:
            virtual ~IPointLightNode() {}

            //virtual void move( const Vector<>& vec, bool absolute = false ) = 0;
    };

    class IModelNode
    {
        public:
            virtual ~IModelNode() {}

            virtual Vector<> getPos() = 0;
            virtual void move( const Vector<>& vec, bool absolute = false ) = 0;
            virtual void setYaw( float yaw ) = 0;
    };

    class IStaticModelNode
    {
        public:
            virtual ~IStaticModelNode() {}
    };

    class ISceneGraph : public IResource
    {
        public:
            virtual ~ISceneGraph() {}

            /**
             *  @brief Create and add a directional light node.
             *
             *  @param properties properties of the light
             */
            virtual IDirectionalLightNode* addDirectionalLight( const DirectionalLightProperties& properties ) = 0;

            /**
             *  @brief Create and add a point light node.
             *
             *  @param properties properties of the light
             *  @param cubeShadowMapping use a cube map for shadow mapping (not implemented)
             *  @param fov Field of View for shadow map generation
             *  @param shadowMapDetail relative shadow map edge resolution in pixels
             */
            virtual IPointLightNode* addPointLight( const PointLightProperties& properties, bool cubeShadowMapping, float fov, unsigned shadowMapDetail ) = 0;

            virtual IModelNode* addModel( IModel* model, const Vector<>& pos, const Vector<>& yawPitchRoll ) = 0;
            virtual IStaticModelNode* addStaticModel( IStaticModel* model ) = 0;

            virtual void prerender() = 0;
            virtual void render() = 0;
            //virtual void render( IRenderQueue* renderQueue ) = 0;

            virtual void setSceneAmbient( const Colour& sceneAmbient ) = 0;
    };
}
