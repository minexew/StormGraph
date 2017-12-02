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

#include "GameScene.hpp"
#include "TitleScene.hpp"

#include <StormGraph/Engine.hpp>
#include <StormGraph/ResourceManager.hpp>

namespace Duel
{
    ConnectingThread::ConnectingThread( IEngine* engine, const String& address )
            : engine( engine ), statusChanged( false ), status( connecting ), progress( 0.0f )
    {
        int colon = address.findChar( ':' );

        //if ( colon < 0 )
        {
            host = address;
            port = 0xD0E1;
        }
    }

    void ConnectingThread::run()
    {
        // TODO: All the exceptions here (store/rethrow)

        try
        {
            String mapName;

            if ( host == "#noserv" )
            {
                mapName = engine->getVariableValue( "map_name", true );
            }
            else if ( host.beginsWith( '#' ) )
            {
                changeStatus( startingServer, 0.1f );

                Library* serverLibrary = Common::getAppModule( "Duel", "Server", true );

                InterfaceProvider createInterface = serverLibrary->getEntry<InterfaceProvider>( "createInterface" );
                Object<IGameServerProvider> serverProvider = reinterpret_cast<IGameServerProvider*>( createInterface( "Duel.IGameServerProvider" ) );
                SG_assert ( serverProvider != nullptr )

                server = serverProvider->createGameServer( engine );
                server->init();

                mapName = engine->getVariableValue( "map_name", true );

                server->start();
            }
            else
            {
                changeStatus( connecting, 0.1f );

                socket = TcpSocket::create( false );

                bool connectedOk = socket->connect( host, port, true );

                if ( !connectedOk )
                {
                    changeStatus( error, 0.0f );
                    return;
                }
                else
                    changeStatus( connected, 0.2f );

                ArrayIOStream buffer;

                if ( !socket->receive( buffer, Timeout( 5000 ) ) )
                {
                    changeStatus( error, 0.0f );
                    return;
                }

                String gameName = buffer.readString();
                mapName = buffer.readString();

                int numPlayers = buffer.read<int16_t>();
                int maxPlayers = buffer.read<int16_t>();

                printf( "Game: %s [%i/%i players]\n", gameName.c_str(), numPlayers, maxPlayers );
                engine->setVariableValue( "map_name", mapName );

                String playerName = engine->getVariableValue( "playername", true );

                buffer.clear();
                buffer.write<MsgType>( MSG_JOIN );
                buffer.write<uint16_t>( playerName.getNumBytes() );
                buffer.write( playerName, playerName.getNumBytes() );
                socket->write( buffer.getPtr(), buffer.getSize() );

                /*String playerName = sg->getVariableValue( "playername", true );

                ArrayIOStream buffer;
                buffer.write<MsgType>( MSG_PLAYER_HELLO );
                buffer.write<uint16_t>( playerName.getNumBytes() );
                buffer.write( playerName, playerName.getNumBytes() );
                socket->write( buffer.getPtr(), buffer.getSize() );*/
            }

            // FIXME: Check mapName for .. etc.

            map = new Map;

            map->fileSystem = engine->createUnionFileSystem();
            map->fileSystem->add( engine->createFileSystem( "mox:Duel/Maps/" + mapName + ".mox" ) );
            map->fileSystem->add( engine->getFileSystem()->reference() );

            map->resMgr = engine->createResourceManager( "map.resMgr", true, map->fileSystem->reference() );
            map->resMgr->setLoadFlag( LoadFlag::useShadowMapping, true );

            cfx2::Document mapInfo = engine->loadCfx2Asset( "MapInfo.cfx2", true, map->fileSystem );
            String mapDisplayName = mapInfo.queryValue( "Properties/displayName" );

            changeStatus( loading, 0.5f, mapDisplayName + " (CTF)" );

            // TODO: preload

            map->bsp = map->resMgr->getStaticModel( "_BSP", false );

            changeStatus( ready, 1.0f );
        }
        catch ( Exception ex )
        {
            ex.save( savedException );
            changeStatus( exception, 0.0f );
        }
    }

    TitleScene::TitleScene( IEngine* sg )
            : sg( sg )
    {
        graphicsDriver = sg->getGraphicsDriver();
    }

    TitleScene::~TitleScene()
    {
    }

    void TitleScene::init()
    {
        //printf( "@@ TitleScene.init ...\n" );
        resMgr = sg->createResourceManager( "resMgr", true );

        IGuiDriver* guiDriver = sg->getGuiDriver();

        Vector<unsigned> viewport = graphicsDriver->getViewportSize();

        //printf( "@@ Creating GUI ...\n" );
        bigFont = resMgr->getFont( "Common/Fonts/DejaVuSans.ttf", 56, IFont::bold );
        mediumFont = resMgr->getFont( "Common/Fonts/DejaVuSans.ttf", 24, IFont::bold );
        Reference<IFont> menuFont = resMgr->getFont( "Common/Fonts/DejaVuSans.ttf", 24, IFont::normal );

        gui = guiDriver->createGui( Vector<>(), viewport );

        IBoxSizer* sizer1 = gui->createBoxSizer( Orientation::horizontal );
        sizer1->setBounds( Vector<>( 40.0f, viewport.y - 60.0f ), Vector<>( viewport.x - 80.0f, 20.0f ) );

            IStaticText* noserv = gui->createStaticText( Vector<>(), Vector<>(), "Serverless" );
            noserv->setEventListener( this );
            noserv->setFont( menuFont->reference() );
            noserv->setName( "noserv" );
            sizer1->add( noserv );

            IStaticText* nonet = gui->createStaticText( Vector<>(), Vector<>(), "Local Server" );
            nonet->setEventListener( this );
            nonet->setFont( menuFont->reference() );
            nonet->setName( "nonet" );
            sizer1->add( nonet );

            IStaticText* host = gui->createStaticText( Vector<>(), Vector<>(), "Host Server" );
            host->setEventListener( this );
            host->setFont( menuFont->reference() );
            host->setName( "host" );
            sizer1->add( host );

            IStaticText* connect = gui->createStaticText( Vector<>(), Vector<>(), "Connect" );
            connect->setEventListener( this );
            connect->setFont( menuFont->reference() );
            connect->setName( "connect" );
            sizer1->add( connect );

            IStaticText* options = gui->createStaticText( Vector<>(), Vector<>(), "Options" );
            options->setEventListener( this );
            options->setFont( menuFont->reference() );
            options->setName( "options" );
            sizer1->add( options );

            IStaticText* exit = gui->createStaticText( Vector<>(), Vector<>(), "Exit" );
            exit->setEventListener( this );
            exit->setFont( menuFont->reference() );
            exit->setName( "exit" );
            sizer1->add( exit );

        gui->add( sizer1 );

        //printf( "@@ Creating CommandLine ...\n" );
        console = sg->createCommandLine( gui );

        //printf( "@@ Registering listeners ...\n" );
        sg->registerCommandListener( this );
        sg->registerEventListener( gui );

        sg->executeFile( "Duel/main.txt" );
        sg->executeFile( "Duel/main_" StormGraph_BuildTarget ".txt" );

        Reference<IWindow> iwnd = gui->createWindow( Vector<>( 50, 50 ), Vector<>( 400, 300 ), "Herpo" );

            Reference<ITableLayout> layout = gui->createTableLayout( 1 );

                Reference<IStaticText> txt = gui->createStaticText( Vector<>(), Vector<>( 200.0f, 100.0f ), "Hello" );
                layout->add( txt.detach() );

            iwnd->add( layout.detach() );

        iwnd->setScrollable( true );
        //gui->add( iwnd.detach() );

        //printf( "@@ Done setup ...\n" );

        printf( "sizeof( LocalEvent ) = %u\n", unsigned( sizeof( LocalEvent ) ) );
    }

    bool TitleScene::onCommand( const List<String>& tokens )
    {
        /*if ( tokens[0] == "connect" )
        {
            sg->setVariableValue( "hostname", tokens[1] );
            connect();
        }
        else */if ( tokens[0] == "mm.connectTo" )
        {
            sg->setVariableValue( "hostname", tokens[1] );
            startConnecting();
        }
        else
            return false;

        return true;
    }

    void TitleScene::onGuiCommand( const CommandEvent& event )
    {
        if ( String::equals( event.widget->getName(), "noserv" ) )
            sg->command( "mm.connectTo #noserv" );

        if ( String::equals( event.widget->getName(), "nonet" ) )
            sg->command( "mm.connectTo #nonet" );

        if ( String::equals( event.widget->getName(), "host" ) )
            sg->command( "mm.connectTo #" );

        if ( String::equals( event.widget->getName(), "connect" ) )
            sg->command( "mm.connectTo" );

        if ( String::equals( event.widget->getName(), "exit" ) )
            sg->exit();
    }

    void TitleScene::onRender()
    {
        graphicsDriver->set2dMode( -1.0f, 1.0f );

        Vector2<> viewport = graphicsDriver->getViewportSize();
        Vector2<> pos( viewport.x / 2, viewport.y / 4 );
        bigFont->drawString( pos, "Storm", Colour::white(), IFont::centered | IFont::top );

        pos.y = viewport.y / 2;
        mediumFont->drawString( pos, "This is an alpha version product not representing", Colour::white(), IFont::centered | IFont::top );
        pos.y += mediumFont->getLineSkip();
        mediumFont->drawString( pos, "the content nor quality of the final release.", Colour::white(), IFont::centered | IFont::top );
        pos.y += 2 * mediumFont->getLineSkip();
        mediumFont->drawString( pos, "Please report any bugs you may encounter.", Colour::white(), IFont::centered | IFont::top );
        pos.y += mediumFont->getLineSkip();
        mediumFont->drawString( pos, "Thanks for testing.", Colour::white(), IFont::centered | IFont::top );
        pos.y += 4 * mediumFont->getLineSkip();
        mediumFont->drawString( pos, "Copyright (c) 2011 Minexew Games", Colour::grey( 0.4f ), IFont::centered | IFont::top );

        graphicsDriver->drawStats();
    }

    void TitleScene::onUpdate( double delta )
    {
        if ( connecting.thread != nullptr && connecting.thread->statusChanged )
        {
            CriticalSection cs( connecting.thread );

            connecting.thread->statusChanged = false;

            switch ( connecting.thread->status )
            {
                case ConnectingThread::connected:
                    connecting.label->setText( "Connected." );
                    break;

                case ConnectingThread::error:
#ifdef __li_MSW
                    MessageBoxA( nullptr, "Failed to connect to the server.", "Storm", MB_ICONERROR );
#else
                    // FIXME
#endif

                    connecting.thread->waitFor();

                    cs.leave();
                    connecting.thread.release();

                    gui->remove( connecting.panel );
                    return;

                case ConnectingThread::exception:
                    Exception::rethrow( connecting.thread->savedException );

                case ConnectingThread::loading:
                    connecting.label->setText( "Loading " + connecting.thread->mapName + "..." );
                    break;

                case ConnectingThread::ready:
                    connecting.label->setText( "Entering game." );

                    connecting.thread->status = ConnectingThread::ready2;
                    connecting.thread->statusChanged = true;
                    break;

                case ConnectingThread::ready2:
                    sg->changeScene( new GameScene( sg, connecting.thread->map.detach(), connecting.thread->server.detach(), connecting.thread->socket.detach() ) );
                    break;

                case ConnectingThread::startingServer:
                    connecting.label->setText( "Starting game server." );
                    break;
            }

            connecting.progressBar->setProgress( connecting.thread->progress );
        }
    }

    void TitleScene::startConnecting()
    {
        String hostname = sg->getVariableValue( "hostname", true );

        Vector2<> viewport = graphicsDriver->getViewportSize();

        const Vector2<> size( 250.0f, 80.0f );
        const Vector2<> margin( 40.0f, 40.0f );

        const Vector2<> panelPos = viewport - margin - size;

        connecting.panel = gui->createPanel( panelPos, size );
        connecting.panel->setPadding( Vector<>( 10.0f, 10.0f ) );

            ITableLayout* layout = gui->createTableLayout( 1 );
            layout->setColumnGrowable( 0, true );
            layout->setRowGrowable( 0, true );
            //layout->setRowGrowable( 1, true );
            layout->setRowGrowable( 2, true );

                connecting.label = gui->createStaticText( Vector2<>(), Vector2<>(), "Connecting to " + hostname + "..." );
                layout->add( connecting.label );

                connecting.progressBar = gui->createProgressBar();
                connecting.progressBar->setExpand( true );
                connecting.progressBar->setMinSize( Vector2<>( 0.0f, 10.0f ) );
                connecting.progressBar->setPadding( Vector2<>( 2.0f, 2.0f ) );
                connecting.progressBar->setProgress( 0.1f );
                layout->add( connecting.progressBar );

                //IButton* cancelButton = gui->createButton( Vector2<>(), Vector2<>(), "Cancel" );
                //cancelButton->setAlign( ISizableWidget::right | ISizableWidget::middle );
                //layout->add( cancelButton );

            connecting.panel->add( layout );

        gui->add( connecting.panel );

        connecting.thread = new ConnectingThread( sg, hostname );
        connecting.thread->start();
    }

    void TitleScene::uninit()
    {
        sg->unregisterCommandListener( this );
        sg->unregisterEventListener( gui );
    }
}