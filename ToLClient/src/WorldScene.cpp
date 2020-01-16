/*
    Tales of Lanthaia Game Client
    Copyright (c) 2011, 2020 The Tales of Lanthaia Project

    All rights reserved.
    Created by: Xeatheran Minexew
*/

#include "Messages.hpp"
#include "WorldScene.hpp"
#include "WorldSession.hpp"

#include <StormGraph/GraphicsDriver.hpp>
#include <StormGraph/GuiDriver.hpp>
#include <StormGraph/HeightMap.hpp>
#include <StormGraph/ResourceManager.hpp>

namespace TolClient
{
    enum BindingIndices { moveUpKey, moveDownKey, moveLeftKey, moveRightKey, chatKey, hideUiKey,
        /****/ escKey, numEnterKey, toggleShadersKey, inventoryKey };

    static const char* bindingNames[] = { "moveUp", "moveDown", "moveLeft", "moveRight", "chat", "hideUi" };

// Test code
    static ILight* light;
    static bool shaders = true;

    static int mouseX, mouseY;

    extern IResourceManager* globalResMgr;

    static void loadKeyBindings( Array<unsigned short>& values, const char* fileName, const char** bindingNames, unsigned count )
    {
        cfx2_Node* doc = sg->loadCfx2Asset( fileName );

        for ( unsigned i = 0; i < count; i++ )
            values[i] = sg->getGraphicsDriver()->getKey( cfx2_query_value( doc, ( String )"Controls/" + bindingNames[i] ) );

        cfx2_release_node( doc );
    }

    WorldScene::WorldScene( TcpSocket* socket ) :
        ui( 0 ), overlay( 0 ), displayUi( false ),
         player( 0 ), hasMoved( false ), isTalking( false ), viewDrag( false ), map( 0 ), renderProgram( 0 ), renderProgram2D( 0 )
    {
        loadKeyBindings( keys, "profile/default.tolcl/controls.cfx2", bindingNames, sizeof( bindingNames ) / sizeof( *bindingNames ) );
        auto gr = sg->getGraphicsDriver();
        keys[escKey] = gr->getKey( "Escape" );
        keys[numEnterKey] = gr->getKey( "Num Enter" );
        keys[toggleShadersKey] = gr->getKey( "G" );
        keys[inventoryKey] = gr->getKey( "I" );

        auto waterTex = globalResMgr->getTexture("tolcl/tex/0_water.png");
        IMaterial* mat = gr->createSolidMaterial( "water", Colour::white()/*, waterTex*/ ,nullptr );

        //IHeightMap* flat = sg->loa HeightMap( "tolcl/heightmap/flat.png", Vector<float>( 1000.0f, 1000.0f ), 0.0f );
        li::Object<IHeightMap> flat = sg->createHeightMap( {2, 2} );
        TerrainCreationInfo terrain {
            flat,
            Vector<float>( 50.0f, 50.0f, 0.0f ),
            Vector<>( 0.0f, 0.0f, 0.0f ),
            flat->getResolution(),
            Vector2<>(),
            Vector2<float>( 50.0f, 50.0f ),
            true,
            true,
            false,
            mat
        };
//        water = gr->createTerrain( "water", &terrain, 0 );
        PlaneCreationInfo pci {
                {300.0f, 300.0f},
                {0.0f, 0.0f},
                {0.0f, 0.0f},
                {50.0f, 50.0f},
                false, true, mat
        };
        water = gr->createPlane("water", &pci);
        flat.release();

        IResourceManager* uiResMgr = Resources::getUiResMgr();
        uiFont = uiResMgr->getFont( "Radiance.EpicStyler.Assets/DefaultFont.ttf", 20, IFont::normal );
        chatFont = uiFont;

//        if ( shaders )
//        {
//            renderProgram = new Program( "sg_assets/shader/Default" );
//            renderProgram2D = new Program( "sg_assets/shader/2D" );
//
//            renderProgram->use();
//        }

        gr->setSceneAmbient( Colour( 0.2f, 0.2f, 0.2f ) );

        displayMode = gr->getWindowSize();
        maxChatLength = unsigned( ( displayMode.y - 100.0f ) / 25.0f );

        session = new WorldSession( socket, this );

        runSpeed = 5.0f;

        //light = new Light( Light_positional, Vector<float>( 300.0f, 300.f, 30.f ), Vector<float>(), Colour( 0.2f, 0.2f, 0.2f ), Colour( 1.0f, 1.0f, 1.0f ), 40.0f );
        light = gr->createDirectionalLight( Vector<float>( 0.0f, 0.0f, -1.0f ), Colour(), Colour( 0.5f, 0.5f, 0.5f ), 40.0f );

//        inventory = new Inventory();

        ui = sg->getGuiDriver()->createGui({}, Vector<>(displayMode.x, displayMode.y));

//        IPanel* mainPanel = ui->createPanel( Vector<>(displayMode.x / 2 + 40, displayMode.y - 48), {400, 48} );
//        GameUI::GraphicalButton* invButton = new GameUI::GraphicalButton( 8, 8, 32, 32, globalResMgr->getTexture( "tolcl/gfx/invbutton.png" ) );
//        invButton->setName( "inv_button" );
//        invButton->setOnPush( this );
//        mainPanel->add( invButton );
//        ui->add( mainPanel );

//        GameUI::ItemDrag* drag = new GameUI::ItemDrag( ui );
//        drag->setName( "item_drag" );
//        ui->add( drag );
//        ui->overlay( drag );

        up = down = left = right = zin = zout = false;
        angle2 = M_PI / 5.0f;
        dist = 12.0f;

        // init scripting engine

//        scripting = new Scripting( this );
//        InputStream* autorun = sg->open( "autorun.misl" );
//
//        if ( autorun )
//            scripting->execute( autorun );
//
        graph = sg->createSceneGraph("world");
    }

    WorldScene::~WorldScene()
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

    void WorldScene::bind( const String& keyName, const String& event )
    {
        unsigned short key = sg->getGraphicsDriver()->getKey( keyName );

        reverse_iterate ( bindings )
            if ( key == bindings.current().key )
            {
                bindings.remove( bindings.iter() );
                break;
            }

        Binding binding = { key, event };
        bindings.add( binding );
    }

    void WorldScene::characterInfo( int pid, const String& name, const Vector<float>& loc, float angle )
    {
        map = new Map( graph, this, loc.x, loc.y );
        players.add( player = new Player( pid, name, loc, angle ) );

        displayUi = true;
    }

    bool WorldScene::closeButtonAction()
    {
        if ( !isTalking )
            return true;
        else
            return false;
    }

    void WorldScene::doMovement( float delta )
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

    void WorldScene::onKeyState( int16_t key, Key::State state, Unicode::Char character )
    {
        if ( isTalking && state == Key::pressed )
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

        if ( state == Key::pressed )
        {
            iterate ( bindings )
                if ( key == bindings.current().key )
                {
                    session->chat( bindings.current().chat );
                    return;
                }
        }

        if ( key == keys[moveUpKey] )
            up = (state == Key::pressed);
        else if ( key == keys[moveDownKey] )
            down = (state == Key::pressed);
        else if ( key == keys[moveLeftKey] )
            left = (state == Key::pressed);
        else if ( key == keys[moveRightKey] )
            right = (state == Key::pressed);
        else if ( key == keys[chatKey] && state == Key::pressed )
            isTalking = true;
        else if ( key == keys[hideUiKey] && state == Key::pressed )
        {
            displayUi = !displayUi;
            hasMoved = true;
        }
        else if ( key == keys[toggleShadersKey] && state == Key::pressed )
        {
            // TODO
            shaders = !shaders;
            //sg->detachShader();
        }
        else if ( key == keys[inventoryKey] && state == Key::pressed )
        {
        }

        if ( devMode && key == /*Key::mouseWheelUp*/ -4 && state == Key::pressed ) {
            dist -= 1.0f;
        }

        if ( devMode && key == /*Key::mouseWheelDown*/ -5 && state == Key::pressed ) {
            dist += 1.0f;
        }
    }

    void WorldScene::mouseButton( int x, int y, bool right, bool down )
    {
        if ( !right )
        {
//            if ( down )
//                ui->mouseDown( x, y );
//            else
//                ui->mouseUp( x, y );
        }
        else
        {
            viewDrag = down;
            viewDragOrigin = Vector<int>( x, y );
        }
    }

    void WorldScene::onMouseMoveTo( const Vector2<int>& mouse )
    {
        mouseX = mouse.x;
        mouseY = mouse.y;
//        ui->mouseMove( x, y );

        if ( player && viewDrag )
        {
            if ( devMode )
                angle2 = angle2 + ( mouse.y - viewDragOrigin.y ) / 400.0f;

            player->angle -= ( mouse.x - viewDragOrigin.x ) / 400.0f;

            if ( mouse.x != viewDragOrigin.x )
                hasMoved = true;

            viewDragOrigin = Vector<int>( mouse.x, mouse.y );
        }
    }

    void WorldScene::newPlayer( unsigned pid, const String& name, const Vector<float>& loc, float angle )
    {
        players.add( new Player( pid, name, loc, angle ) );
    }

    void WorldScene::onPickingMatch( ObjectNode* node )
    {
//        pickingMatch = node->getName();
    }

    void WorldScene::parseClientCommand( const String& text )
    {
        List<String> tokens;

        text.parse( tokens, ' ', '\\' );

        if ( false ) {}
//        if ( tokens[0] == "exec" )
//        {
//            InputStream* input = File::open( tokens[1] );
//
//            if ( !input || !scripting->execute( input ) )
//                write( "\\r" "Script failed!" );
//        }
//        else if ( tokens[0] == "export" && player )
//        {
//            Sector* sect = map->getSectorAt( player->loc.x, player->loc.y );
//
//            if ( sect )
//                sect->saveAs( tokens[1] );
//        }
//        else if ( tokens[0] == "give" )
//        {
//            unsigned bag, slot;
//
//            if ( !inventory->getEmptySlot( bag, slot ) )
//                write( "\\r" "No empty slot found." );
//            else
//                inventory->set( bag, slot, Item( tokens[1] ) );
//        }
//        else if ( tokens[0] == "inv" )
//        {
//            unsigned numBags = inventory->getNumBags();
//
//            for ( unsigned bag = 0; bag < numBags; bag++ )
//            {
//                unsigned numSlots = inventory->getNumSlots( bag );
//                write( ( String )"\\S\\g" "Bag " + bag + ": " + numSlots + " slots" );
//
//                for ( unsigned i = 0; i < numSlots; i++ )
//                {
//                    Slot* slot = inventory->get( bag, i );
//
//                    if ( slot && !slot->isEmpty() )
//                        write( ( String )"\\l" " -- " + bag + "." + i + ": " + slot->get().getName() );
//                }
//            }
//        }
//        else if ( tokens[0] == "savesect" && player )
//        {
//            Sector* sect = map->getSectorAt( player->loc.x, player->loc.y );
//
//            if ( sect )
//                sect->save();
//        }
        else if ( tokens[0] == "setmodel" && player )
            player->changeModel( tokens[1] );
        else if ( tokens[0] == "tp" && player )
        {
            player->loc = Vector<float>( tokens[1], tokens[2] );
            hasMoved = true;
        }
        else if ( tokens[0] == "ubind" )
        {
            unsigned short key = sg->getGraphicsDriver()->getKey( tokens[1] );

            iterate ( bindings )
                if ( key == bindings.current().key )
                {
                    bindings.remove( bindings.iter() );
                    break;
                }
        }
    }

    void WorldScene::playerLocation( unsigned pid, const Vector<float>& loc, float angle )
    {
        iterate ( players )
            if ( players.current()->pid == pid )
            {
                players.current()->loc = loc;
                players.current()->angle = angle;
                return;
            }
    }

    void WorldScene::playerStatus( unsigned pid, const String& name, unsigned status )
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

    void WorldScene::removePlayer( unsigned pid )
    {
        iterate ( players )
            if ( players.current()->pid == pid )
            {
                delete players.current();
                players.remove( players.iter() );
                return;
            }
    }

    void WorldScene::removeWorldObj( float x, float y )
    {
        Sector* sect = map->getSectorAt( x, y );

        if ( sect )
            sect->deleteWorldObj( x, y );
    }

    void WorldScene::onUpdate( double delta )
    {
        // TODO: remove one time
        // Lock map object (mthread sort())
        // MUST BE DONE AND MUST BE DONE FIRST
        // otherwise deadlock

        // Handle network I/O

        session->process();

        // Lock the self-sorting world map

        if ( map )
            map->lock();

        // Movement

        doMovement( delta );

        // Movement sync

        if ( hasMoved )
        {
            player->loc.z = map->getHeightAt( player->loc.x, player->loc.y );
            session->movement( player->loc, player->angle );
            map->moveCenter( player->loc.x, player->loc.y );
            hasMoved = false;
        }

        if (ui) {
            ui->onUpdate(delta);
        }
    }

    void WorldScene::render()
    {
        // No hello received? Nothing to do!

        if ( !player )
        {
//            if ( shaders )
//                renderProgram2D->use();
//
            uiFont->drawString( {10.0f, 10.0f}, "Entering world...", Colour( 0.8f, 0.9f, 1.0f ), 0 );
            return;
        }

        // Projection + camera

        auto gr = sg->getGraphicsDriver();
        gr->setPerspectiveProjection();
        Vector<> eye, center, up;
        Camera::convert(dist, player->angle + M_PI, angle2, eye, center, up);
        auto pos = player->loc + Vector<float>( 0.0f, 0.0f, 1.6f );
        gr->setCamera( eye + pos, center + pos, up );

        // Picking

//        pickingMatch.clear();
//
//        picking->begin();
//        graph->pick( picking );
//        picking->end( mouseX, mouseY );

//        String table = ( ( OrderedListNode* ) graph->getNode( "world_objs" ) )->getTable();

        // Render

//        if ( shaders )
//            renderProgram->use();

        light->render();

        iterate ( players )
            players.current()->render();

        map->render();
        graph->render();

        map->unlock( player->loc );

//        water->renderBegin();
//        water->translate( Vector<float>( 0.0f, 0.0f, 0.0f ) );
        water->render();
//        water->renderEnd();

        // Overlay

//        if ( shaders )
//            renderProgram2D->use();

//        gr->disableDepthTesting();
        iterate ( players )
            players.current()->renderName();
//        gr->enableDepthTesting();

        gr->set2dMode(-1.0f, 1.0f);
        gr->pushBlendMode( IGraphicsDriver::additive );

        if ( displayUi )
            uiFont->drawString( {displayMode.x - 10.0f, 10.0f}, ( String )"tolcl pre-Alpha -- " + player->loc.x + ", " + player->loc.y, Colour( 0.5f, 0.8f, 1.0f ), IFont::right );
        else
            uiFont->drawString( {displayMode.x - 10.0f, 10.0f}, "Tales of Lanthaia pre-Alpha", Colour( 0.5f, 0.8f, 1.0f ), IFont::right );

//        chatFont->render( 8.0f, 8.0f, table + "\n\npicking: " + picking->getId() );

        gr->popBlendMode();

        while ( chat.getLength() > maxChatLength )
            chat.remove( 0 );

        if ( displayUi )
        {
            for ( unsigned i = 0; i < chat.getLength(); i++ )
                chatFont->drawString( {20.0f, round( displayMode.y - 60.0f - i * chatFont->getLineSkip() * 1.2f )}, chat[chat.getLength() - i - 1], Colour::white(), 0 );

            if ( isTalking )
            {
                gr->drawRectangle( Vector<float>( 0.0f, displayMode.y - 30.0f ), Vector2<float>( displayMode.x, displayMode.y ), Colour( 0.0f, 0.0f, 0.0f, 0.6f ), nullptr );
                chatFont->drawString( {10, (float)displayMode.y - 15}, text + "+", Colour( 1.0f, 1.0f, 1.0f ), IFont::left | IFont::middle );
            }
        }

//        if ( !pickingMatch.isEmpty() )
//            ui->showTooltip( mouseX + 40, mouseY, "\\w\\ Picking: \\s" + pickingMatch );
//        else
//            ui->hideTooltip();

        if (ui) {
            ui->onRender();
        }
    }

    void WorldScene::say( const String& text )
    {
        if ( !text.isEmpty() )
        {
            if ( text.beginsWith( "#" ) )
                parseClientCommand( text.dropLeftPart( 1 ) );
//            else if ( text.beginsWith( "$" ) )
//                scripting->execute( new ArrayIOStream( text.dropLeftPart( 1 ) ) );
            else
                session->chat( text );
        }
    }

    void WorldScene::setDevMode( bool enabled )
    {
        devMode = enabled;
    }

    void WorldScene::spawnWorldObj( const String& name, float x, float y, float orientation )
    {
        Sector* sect = map->getSectorAt( x, y );

        if ( sect )
            sect->addWorldObj( name, x, y, map->getHeightAt( x, y ), orientation );
    }

//    void WorldScene::uiEvent( GameUI::Widget* widget, const String& event )
//    {
//        if ( widget->getName() == "inv_button" && event == "push" )
//        {
//            GameUI::InventoryWindow* inv = new GameUI::InventoryWindow( ui, 50, 50, inventory->getBag( 0 ) );
//            inv->setName( "inventory_window" );
//            inv->setOnClose( this );
//            ui->add( inv );
//        }
//        else if ( widget->getName() == "inventory_window" && event == "close" )
//            ui->destroyWidget( widget );
//    }

    void WorldScene::write( const String& text )
    {
        chat.add( text );
    }

    Player::Player( unsigned pid, const String& name, const Vector<float>& loc, float angle )
            : pid( pid ), name( name ), loc( loc ), angle( angle )
    {
        model = globalResMgr->getModel( "tolcl/model/human_0_sny.ms3d" );
        sword = globalResMgr->getModel( "tolcl/model/sword_0_sny.ms3d" );
//        nameTexture = globalResMgr->getNamedFont( "ui_big" )->render( name, Colour( 1.0f, 1.0f, 1.0f ) );
    }

    Player::~Player()
    {
        model->release();
    }

    bool Player::changeModel( const String& name )
    {
        IModel* newModel = globalResMgr->getModel( "tolcl/model/" + name + ".ms3d" );

        if ( newModel )
        {
            model->release();
            model = newModel;
            return true;
        }
        else
            return false;
    }

    void Player::render()
    {
        {
            Transform transforms[] = {
                    {Transform::translate, loc},
                    {Transform::rotate,    Vector<float>(0.0f, 0.0f, 1.0f), angle},
            };
            model->render(transforms);
        }

        {
            Transform transforms[] = {
                    {Transform::translate, loc},
                    {Transform::rotate,    {0.0f, 0.0f, 1.0f}, angle}, // rotate with player
                    {Transform::translate, {0.2f, 0.15f, 0.6f}}, // position to player
                    {Transform::rotate,    {0.0f, 1.0f, 0.0f}, M_PI / 5.0f}, // roll
                    {Transform::rotate,    {1.0f, 0.0f, 0.0f}, M_PI / 3.0f}, // yaw
            };

            sword->render(transforms);
        }
    }

    void Player::renderName()
    {
//        if ( nameTexture )
//            nameTexture->renderBillboard2( loc + Vector<float>( 0.0f, 0.0f, 2.5f ), nameTexture->getWidth() / 200.0f, nameTexture->getHeight() / 200.0f );
    }
}
