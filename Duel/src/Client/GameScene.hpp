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

#include <StormGraph/CommandLine.hpp>
#include <StormGraph/GraphicsDriver.hpp>
#include <StormGraph/GuiDriver.hpp>
#include <StormGraph/KeyScanner.hpp>
#include <StormGraph/OnScreenLog.hpp>
#include <StormGraph/Scene.hpp>
#include <StormGraph/SceneGraph.hpp>

#include <StormGraph/IO/Ctree2.hpp>
#include <StormGraph/IO/FileSystem.hpp>

#include <littl/HashMap.hpp>
#include <littl/TcpSocket.hpp>

#include <Shared/Server.hpp>

namespace Duel
{
    using namespace StormGraph;

    struct Entity
    {
        //IEnt* ent;

        uint16_t entId;
        //String name;

        IModelNode* modelNode;
    };

    struct Map
    {
        String mapName;

        Reference<IUnionFileSystem> fileSystem;
        Reference<IResourceManager> resMgr;

        Reference<IStaticModel> bsp;
    };

    struct Movement
    {
        Vector2<> pos, vec;
        float r, speed;
        Vector2<> newPos;

        void recalc() { newPos = pos + vec * speed; }
    };

    class GameScene : public IScene
    {
        // Can't do this as a template, fails to link
        static uint16_t pass( const uint16_t& value ) { return value; }

        // Engine
        IEngine* sg;
        IGraphicsDriver* graphicsDriver;

        // Server
        Object<IGameServer> server;

        // Resources
        Reference<IResourceManager> resMgr;

        Object<IGui> gui;
        Object<ICommandLine> console;
        Object<IOnScreenLog> log;
        Object<IKeyScanner> keyScanner;

        // Scene
        Object<Map> map;

        Object<ISceneGraph> sceneGraph;
        Object<IRenderQueue> renderQueue;

        Camera camera;
        Vector<unsigned> viewport;

        // World
        Object<Ct2Node> ctree2;

        // MP
        bool playerSpawned;

        //List<Entity*> entities;
        HashMap<uint16_t, Entity*, uint16_t, pass> entities;
        ILocalPlayerEnt* playerEnt = nullptr;

        Reference<TcpSocket> socket;
        ArrayIOStream socketBuffer, receiveBuffer;

        // Tmp resources
        IStaticModelNode* world;
        IModelNode* player;

        // Keymapping
        size_t up, down, left, right;

        // Debugging/special
        Reference<IFont> osdFont;
        bool drawCollisions, noclip;
        Vector2<float> mouse;

        void collideNode( const Ct2Node* node, Movement& movement );
        void playerMove( Vector2<> vec, float delta );
        void renderCtree2Node( const Ct2Node* node, unsigned depth, unsigned& maxDepth, size_t& numLeaves );
        void spawnEntity( bool localplayer, uint16_t entId, String name, const Vector<>& pos );

        void processIncoming();
        void processLocalEvents();

        public:
            li_ReferencedClass_override( GameScene )

            GameScene( IEngine* sg, Map* map, IGameServer* server, TcpSocket* socket );
            virtual ~GameScene();

            virtual void init() override;
            virtual void onFrameBegin() override;
            virtual void onMouseMoveTo( const Vector2<int>& mouse ) override;
            virtual void onRender() override;
            virtual void onUpdate( double delta ) override;
            virtual void onViewportResize( const Vector2<unsigned>& dimensions ) override { viewport = dimensions; }
            virtual void uninit() override;
    };
}
