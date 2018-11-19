
#include "InitScene.hpp"
#include "WorldScene.hpp"

#include <Radiance/EpicStyler.hpp>

#include <StormGraph/Engine.hpp>
#include <StormGraph/ResourceManager.hpp>

// TODO: [$01 GROUP] Temporary, pre-multichar approach

namespace TolClient
{
    TitleScenePreloader::TitleScenePreloader()
            : failed( false ), titleScreenBg( 0 )
    {
    }

    TitleScenePreloader::~TitleScenePreloader()
    {
        titleScreenBg.release();

        ResourceManager* uiResMgr = Resources::getUiResMgr( false );

        if ( uiResMgr )
            uiResMgr->releaseUnused();
    }

    void TitleScenePreloader::run()
    {
        try
        {
            ResourceManager* uiResMgr = Resources::getUiResMgr();
            uiResMgr->addPath( "" );

            ResourceManager* musicResMgr = Resources::getMusicResMgr();
            musicResMgr->addPath( "" );

            uiResMgr->getTexturePreload( "TolClient/UI/TitleScreen.jpg", 0, &titleScreenBg );

            //Sleep( 500 );
            printf( "TitleScenePreloader: Done loading.\n" );
        }
        catch ( Exception ex )
        {
            failed = true;
            ex.save( exception );
        }
    }

    LoginSession::LoginSession( const String& uri, TitleScene* listener )
            : listener( listener ), uri( uri ), task( none )
    {
        info.status = connecting;
    }

    LoginSession::~LoginSession()
    {
    }

    void LoginSession::addTask( Task task, const String& username, const String& password )
    {
        CriticalSection cs( this );

        this->task = task;
        this->username = username;
        this->password = password;
    }

    const LoginSession::Info& LoginSession::getInfo()
    {
        return info;
    }

    void LoginSession::run()
    {
        unsigned numCharacters;

        setStatus( connecting );
        listener->onLoginSessionStatus();

        socket = new TcpSocket( false );

        // Parse Server URI
        Uri::Parts uriParts;
        Uri::parse( uri, uriParts );
        uri.clear();
        int port = uriParts.port.isEmpty() ? 24897 : uriParts.port.toInt();

        // Try to connect
        if ( !socket->connect( uriParts.host, port ) )
        {
            setStatus( error, "local.err_connect" );
            listener->onLoginSessionStatus();
            return;
        }

        if ( shouldEnd )
            return;

        // Send the hello
        ArrayIOStream message;
        message.writeString( "login.client_hello" );
        message.write<uint32_t>( clientVersion );
        socket->send( message );

        // Main message processing loop
        while ( true )
        {
            CriticalSection cs( this );

            if ( shouldEnd || info.status == LoginSession::error )
                break;

            cs.leave();

            switch ( info.status )
            {
                // Connecting (wait for the server info message)
                case connecting:
                {
                    if ( !socket->receive( message, Timeout( CONNECT_TIMEOUT ) ) )
                    {
                        setStatus( error, "local.err_timeout" );
                        listener->onLoginSessionStatus();
                        return;
                    }

                    message.dump();
                    String messageName = message.readString();

                    if ( messageName == "login.server_info" )
                    {
                        String realmName = message.readString();
                        String realmNews = message.readString();
                        setStatus( connected, realmName, realmNews );
                        listener->onLoginSessionStatus();
                    }
                    else if ( messageName == "login.server_down" )
                    {
                        String reason = message.readString();
                        setStatus( serverDown, reason );
                        listener->onLoginSessionStatus();
                    }
                    else
                    {
                        setStatus( error, messageName );
                        listener->onLoginSessionStatus();
                        return;
                    }

                    break;
                }

                // Connected
                case connected:
                    if ( socket->receive( message ) )
                    {
                        String messageName = message.readString();

                        setStatus( error, messageName );
                        listener->onLoginSessionStatus();
                        return;
                    }
                    else if ( task == loginRequest )
                    {
                        message.clear();
                        message.writeString( "login.login_request" );
                        message.writeString( username );
                        message.writeString( password );

                        if ( !socket->send( message ) )
                        {
                            setStatus( error, "local.err_send" );
                            listener->onLoginSessionStatus();
                        }

                        setStatus( loggingIn );
                        listener->onLoginSessionStatus();

                        task = none;
                    }
                    else if ( task == registrationRequest )
                    {
                        message.clear();
                        message.writeString( "login.registration_request" );
                        message.writeString( username );
                        message.writeString( password );

                        if ( !socket->send( message ) )
                        {
                            setStatus( error, "local.err_send" );
                            listener->onLoginSessionStatus();
                        }

                        setStatus( registering );
                        listener->onLoginSessionStatus();

                        task = none;
                    }
                    else
                        pauseThread( 100 );
                    break;

                // Logging in, registering
                case loggingIn:
                case registering:
                {
                    if ( !socket->receive( message, Timeout( LOGIN_TIMEOUT ) ) )
                    {
                        setStatus( error, "local.err_timeout" );
                        listener->onLoginSessionStatus();
                        return;
                    }

                    message.dump();
                    String messageName = message.readString();

                    if ( messageName != "result.ok" )
                    {
                        setStatus( failed, messageName );
                        listener->onLoginSessionStatus();
                        continue;
                    }

                    setStatus( loggedIn );
                    listener->onLoginSessionStatus();
                    break;
                }

                // Logged in
                case loggedIn:
                    if ( socket->receive( message ) )
                    {
                        message.dump();
                        String messageName = message.readString();

                        // TODO: [$01] Begin of Automatic enter-world
                        if ( messageName == "result.ok" )
                        {
                            break;
                        }
                        else if ( messageName == "login.entering_world" )
                        {
                            MessageBoxA( 0, "Entering world!", "Success", MB_OK );
                            //sg->changeScene( new WorldScene( 0 ) );
                            return;
                        }
                        // TODO: [$01] End of Automatic enter-world

                        if ( messageName != "login.character_info" )
                        {
                            setStatus( error, messageName );
                            listener->onLoginSessionStatus();
                            return;
                        }

                        // Display character selection
                        // TODO: [$01] Auto-create character
                        numCharacters = message.read<uint16_t>();

                        if ( numCharacters < 1 )
                        {
                            message.clear();
                            message.writeString( "login.create_character" );
                            message.writeString( username );
                            message.write<uint16_t>( 0 );

                            if ( !socket->send( message ) )
                            {
                                setStatus( error, "local.err_send" );
                                listener->onLoginSessionStatus();
                            }
                        }
                        else
                        {
                            message.clear();
                            message.writeString( "login.enter_world" );
                            message.write<uint16_t>( 0 );

                            if ( !socket->send( message ) )
                            {
                                setStatus( error, "local.err_send" );
                                listener->onLoginSessionStatus();
                            }
                            break;
                        }
                    }
                    else
                        pauseThread( 100 );
                    break;

                default:
                    pauseThread( 100 );
            }
        }
    }

    void LoginSession::setStatus( Status status )
    {
        CriticalSection cs( this );

        info.status = status;
    }

    void LoginSession::setStatus( Status status, const String& errorDesc )
    {
        CriticalSection cs( this );

        info.status = status;
        info.errorDesc = errorDesc;
    }

    void LoginSession::setStatus( Status status, const String& serverName, const String& serverNews )
    {
        CriticalSection cs( this );

        info.status = status;
        info.serverName = serverName;
        info.serverNews = serverNews;
    }

    TitleScene::TitleScene( GraphicsDriver* driver, const Vector2<unsigned>& windowSize, TitleScenePreloader* preloader )
            : preloader( preloader ), state( title ), driver( driver ), windowSize( windowSize ), statusChanged( *this )
    {
    }

    TitleScene::~TitleScene()
    {
        if ( loginSession && loginSession->isRunning() )
        {
            // TODO Do not wait here, destroy engine first
            loginSession->end();
            loginSession->waitFor();
        }
    }

    void TitleScene::initialize()
    {
        ResourceManager* uiResMgr = Resources::getUiResMgr();
        ResourceManager* musicResMgr = Resources::getMusicResMgr();

        font = uiResMgr->getFont( "Radiance.EpicStyler.Assets/DefaultFont.ttf", 20 );

        // "Press any key"
        pressAnyKey.alphaBase = 0.0f;

        // Background
        IMaterial* material = driver->createSolidMaterial( "TitleBgMat", Colour::white(), preloader->titleScreenBg->getFinalized() );

        // Background Plane
        {
            PlaneCreationInfo plane;
            plane.dimensions = windowSize;
            plane.origin = Vector<>();
            plane.uv0 = Vector2<>( 0.0f, 0.0f );
            plane.uv1 = Vector2<>( 1.0f, 1.0f );
            plane.withNormals = false;
            plane.withUvs = true;
            plane.material = material;
            background.model = driver->createPlane( "TitleBg", &plane );
        }

        // Keyz
        keyMappings[leftMouseButton] = driver->getKey( "Left Mouse Button" );
        keyMappings[printResources] = driver->getKey( "R" );

        // Unseres Netzwerkthread
        realm = Engine::getInstance()->getConfig( "TolClient/realm" );
        loginSession = new LoginSession( realm, this );
        loginSession->start();

        driver->setClearColour( Colour( 0.0f, 0.0f, 0.0f, 0.0f ) );
        driver->set2dMode( -1.0f, 1.0f );

        // UI
        ui.styler = new Radiance::EpicStyler( driver, uiResMgr->reference() );
        ui.ui = new Radiance::UI( ui.styler, Vector<>(), windowSize );

        {
            cfx2::Document activityIndicatorDoc = sg->loadCfx2Asset( "TolClient/UI/activityIndicator.cfx2" );
            ui.activityIndicator = new Radiance::Image( ui.styler, Vector<>( 32.0f, windowSize.y - 32.0f ), activityIndicatorDoc );
            ui.activityIndicator->hide();
            ui.ui->add( ui.activityIndicator );

            cfx2::Document watermarkDoc = sg->loadCfx2Asset( "TolClient/UI/watermark.cfx2", false );

            if ( watermarkDoc )
            {
                ui.watermark = new Radiance::Image( ui.styler, Vector<>( windowSize.x, 0.0f ), watermarkDoc );
                ui.ui->add( ui.watermark );
            }
        }

        ui.statusText = new Radiance::Label( ui.styler, Vector<>( 56.0f, windowSize.y - 20.0f ), String(), IFont::left | IFont::bottom );
        ui.ui->add( ui.statusText );

        ui.ui->load( "TolClient/UI/TitleScreenUI.cfx2", this );

        ui.aboutDlg =           dynamic_cast<Radiance::Window*>( ui.ui->findWidget( "aboutDlg" ) );
        ui.registrationDlg =    dynamic_cast<Radiance::Window*>( ui.ui->findWidget( "registrationDlg" ) );
        ui.loginPanel =         dynamic_cast<Radiance::Panel*>( ui.ui->findWidget( "loginPanel" ) );
        ui.loggingInPanel =     dynamic_cast<Radiance::Panel*>( ui.ui->findWidget( "loggingInPanel" ) );
        ui.menuPanel =          dynamic_cast<Radiance::Panel*>( ui.ui->findWidget( "menuPanel" ) );

        if ( !ui.aboutDlg || !ui.registrationDlg || !ui.loginPanel || !ui.loggingInPanel || !ui.menuPanel )
            throw StormGraph::Exception( "TolClient.TitleScene.initialize", "UiError", "Game User Interface is corrupted or incomplete." );

        ui.loginPanel->moveTo( Vector<>( ( windowSize.x - ui.loginPanel->getSize().x ) / 2, windowSize.y * 3.0f / 5.0f ).round() );
        ui.loggingInPanel->moveTo( Vector<>( ( windowSize.x - ui.loggingInPanel->getSize().x ) / 2, windowSize.y * 3.0f / 5.0f ).round() );
        ui.menuPanel->moveTo( Vector<>( windowSize ) - ui.menuPanel->getSize() - Vector<>( 20.0f, 20.0f ).round() );

        soundDriver = sg->getSoundDriver();

        if ( soundDriver )
        {
            background.music = soundDriver->createSoundSource( musicResMgr->getSoundStream( "TolClient/Music/ToL_main_theme_d01.ogg" ) );
            background.music->play();
        }

        //
        preloader.release();
    }

    void TitleScene::messageBox( const Vector<>& size, const String& title, const String& text )
    {
        Radiance::MessageBox* msgBox = new Radiance::MessageBox( ui.styler, ( Vector<>( windowSize ) - size ) / 2, size, title, text );
        msgBox->setName( "msgBox" );
        msgBox->setOnClose( this );
        ui.ui->doModal( msgBox );
    }

    void TitleScene::onKeyState( int16_t key, Key::State state, Unicode::Char character )
    {
        if ( ui.ui && ui.ui->onKeyState( character, state ) )
            return;

        if ( key == keyMappings[leftMouseButton] )
        {
            if ( state == Key::pressed )
                ui.ui->mouseDown( mouse );
            else if ( state == Key::released )
                ui.ui->mouseUp( mouse );
        }

        if ( this->state == title && state == Key::pressed )
            showMainMenu();

        if ( key == keyMappings[printResources] && state == Key::pressed )
            Resource::listResources();
    }

    void TitleScene::onLoginSessionStatus()
    {
        statusChanged = true;
    }

    void TitleScene::onMouseMoveTo( const Vector2<int>& mouse )
    {
        if ( ui.ui )
        {
            ui.ui->mouseMove( mouse );

            this->mouse = mouse;
        }
    }

    void TitleScene::onRadianceEvent( Radiance::Widget* widget, const String& eventName, void* eventProperties )
    {
        if ( widget->getName() == "about" )
        {
            ui.aboutDlg->moveTo( ( windowSize - ui.aboutDlg->getSize().getXy() ) / 2 );
            ui.aboutDlg->show();
        }
        else if ( widget->getName() == "aboutCloseBtn" && eventName == "push" )
            ui.aboutDlg->hide();
        else if ( widget->getName() == "loginBtn" && eventName == "push" )
        {
            CriticalSection cs( loginSession );

            if ( loginSession->getInfo().status != LoginSession::connected )
                return;

            cs.leave();

            ui.aboutDlg->hide();
            ui.registrationDlg->hide();

            ui.loginPanel->hide();
            ui.menuPanel->hide();

            ui.loggingInPanel->show();

            Radiance::Input* username = dynamic_cast<Radiance::Input*>( ui.loginPanel->findWidget( "username" ) );
            Radiance::Input* password = dynamic_cast<Radiance::Input*>( ui.loginPanel->findWidget( "password" ) );

            if ( username && password )
                loginSession->addTask( LoginSession::loginRequest, username->getText(), password->getText() );
        }
        else if ( widget->getName() == "msgBox" && eventName == "close" )
        {
            ui.ui->remove( widget );
            showMainMenu();
        }
        else if ( widget->getName() == "register" )
        {
            ui.registrationDlg->moveTo( ( windowSize - ui.registrationDlg->getSize().getXy() ) / 2 );
            ui.registrationDlg->show();
        }
        else if ( widget->getName() == "registrationRegisterBtn" )
        {
            CriticalSection cs( loginSession );

            if ( loginSession->getInfo().status != LoginSession::connected )
                return;

            cs.leave();

            Radiance::Input* username = dynamic_cast<Radiance::Input*>( ui.registrationDlg->findWidget( "username" ) );
            Radiance::Input* password = dynamic_cast<Radiance::Input*>( ui.registrationDlg->findWidget( "password" ) );

            if ( username && password )
            {
                loginSession->addTask( LoginSession::registrationRequest, username->getText(), password->getText() );
                ui.registrationDlg->hide();
            }
        }
    }

    void TitleScene::render()
    {
        background.model->render( background.transforms );

        if ( state == title )
            font->renderString( windowSize.x / 2, windowSize.y  * 2 / 3, "- Press any key -", Colour( 1.0f, 1.0f, 1.0f, sin( fabs( pressAnyKey.alphaBase ) ) ), IFont::centered | IFont::middle );

        if ( ui.ui )
            ui.ui->render();
    }

    void TitleScene::sessionError( const String& errorName )
    {
        if ( errorName == "result.err_account_exists" )
            messageBox( Vector<>( 500.0f, 144.0f ), "\\rError", sg->getString( "account_exists" ) + "\n" + sg->getString( "please_choose_different_name" ) );
        else if ( errorName == "result.err_login_incorrect" )
            messageBox( Vector<>( 440.0f, 144.0f ), "\\rError", sg->getString( "login_incorrect" ) + "\n" + sg->getString( "please_try_again" ) );
        else if ( errorName == "result.err_name_invalid" )
        {
            // TODO: The same message for character creation
            messageBox( Vector<>( 400.0f, 144.0f ), "\\rError", sg->getString( "specified_username_invalid" ) + "\n" + sg->getString( "please_choose_different_name" ) );
        }
        else
            messageBox( Vector<>( 500.0f, 100.0f ), "\\rError", sg->getString( "unknown_error" ) + ": \\l" + errorName );
    }

    void TitleScene::showMainMenu()
    {
        ui.aboutDlg->hide();
        ui.registrationDlg->hide();

        ui.loginPanel->show();
        ui.menuPanel->show();

        ui.loggingInPanel->hide();

        this->state = mainMenu;
    }

    void TitleScene::update( double delta )
    {
        if ( state == title )
        {
            pressAnyKey.alphaBase += delta / 0.5f;

            while ( pressAnyKey.alphaBase > 1.0f )
                pressAnyKey.alphaBase -= 2.0f;
        }

        if ( ui.ui )
        {
            ui.ui->update( delta );

            if ( statusChanged )
            {
                String status;
                bool activity = false;

                CriticalSection cs( loginSession );

                const LoginSession::Info& loginSessionInfo = loginSession->getInfo();

                switch ( loginSessionInfo.status )
                {
                    case LoginSession::connecting:
                        status = "\\wConnecting to \\l" + realm + "\\w...";
                        activity = true;
                        break;

                    case LoginSession::connected:
                        status = "\\lConnected to " + loginSessionInfo.serverName + ".";
                        break;

                    case LoginSession::loggingIn:
                        status = "\\lLogging in...";
                        activity = true;
                        break;

                    case LoginSession::registering:
                        status = "\\l" + sg->getString( "registering" );
                        activity = true;
                        break;

                    case LoginSession::failed:
                        status = "\\lConnected to " + loginSessionInfo.serverName + ".";
                        sessionError( loginSessionInfo.errorDesc );
                        loginSession->info.status = LoginSession::connected;
                        break;

                    case LoginSession::error:
                        if ( loginSessionInfo.errorDesc == "local.err_connect" )
                            status = "\\r" + sg->getString( "unable_to_connect" );
                        else if ( loginSessionInfo.errorDesc == "local.err_timeout" )
                            status = "\\r" + sg->getString( "connection_timed_out" );
                        else
                            status = "\\r" + sg->getString( "unknown_error" ) + ": \\w" + loginSessionInfo.errorDesc;
                        break;

                    case LoginSession::serverDown:
                        status = "\\o" + sg->getString( "server_maintenance" ) + ": \\w" + loginSessionInfo.errorDesc;
                        break;

                    default:
                        ;
                }

                cs.leave();

                ui.statusText->setText( status );
                statusChanged = false;

                if ( activity )
                    ui.activityIndicator->show();
                else
                    ui.activityIndicator->hide();
            }

            if ( ui.activityIndicator )
                ui.activityIndicator->setRotation( ui.activityIndicator->getRotation() - M_PI * 4.0f * delta );
        }
    }
}
