/*
    Tales of Lanthaia Game Client
    Copyright (c) 2011 The Tales of Lanthaia Project

    All rights reserved.
    Created by: Xeatheran Minexew
*/

#pragma once

#include "Map.hpp"
#include "TolClient.hpp"

#include <StormGraph/GraphicsDriver.hpp>
#include <StormGraph/Scene.hpp>

#include <littl/TcpSocket.hpp>

namespace TolClient
{
//    class Inventory;
    class ObjectNode;
    class OrderedListNode;
    class PickingListener;
    class Scripting;
    class Sector;
    class WorldSession;

    struct Binding
    {
        //int type;
        unsigned short key;
        String chat;
    };

    class Player
    {
        unsigned pid;
        String name;
        Vector<float> loc;
        float angle;

        StormGraph::IModel* model, * sword;
//        StormGraph::ITexture* nameTexture;

        friend class WorldScene;

    public:
        Player( unsigned pid, const String& name, const Vector<float>& loc, float angle );
        ~Player();

        bool changeModel( const String& name );
        void render();
        void renderName();
    };

    class WorldScene: public IScene,
            public PickingListener
    {
        public:
            WorldScene( TcpSocket* socket );
            virtual ~WorldScene();

            void init() override {}
            void uninit() override {}

            void onKeyState( int16_t key, Key::State state, Unicode::Char character ) override;
            void onMouseMoveTo( const Vector2<int>& mouse ) override;
            void onRender() override { render(); }
            void onUpdate( double delta ) override;

            void characterInfo( int pid, const String& name, const Vector<float>& loc, float playerAngle );
            void newPlayer( unsigned pid, const String& name, const Vector<float>& loc, float angle );
            void playerLocation( unsigned pid, const Vector<float>& loc, float angle );
            void playerStatus( unsigned pid, const String& name, unsigned status );
            void removePlayer( unsigned pid );
            void removeWorldObj( float x, float y );
            void spawnWorldObj( const String& name, float x, float y, float orientation );
            void write( const String& text );

    private:
        void bind( const String& keyName, const String& event );
        virtual bool closeButtonAction();
        void doMovement( float delta );
        const String& getPlayerName() const { return player ? player->name : emptyString; }
        virtual void mouseButton( int x, int y, bool right, bool down ) ;//override;
        virtual void onPickingMatch( ObjectNode* node );
        void parseClientCommand( const String& text );
        virtual void render();
        void say( const String& what );
        void setDevMode( bool enabled );
//        virtual void uiEvent( GameUI::Widget* widget, const String& event );

        WorldSession* session;

        // Scene Objects
        StormGraph::Camera* cam;
        StormGraph::IModel* water;

        // Camera
        float angle2, dist;

        // User Interface
        StormGraph::IGui* ui;
        Vector<unsigned short> displayMode;
        StormGraph::ITexture* overlay;
        bool displayUi;
        StormGraph::IFont* uiFont, * chatFont;

        // Players
        List<Player*> players;
        Player* player;

        // Controls
        bool left, right, up, down, zin, zout;
        Array<unsigned short> keys;
        List<Binding> bindings;

        // Movement
        bool hasMoved;
        float runSpeed;

        // Chat
        unsigned maxChatLength;
        List<String> chat;

        bool isTalking;
        String text;

        // View Panning
        bool viewDrag;
        Vector<int> viewDragOrigin;

        // Game Map
        Map* map;

        // Render Programs
        IShaderProgram* renderProgram, * renderProgram2D;

        // Beta/WIP features
        ISceneGraph* graph;
//        Inventory* inventory;
//        Scripting* scripting;
        String emptyString;
        bool devMode = true;
//        String pickingMatch;
    };
}
