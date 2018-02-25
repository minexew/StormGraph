
#include "GameScene.hpp"
#include "Messages.hpp"

namespace GameClient
{
    enum BindingIndices { moveUpKey, moveDownKey, moveLeftKey, moveRightKey, chatKey, hideUiKey,
            /****/ escKey, numEnterKey, toggleShadersKey, inventoryKey };

    static const char* bindingNames[] = { "moveUp", "moveDown", "moveLeft", "moveRight", "chat", "hideUi" };

// Test code
static Light* light;
static bool shaders = true;

static int mouseX, mouseY;

    GameScene::GameScene( TcpSocket* socket ) : ui( 0 ), overlay( 0 ), displayUi( false ), player( 0 ),
            hasMoved( false ), isTalking( false ), viewDrag( false ), map( 0 ), renderProgram( 0 ), renderProgram2D( 0 ), devMode( false )
    {
        sg->loadKeyBindings( keys, "profile/default.tolcl/controls.cfx2", bindingNames, sizeof( bindingNames ) / sizeof( *bindingNames ) );
        keys[escKey] = sg->getKey( "Escape" );
        keys[numEnterKey] = sg->getKey( "Num Enter" );
        keys[toggleShadersKey] = sg->getKey( "G" );
        keys[inventoryKey] = sg->getKey( "I" );

        Material* mat = Material::createSimple( "tolcl/tex/0_water.png" );
        mat->alpha = 0.6f;

        HeightMap* flat = new HeightMap( "tolcl/heightmap/flat.png", Vector<float>( 1000.0f, 1000.0f ), 0.0f );
        water = Model::createTerrain( flat, Vector<float>( 50.0f, 50.0f ), mat );
        flat->release();

        uiFont = globalResMgr->getNamedFont( "ui" );
        chatFont = globalResMgr->getNamedFont( "chat" );

        if ( shaders )
        {
            renderProgram = new Program( "sg_assets/shader/Default" );
            renderProgram2D = new Program( "sg_assets/shader/2D" );

            renderProgram->use();
        }

        sg->setSceneAmbient( Colour( 0.2f, 0.2f, 0.2f ) );

        displayMode = sg->getDisplayMode();
        maxChatLength = unsigned( ( displayMode.y - 100.0f ) / 25.0f );

        session = new WorldSession( socket, this );

        runSpeed = 5.0f;

        //light = new Light( Light_positional, Vector<float>( 300.0f, 300.f, 30.f ), Vector<float>(), Colour( 0.2f, 0.2f, 0.2f ), Colour( 1.0f, 1.0f, 1.0f ), 40.0f );
        light = new Light( Light_directional, Vector<float>(), Vector<float>( 0.0f, 0.0f, -1.0f ), Colour(), Colour( 0.5f, 0.5f, 0.5f ), 40.0f );

        inventory = new Inventory();

        ui = new GameUI::UI( displayMode.x, displayMode.y );

        GameUI::Panel* mainPanel = new GameUI::Panel( displayMode.x / 2 + 40, displayMode.y - 48, 400, 48, true );
            GameUI::GraphicalButton* invButton = new GameUI::GraphicalButton( 8, 8, 32, 32, globalResMgr->getTexture( "tolcl/gfx/invbutton.png" ) );
            invButton->setName( "inv_button" );
            invButton->setOnPush( this );
            mainPanel->add( invButton );
        ui->add( mainPanel );

        GameUI::ItemDrag* drag = new GameUI::ItemDrag( ui );
        drag->setName( "item_drag" );
        ui->add( drag );
        ui->overlay( drag );

        up = down = left = right = zin = zout = false;
        angle2 = M_PI / 5.0f;
        dist = 12.0f;

        // init scripting engine

        scripting = new Scripting( this );
        InputStream* autorun = sg->open( "autorun.misl" );

        if ( autorun )
            scripting->execute( autorun );

        graph = new SceneGraph();
    }

    GameScene::~GameScene()
    {
        delete renderProgram;
        delete renderProgram2D;

        delete map;

        water->release();

        if ( graph )
            delete graph;

        iterate ( players )
            delete players.current();
    }

    void GameScene::bind( const String& keyName, const String& event )
    {
        unsigned short key = sg->getKey( keyName );

        reverse_iterate ( bindings )
            if ( key == bindings.current().key )
            {
                bindings.remove( bindings.iter() );
                break;
            }

        Binding binding = { key, event };
        bindings.add( binding );
    }

    void GameScene::characterInfo( int pid, const String& name, const Vector<float>& loc, float angle )
    {
        map = new Map( graph, this, loc.x, loc.y );
        players.add( player = new Player( pid, name, loc, angle ) );

        displayUi = true;
    }

    bool GameScene::closeButtonAction()
    {
        if ( !isTalking )
            return true;
        else
            return false;
    }

    void GameScene::doMovement( float delta )
    {
        if ( up )
        {
            player->loc += Vector<float>( cos( player->angle ) * runSpeed * delta, -sin( player->angle ) * runSpeed * delta );
            hasMoved = true;
        }
        else if ( down )
        {
            player->loc -= Vector<float>( cos( player->angle ) * runSpeed * delta, -sin( player->angle ) * runSpeed * delta );
            hasMoved = true;
        }

        if ( left )
        {
            player->loc += Vector<float>( cos( player->angle + M_PI / 2.0f ) * runSpeed * delta, -sin( player->angle + M_PI / 2.0f ) * runSpeed * delta );
            hasMoved = true;
        }
        else if ( right )
        {
            player->loc += Vector<float>( cos( player->angle - M_PI / 2.0f ) * runSpeed * delta, -sin( player->angle - M_PI / 2.0f ) * runSpeed * delta );
            hasMoved = true;
        }
    }

    void GameScene::keyStateChange( unsigned short key, bool pressed, Utf8Char character )
    {
        if ( isTalking && pressed )
        {
            if ( key == 0x08 )
                text = text.dropRightPart( 1 );
            else if ( key == 0x0D || key == keys[escKey] || key == keys[numEnterKey] )
            {
                if ( key != keys[escKey] )
                    say( text );

                isTalking = false;

                text.clear();
            }
            else if ( character >= 0x20 )
                text = text + Utf8Character( character );

            return;
        }

        if ( pressed )
        {
            iterate ( bindings )
                if ( key == bindings.current().key )
                {
                    session->chat( bindings.current().chat );
                    return;
                }
        }

        if ( key == keys[moveUpKey] )
            up = pressed;
        else if ( key == keys[moveDownKey] )
            down = pressed;
        else if ( key == keys[moveLeftKey] )
            left = pressed;
        else if ( key == keys[moveRightKey] )
            right = pressed;
        else if ( key == keys[chatKey] && pressed )
            isTalking = true;
        else if ( key == keys[hideUiKey] && pressed )
        {
            displayUi = !displayUi;
            hasMoved = true;
        }
        else if ( key == keys[toggleShadersKey] && pressed )
        {
            // TODO
            shaders = !shaders;
            //sg->detachShader();
        }
        else if ( key == keys[inventoryKey] && pressed )
        {
        }
    }

    void GameScene::mouseButton( int x, int y, bool right, bool down )
    {
        if ( !right )
        {
            if ( down )
                ui->mouseDown( x, y );
            else
                ui->mouseUp( x, y );
        }
        else
        {
            viewDrag = down;
            viewDragOrigin = Vector<int>( x, y );
        }
    }

    void GameScene::mouseMove( int x, int y )
    {
        mouseX = x;
        mouseY = y;
        ui->mouseMove( x, y );

        if ( player && viewDrag )
        {
            if ( devMode )
                angle2 = angle2 + ( y - viewDragOrigin.y ) / 400.0f;

            player->angle -= ( x - viewDragOrigin.x ) / 400.0f;

            if ( x != viewDragOrigin.x )
                hasMoved = true;

            viewDragOrigin = Vector<int>( x, y );
        }
    }

    void GameScene::mouseWheel( bool down )
    {
        if ( devMode )
        {
            if ( down )
                dist += 1.0f;
            else
                dist -= 1.0f;
        }
    }

    void GameScene::newPlayer( unsigned pid, const String& name, const Vector<float>& loc, float angle )
    {
        players.add( new Player( pid, name, loc, angle ) );
    }

    void GameScene::onPickingMatch( ObjectNode* node )
    {
        pickingMatch = node->getName();
    }

    void GameScene::parseClientCommand( const String& text )
    {
        List<String> tokens;

        text.parse( tokens, ' ', '\\' );

        if ( tokens[0] == "exec" )
        {
            InputStream* input = File::open( tokens[1] );

            if ( !input || !scripting->execute( input ) )
                write( "\\r" "Script failed!" );
        }
        else if ( tokens[0] == "export" && player )
        {
            Sector* sect = map->getSectorAt( player->loc.x, player->loc.y );

            if ( sect )
                sect->saveAs( tokens[1] );
        }
        else if ( tokens[0] == "give" )
        {
            unsigned bag, slot;

            if ( !inventory->getEmptySlot( bag, slot ) )
                write( "\\r" "No empty slot found." );
            else
                inventory->set( bag, slot, Item( tokens[1] ) );
        }
        else if ( tokens[0] == "inv" )
        {
            unsigned numBags = inventory->getNumBags();

            for ( unsigned bag = 0; bag < numBags; bag++ )
            {
                unsigned numSlots = inventory->getNumSlots( bag );
                write( ( String )"\\S\\g" "Bag " + bag + ": " + numSlots + " slots" );

                for ( unsigned i = 0; i < numSlots; i++ )
                {
                    Slot* slot = inventory->get( bag, i );

                    if ( slot && !slot->isEmpty() )
                        write( ( String )"\\l" " -- " + bag + "." + i + ": " + slot->get().getName() );
                }
            }
        }
        else if ( tokens[0] == "savesect" && player )
        {
            Sector* sect = map->getSectorAt( player->loc.x, player->loc.y );

            if ( sect )
                sect->save();
        }
        else if ( tokens[0] == "setmodel" && player )
            player->changeModel( tokens[1] );
        else if ( tokens[0] == "tp" && player )
        {
            player->loc = Vector<float>( tokens[1], tokens[2] );
            hasMoved = true;
        }
        else if ( tokens[0] == "ubind" )
        {
            unsigned short key = sg->getKey( tokens[1] );

            iterate ( bindings )
                if ( key == bindings.current().key )
                {
                    bindings.remove( bindings.iter() );
                    break;
                }
        }
    }

    void GameScene::playerLocation( unsigned pid, const Vector<float>& loc, float angle )
    {
        iterate ( players )
            if ( players.current()->pid == pid )
            {
                players.current()->loc = loc;
                players.current()->angle = angle;
                return;
            }
    }

    void GameScene::playerStatus( unsigned pid, const String& name, unsigned status )
    {
        if ( status == status::offline )
        {
            write( name + "\\o" " left the game." );

            iterate ( players )
                if ( players.current()->pid == pid )
                {
                    Player* p = players.current();
                    players.remove( players.iter() );
                    delete p;
                    return;
                }
        }
        else if ( status == status::online )
            write( name + "\\g" " connected." );
    }

    void GameScene::removePlayer( unsigned pid )
    {
        iterate ( players )
            if ( players.current()->pid == pid )
            {
                delete players.current();
                players.remove( players.iter() );
                return;
            }
    }

    void GameScene::removeWorldObj( float x, float y )
    {
        Sector* sect = map->getSectorAt( x, y );

        if ( sect )
            sect->deleteWorldObj( x, y );
    }

    void GameScene::render()
    {
        // TODO: remove one time
        // Lock map object (mthread sort())
        // MUST BE DONE AND MUST BE DONE FIRST
        // otherwise deadlock

        // Handle network I/O

        session->process();

        // No hello received? Nothing to do!

        if ( !player )
        {
            if ( shaders )
                renderProgram2D->use();

            uiFont->render( 10.0f, 10.0f, "Entering world...", Colour( 0.8f, 0.9f, 1.0f ) );
            return;
        }

        // Lock the self-sorting world map

        if ( map )
            map->lock();

        // Movement

        float delta = sg->getTimeDelta();

        doMovement( delta );

        // Movement sync

        if ( hasMoved )
        {
            player->loc.z = map->getHeightAt( player->loc.x, player->loc.y );
            session->movement( player->loc, player->angle );
            map->moveCenter( player->loc.x, player->loc.y );
            hasMoved = false;
        }

        // Projection + camera

        sg->setPerspectiveProjection();
        Camera::look( player->loc + Vector<float>( 0.0f, 0.0f, 1.6f ), dist, player->angle - M_PI / 2.0f, angle2 );

        // Picking

        pickingMatch.clear();

        picking->begin();
        graph->pick( picking );
        picking->end( mouseX, mouseY );

        String table = ( ( OrderedListNode* ) graph->getNode( "world_objs" ) )->getTable();

        // Render

        if ( shaders )
            renderProgram->use();

        light->render();

        iterate ( players )
            players.current()->render();

        map->render();
        graph->render();

        map->unlock( player->loc );

        water->renderBegin();
        water->translate( Vector<float>( 0.0f, 0.0f, 0.0f ) );
        water->render();
        water->renderEnd();

        // Overlay

        if ( shaders )
            renderProgram2D->use();

        sg->disableDepthTesting();
        iterate ( players )
            players.current()->renderName();
        sg->enableDepthTesting();

        sg->setOrthoProjection();
        sg->pushBlendMode( Blend_add );

        if ( displayUi )
            uiFont->render( displayMode.x - 10.0f, 10.0f, ( String )"tolcl pre-Alpha -- " + player->loc.x + ", " + player->loc.y, Colour( 0.5f, 0.8f, 1.0f ), Align::right );
        else
            uiFont->render( displayMode.x - 10.0f, 10.0f, "Tales of Lanthaia pre-Alpha", Colour( 0.5f, 0.8f, 1.0f ), Align::right );

        chatFont->render( 8.0f, 8.0f, table + "\n\npicking: " + picking->getId() );

        sg->popBlendMode();

        while ( chat.getLength() > maxChatLength )
            chat.remove( 0 );

        if ( displayUi )
        {
            for ( unsigned i = 0; i < chat.getLength(); i++ )
                chatFont->render( 20.0f, round( displayMode.y - 60.0f - i * chatFont->getLineSkip() * 1.2f ), chat[chat.getLength() - i - 1] );

            if ( isTalking )
            {
                sg->drawRect( Vector<float>( 0.0f, displayMode.y - 30.0f ), Vector<float>( displayMode.x, displayMode.y ), Colour( 0.0f, 0.0f, 0.0f, 0.6f ) );
                chatFont->render( 10, displayMode.y - 15, text + "+", Colour( 1.0f, 1.0f, 1.0f ), Align::middle );
            }
        }

        if ( !pickingMatch.isEmpty() )
            ui->showTooltip( mouseX + 40, mouseY, "\\w\\ Picking: \\s" + pickingMatch );
        else
            ui->hideTooltip();

        ui->render();
        ui->update();
    }

    void GameScene::say( const String& text )
    {
        if ( !text.isEmpty() )
        {
            if ( text.beginsWith( "#" ) )
                parseClientCommand( text.dropLeftPart( 1 ) );
            else if ( text.beginsWith( "$" ) )
                scripting->execute( new ArrayIOStream( text.dropLeftPart( 1 ) ) );
            else
                session->chat( text );
        }
    }

    void GameScene::setDevMode( bool enabled )
    {
        devMode = enabled;
    }

    void GameScene::spawnWorldObj( const String& name, float x, float y, float orientation )
    {
        Sector* sect = map->getSectorAt( x, y );

        if ( sect )
            sect->addWorldObj( name, x, y, map->getHeightAt( x, y ), orientation );
    }

    void GameScene::uiEvent( GameUI::Widget* widget, const String& event )
    {
        if ( widget->getName() == "inv_button" && event == "push" )
        {
            GameUI::InventoryWindow* inv = new GameUI::InventoryWindow( ui, 50, 50, inventory->getBag( 0 ) );
            inv->setName( "inventory_window" );
            inv->setOnClose( this );
            ui->add( inv );
        }
        else if ( widget->getName() == "inventory_window" && event == "close" )
            ui->destroyWidget( widget );
    }

    void GameScene::write( const String& text )
    {
        chat.add( text );
    }
}
