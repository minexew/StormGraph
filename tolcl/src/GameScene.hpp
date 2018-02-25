#pragma once

#include "GameClient.hpp"

#include "ClientUI.hpp"
#include "Inventory.hpp"
#include "Map.hpp"
#include "Player.hpp"
#include "Scripting.hpp"
#include "UI.hpp"
#include "WorldSession.hpp"

namespace GameClient
{
    struct Binding
    {
        //int type;
        unsigned short key;
        String chat;
    };

    class GameScene : public Scene, GameUI::EventListener, PickingListener
    {
        WorldSession* session;

        // Scene Objects
        Camera* cam;
        Model* water;

        // Camera
        float angle2, dist;

        // User Interface
        GameUI::UI* ui;
        Vector<unsigned short> displayMode;
        Texture* overlay;
        bool displayUi;
        Font* uiFont, * chatFont;

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
        Program* renderProgram, * renderProgram2D;

        // Beta/WIP features
        SceneGraph* graph;
        Inventory* inventory;
        Scripting* scripting;
        String emptyString;
        bool devMode;
        String pickingMatch;

        public:
            GameScene( TcpSocket* socket );
            virtual ~GameScene();

            void bind( const String& keyName, const String& event );
            void characterInfo( int pid, const String& name, const Vector<float>& loc, float playerAngle );
            virtual bool closeButtonAction();
            void doMovement( float delta );
            const String& getPlayerName() const { return player ? player->name : emptyString; }
            virtual void keyStateChange( unsigned short key, bool pressed, Utf8Char character );
            virtual void mouseButton( int x, int y, bool right, bool down );
            virtual void mouseMove( int x, int y );
            virtual void mouseWheel( bool down );
            void newPlayer( unsigned pid, const String& name, const Vector<float>& loc, float angle );
            virtual void onPickingMatch( ObjectNode* node );
            void parseClientCommand( const String& text );
            void playerLocation( unsigned pid, const Vector<float>& loc, float angle );
            void playerStatus( unsigned pid, const String& name, unsigned status );
            void removePlayer( unsigned pid );
            void removeWorldObj( float x, float y );
            virtual void render();
            void say( const String& what );
            void setDevMode( bool enabled );
            void spawnWorldObj( const String& name, float x, float y, float orientation );
            virtual void uiEvent( GameUI::Widget* widget, const String& event );
            void write( const String& text );
    };
}
