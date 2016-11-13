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

#include <StormGraph/Engine.hpp>
#include <StormGraph/SceneGraph.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace StormGraph
{
    class SceneGraph;

    class DirectionalLightNode : public IDirectionalLightNode
    {
        public:
            IGraphicsDriver* graphicsDriver;

            DirectionalLightProperties properties;

        public:
            DirectionalLightNode( SceneGraph* sceneGraph, const DirectionalLightProperties& properties );
            virtual ~DirectionalLightNode();

            void render();
            //void render( IRenderQueue* renderQueue );
    };

    class PointLightNode : public IPointLightNode
    {
        public:
            SceneGraph* sceneGraph;
            IGraphicsDriver* graphicsDriver;

            PointLightProperties properties;

            Reference<ITexture> depthTexture;
            Reference<ICubeMap> depthCubeMap;

            Reference<IRenderBuffer> depthBuffer;
            glm::mat4 lightProjection, lightView;
            float shadowFov;

            //Transform transforms[1];

        public:
            PointLightNode( SceneGraph* sceneGraph, const PointLightProperties& properties, bool cubeShadowMapping, float shadowFov, unsigned shadowMapDetail );
            virtual ~PointLightNode();

            void render();
            //void render( IRenderQueue* renderQueue );

            void renderDepthBuffer();
    };

    class ModelNode : public IModelNode
    {
        Reference<IModel> model;
        //Vector<> pos, yawPitchRoll;

        Transform transforms[4];

        public:
            ModelNode( IModel* model, const Vector<>& pos, const Vector<>& yawPitchRoll );

            virtual Vector<> getPos() override { return transforms[3].vector; }
            virtual void move( const Vector<>& vec, bool absolute = false ) override;
            void render();
            //void render( IRenderQueue* renderQueue );
            virtual void setYaw( float yaw ) override { transforms[0].angle = yaw; }
    };

    class StaticModelNode : public IStaticModelNode
    {
        Reference<IStaticModel> model;

        public:
            StaticModelNode( IStaticModel* model ) : model( model ) {}

            void render() { model->render(); }
            //void render( IRenderQueue* renderQueue );
    };

    class SceneGraph : public ISceneGraph
    {
        public:
            IEngine* engine;
            IGraphicsDriver* graphicsDriver;
            String name;

            Colour sceneAmbient;

            List<DirectionalLightNode*> directionalLights;
            List<PointLightNode*> pointLights;
            
            List<StaticModelNode*> staticModels;
            List<ModelNode*> models;

            void renderScene();

        public:
            SceneGraph( IEngine* engine, const String& name );
            virtual ~SceneGraph();

            virtual IDirectionalLightNode* addDirectionalLight( const DirectionalLightProperties& properties ) override;
            virtual IPointLightNode* addPointLight( const PointLightProperties& properties, bool cubeShadowMapping, float fov, unsigned shadowMapDetail ) override;
            virtual IModelNode* addModel( IModel* model, const Vector<>& pos, const Vector<>& yawPitchRoll ) override;
            virtual IStaticModelNode* addStaticModel( IStaticModel* model ) override;
            virtual const char* getClassName() const { return "StormGraph.SceneGraph"; }
            virtual const char* getName() const { return name; }
            virtual void prerender() override;
            virtual void render() override;
            //virtual void render( IRenderQueue* renderQueue ) override;
            virtual void setSceneAmbient( const Colour& sceneAmbient ) override { this->sceneAmbient = sceneAmbient; };
    };

    DirectionalLightNode::DirectionalLightNode( SceneGraph* sceneGraph, const DirectionalLightProperties& properties )
            : properties( properties )
    {
        graphicsDriver = sceneGraph->graphicsDriver;
    }

    DirectionalLightNode::~DirectionalLightNode()
    {
    }

    void DirectionalLightNode::render()
    {
        graphicsDriver->addDirectionalLight( properties, true );
    }

    /*void DirectionalLightNode::render( IRenderQueue* renderQueue )
    {
        renderQueue->addLight( light, nullptr, 0 );
    }*/

    PointLightNode::PointLightNode( SceneGraph* sceneGraph, const PointLightProperties& properties, bool cubeShadowMapping, float shadowFov, unsigned shadowMapDetail )
            : sceneGraph( sceneGraph ), properties( properties ), shadowFov( shadowFov )
    {
        graphicsDriver = sceneGraph->graphicsDriver;

        //transforms[0] = Transform( Transform::translate, pos );

        lightProjection = glm::perspective( shadowFov, 1.0f, 2.0f, 10.0f );

        if ( !cubeShadowMapping )
        {
            depthTexture = sceneGraph->graphicsDriver->createDepthTexture( "PointLightNode.depthTexture", Vector2<unsigned>( shadowMapDetail, shadowMapDetail ) );
            depthBuffer = sceneGraph->graphicsDriver->createRenderBuffer( depthTexture->reference() );

            glm::vec3 up = properties.direction.x != 0 ? glm::vec3( -properties.direction.y, properties.direction.x, properties.direction.z )
                : glm::vec3( properties.direction.x, properties.direction.z, -properties.direction.y );
            lightView = glm::mat4() * glm::lookAt( properties.pos, properties.pos + properties.direction, up );
        }
        else
        {
            depthCubeMap = sceneGraph->graphicsDriver->createDepthCubeMap( "PointLightNode.depthCubeMap", Vector2<unsigned>( shadowMapDetail, shadowMapDetail ) );
            depthBuffer = sceneGraph->graphicsDriver->createRenderBuffer( depthCubeMap->reference() );
        }
    }

    PointLightNode::~PointLightNode()
    {
    }

    void PointLightNode::render()
    {
        //light->render( transforms, lengthof( transforms ) );

        int index = graphicsDriver->addPointLight( properties, true );

        if ( index >= 0 )
            graphicsDriver->setPointLightShadowMap( index, depthTexture, glm::mat4( 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f ) * lightProjection * lightView );
    }

    void PointLightNode::renderDepthBuffer()
    {
        if ( depthBuffer != nullptr )
        {
            graphicsDriver->pushRenderBuffer( depthBuffer );
            graphicsDriver->clear();
            graphicsDriver->setProjection( lightProjection );
            graphicsDriver->setViewTransform( lightView );
            //graphicsDriver->beginDepthRendering();
            sceneGraph->renderScene();
            //graphicsDriver->endDepthRendering();
            graphicsDriver->popRenderBuffer();
        }

        //First pass - from light's point of view
        /*glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(lightProjectionMatrix);

        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(lightViewMatrix);*/

        //Use viewport the same size as the shadow map
        //glViewport(0, 0, shadowMapSize, shadowMapSize);

        //Draw back faces into the shadow map
        /*glCullFace(GL_FRONT);

        //Disable color writes, and use flat shading for speed
        glShadeModel(GL_FLAT);
        glColorMask(0, 0, 0, 0);

        //Draw the scene
        DrawScene(angle);

        //restore states
        glCullFace(GL_BACK);
        glShadeModel(GL_SMOOTH);
        glColorMask(1, 1, 1, 1);*/
    }

    ModelNode::ModelNode( IModel* model, const Vector<>& pos, const Vector<>& yawPitchRoll )
            : model( model )
    {
        transforms[0] = Transform( Transform::rotate, Vector<>( 0.0f, 0.0f, 1.0f ), yawPitchRoll.z );
        transforms[1] = Transform( Transform::rotate, Vector<>( 1.0f, 0.0f, 0.0f ), yawPitchRoll.y );
        transforms[2] = Transform( Transform::rotate, Vector<>( 0.0f, 1.0f, 0.0f ), yawPitchRoll.x );
        transforms[3] = Transform( Transform::translate, pos );
    }

    void ModelNode::move( const Vector<>& vec, bool absolute )
    {
        if ( absolute )
            transforms[3].vector = vec;
        else
            transforms[3].vector += vec;
    }

    void ModelNode::render()
    {
        model->render( transforms, lengthof( transforms ) );
    }

    /*void ModelNode::render( IRenderQueue* renderQueue )
    {
        renderQueue->add( model, transforms, 4 );
    }*/

    SceneGraph::SceneGraph( IEngine* engine, const String& name )
            : engine( engine ), name( name )
    {
        graphicsDriver = engine->getGraphicsDriver();
    }

    SceneGraph::~SceneGraph()
    {
        iterate2 ( i, directionalLights )
            delete i;

        iterate2 ( i, pointLights )
            delete i;

        iterate2 ( i, models )
            delete i;

        iterate2 ( i, staticModels )
            delete i;
    }

    IDirectionalLightNode* SceneGraph::addDirectionalLight( const DirectionalLightProperties& properties )
    {
        DirectionalLightNode* node = new DirectionalLightNode( this, properties );
        directionalLights.add( node );
        return node;
    }

    IPointLightNode* SceneGraph::addPointLight( const PointLightProperties& properties, bool cubeShadowMapping, float fov, unsigned shadowMapDetail )
    {
        PointLightNode* node = new PointLightNode( this, properties, cubeShadowMapping, fov, shadowMapDetail );
        pointLights.add( node );
        return node;
    }

    IModelNode* SceneGraph::addModel( IModel* model, const Vector<>& pos, const Vector<>& yawPitchRoll )
    {
        ModelNode* node = new ModelNode( model, pos, yawPitchRoll );
        models.add( node );
        return node;
    }

    IStaticModelNode* SceneGraph::addStaticModel( IStaticModel* model )
    {
        StaticModelNode* node = new StaticModelNode( model );
        staticModels.add( node );
        return node;
    }

    void SceneGraph::prerender()
    {
        iterate2 ( i, pointLights )
            i->renderDepthBuffer();
    }

    void SceneGraph::render()
    {
        graphicsDriver->clearLights();

        graphicsDriver->setSceneAmbient( sceneAmbient );

        iterate2 ( i, directionalLights )
            i->render();

        iterate2 ( i, pointLights )
            i->render();

        //glm::mat4 biasMatrix = glm::translate( glm::scale( glm::mat4(), glm::vec3( 0.5f, 0.5f, 0.5f ) ), glm::vec3( 1.0f, 1.0f, 1.0f ) );
        const glm::mat4 biasMatrix( 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f );
        const glm::mat4 textureMatrix = biasMatrix * pointLights[0]->lightProjection * pointLights[0]->lightView;

        graphicsDriver->beginShadowMapping( pointLights[0]->depthTexture, textureMatrix );

        renderScene();

        graphicsDriver->endShadowMapping();
    }

    /*void SceneGraph::render( IRenderQueue* renderQueue )
    {
        iterate2 ( i, directionalLights )
            i->render( renderQueue );

        iterate2 ( i, models )
            i->render( renderQueue );
    }*/

    void SceneGraph::renderScene()
    {
        iterate2 ( i, staticModels )
            i->render();

        iterate2 ( i, models )
            i->render();
    }

    ISceneGraph* createSceneGraph( IEngine* engine, const char* name )
    {
        return new SceneGraph( engine, name );
    }
}
