
#include "GameScene.hpp"
#include "MainMenu.hpp"

/*
    TODO: (requires: timing)
        send keep_alive
        connection retry
*/

namespace GameClient
{
    static const uint32_t clientVersion = 2;

    /*class ConnectionThread : public Thread
    {
    };*/

    MainMenuScene::MainMenuScene() : state( Login_connecting )
    {
        const char* hostname = sg->getConfig( "tolcl/realm" );
        int port = 24897;

        Vector<unsigned short> display = sg->getDisplayMode();

        ui = new GameUI::UI( display.x, display.y );
        ui->add( new GameUI::StaticImage( 0, 0, new Texture( "tolcl/gfx/title.jpg" ), display.x, display.y ) );

        loginPanel = new GameUI::Panel( 40, display.y - 280, display.x - 80, 200, true );

            loginPanel->add( new GameUI::Label( 40, 64, "login:" ) );
            GameUI::Input* loginInput = new GameUI::Input( 140, 64, 200, 24 );
            loginInput->setName( "login_input" );
            loginPanel->add( loginInput );

            loginPanel->add( new GameUI::Label( 40, 96, "password:" ) );
            GameUI::Input* passInput = new GameUI::Input( 140, 96, 200, 24 );
            passInput->setName( "pass_input" );
            loginPanel->add( passInput );

            GameUI::Button* loginButton = new GameUI::Button( 40, 136, 160, 40, "login" );
            loginButton->setName( "login_btn" );
            loginButton->setOnPush( this );
            loginPanel->addWidget( loginButton );

            GameUI::Button* registerButton = new GameUI::Button( 240, 136, 160, 40, "registration..." );
            registerButton->setName( "register_btn" );
            registerButton->setOnPush( this );
            loginPanel->addWidget( registerButton );

            hello = new GameUI::Label( 40, 30, 0 );
            loginPanel->addWidget( hello );

        ui->add( loginPanel );

        GameUI::Panel* statusPanel = new GameUI::Panel( 0, display.y - 40, display.x, 40, true );

            status = new GameUI::Label( 10, 10, ( String )"connecting to " + hostname + ":" + port + "..." );
            statusPanel->addWidget( status );

        ui->add( statusPanel );

        characterPanel = new GameUI::Panel( 40, display.y / 5, display.x - 80, display.y / 5 * 3, true, Colour( 0.0f, 0.0f, 0.0f, 0.5f ) );

            GameUI::Button* newCharButton = new GameUI::Button( 40, display.y / 5 * 3 - 80, 200, 40, "create character" );
            newCharButton->setName( "new_char_btn" );
            newCharButton->setOnPush( this );
            characterPanel->addWidget( newCharButton );

        characterPanel->disable();

        ui->add( characterPanel );

        registerDlg = new GameUI::RegistrationDlg( 200, 160 );
        registerDlg->setName( "register_dlg" );
        registerDlg->setOnCancel( this );
        registerDlg->setOnConfirm( this );

        newCharDlg = new GameUI::CharacterCreationDlg( 200, 160 );
        newCharDlg->setName( "new_char_dlg" );
        newCharDlg->setOnCancel( this );
        newCharDlg->setOnConfirm( this );

        socket = new TcpSocket( false );
        socket->connect( hostname, port, true );

        sg->setOrthoProjection();
    }

    MainMenuScene::~MainMenuScene()
    {
        socket->release();
        delete ui;
    }

    void MainMenuScene::keyStateChange( unsigned short key, bool pressed, Utf8Char character )
    {
        if ( pressed )
            ui->keyDown( character );
    }

    void MainMenuScene::messageBox( const char* text, const char* title, int w, GameUI::Panel* parent )
    {
        Vector<unsigned short> display = sg->getDisplayMode();

        const int h = 120;

        if ( !parent )
            parent = ui;

        GameUI::MessageBox* mb = new GameUI::MessageBox( text, title, ( display.x - w ) / 2, ( display.y - h ) / 2, w, h );
        mb->setName( "msgbox" );
        mb->setOnClose( this );
        parent->doModal( mb );
    }

    void MainMenuScene::mouseButton( int x, int y, bool right, bool down )
    {
        if ( !right )
        {
            if ( down )
                ui->mouseDown( x, y );
            else
                ui->mouseUp( x, y );
        }
    }

    void MainMenuScene::mouseMove( int x, int y )
    {
        ui->mouseMove( x, y );
    }

    void MainMenuScene::render()
    {
        if ( state == Login_connecting )
        {
            if ( socket->isWritable() )
            {
                buffer.clear();
                buffer.writeString( "login.client_hello" );
                buffer.write<uint32_t>( clientVersion );
                socket->send( buffer );

                state = Login_ready;
                status->setText( "Connected." );
            }
        }
        else if ( socket->receive( buffer ) )
        {
            String message = buffer.readString();

            if ( message == "login.server_info" )
            {
                String serverName = buffer.readString();
                String serverNews = buffer.readString();

                hello->setText( "\\g" + serverNews );
                status->setText( "\\g" "Connected to \\w" + serverName + "\\g." );
            }
            else if ( message == "login.character_info" )
            {
                status->setText( "\\g" "Character list obtained." );

                Vector<unsigned short> display = sg->getDisplayMode();

                for ( unsigned i = 0; i < 5; i++ )
                {
                    GameUI::Widget* panel = characterPanel->findWidget( ( String )"char_panel_" + i );

                    if ( panel )
                    {
                        characterPanel->remove( panel );
                        ui->destroyWidget( panel );
                    }
                }

                unsigned count = buffer.read<uint16_t>();

                for ( unsigned i = 0; i < count; i++ )
                {
                    String name = buffer.readString();
                    String location = buffer.readString();
                    /*unsigned race = */buffer.read<uint16_t>();
                    /*unsigned classID = */buffer.read<uint16_t>();
                    unsigned level = buffer.read<uint16_t>();

                    GameUI::Panel* panel = new GameUI::Panel( ( display.x - 80 ) / 26 * ( 1 + i * 5 ), 40, ( display.x - 80 ) / 26 * 4, 100, true );
                    panel->setName( ( String )"char_panel_" + i );
                    panel->setOnClick( this );

                        panel->add( new GameUI::Label( 12, 12, "\\#696" + name ) );
                        panel->add( new GameUI::Label( 12, 40, ( String )"\\#159" "Level " + level + " Race\n" "\\#579" "-class-\n" "\\#999" + location, "ui_small" ) );

                    characterPanel->add( panel );
                }

                loginPanel->disable();
                characterPanel->enable();
                ui->remove( registerDlg );
            }
            else if ( message == "login.entering_world" )
            {
                sg->changeScene( new GameScene( socket->reference() ) );
                return;
            }
            else if ( message == "login.server_down" )
            {
                messageBox( "Server is down for maintenance: " + buffer.readString(), "Server down", 640 );
                state = Login_error;
                status->setText( "\\r" "Server down." );
            }
            else if ( message == "result.err_account_exists" )
                messageBox( "An account with that name already exists.", "Registration failed", 480 );
            else if ( message == "result.err_client_version" )
            {
                messageBox( "Your game client is probably outdated.", "Version mismatch", 480 );
                state = Login_error;
                status->setText( "\\r" "Invalid client version." );
            }
            else if ( message == "result.err_login_incorrect" )
                messageBox( "Invalid username and/or password.", "Login failed", 400 );
            else
                status->setText( "\\o" + message );
        }

        ui->render();
        ui->update();
    }

    void MainMenuScene::uiEvent( GameUI::Widget* widget, const String& event )
    {
        if ( widget->getName() == "login_btn" && event == "push" )
        {
            GameUI::Input* loginInput = ( GameUI::Input* )loginPanel->findWidget( "login_input" );
            GameUI::Input* passInput = ( GameUI::Input* )loginPanel->findWidget( "pass_input" );

            if ( state != Login_ready )
                messageBox( "You are not connected to the game server.", "Connection required", 480 );
            else if ( !loginInput->getText().isEmpty() && !passInput->getText().isEmpty() )
            {
                buffer.clear();
                buffer.writeString( "login.login_request" );
                buffer.writeString( loginInput->getText() );
                buffer.writeString( passInput->getText() );
                socket->send( buffer );

                status->setText( "Logging in..." );
            }
        }
        else if ( widget->getName() == "register_btn" && event == "push" )
            ui->doModal( registerDlg );
        else if ( ( widget->getName() == "register_dlg" || widget->getName() == "new_char_dlg" ) && event == "cancel" )
            ui->remove( widget );
        else if ( widget->getName() == "register_dlg" && event == "confirm" )
        {
            GameUI::Input* loginInput = ( GameUI::Input* )widget->findWidget( "login_input" );
            GameUI::Input* passInput = ( GameUI::Input* )widget->findWidget( "pass_input" );

            if ( state != Login_ready )
                messageBox( "You are not connected to the game server.", "Connection required", 480, registerDlg );
            else if ( !loginInput->getText().isEmpty() && !passInput->getText().isEmpty() )
            {
                buffer.clear();
                buffer.writeString( "login.registration_request" );
                buffer.writeString( loginInput->getText() );
                buffer.writeString( passInput->getText() );
                socket->send( buffer );

                ui->remove( registerDlg );

                status->setText( "Registration request sent." );
            }
        }
        else if ( widget->getName() == "new_char_dlg" && event == "confirm" )
        {
            GameUI::Input* nameInput = ( GameUI::Input* )widget->findWidget( "name_input" );

            if ( !nameInput->getText().isEmpty() )
            {
                buffer.clear();
                buffer.writeString( "login.create_character" );
                buffer.writeString( nameInput->getText() );
                buffer.write<uint16_t>( 0 );
                socket->send( buffer );

                ui->remove( newCharDlg );
            }
        }
        else if ( widget->getName() == "msgbox" && event == "close" )
        {
            ( ( GameUI::Panel* )widget->getModalParent() )->removeWidget( widget );
            ui->destroyWidget( widget );
        }
        else if ( widget->getName().beginsWith( "char_panel_" ) && event == "click" )
        {
            buffer.clear();
            buffer.writeString( "login.enter_world" );
            buffer.write<uint16_t>( widget->getName().rightPart( 1 ) );
            socket->send( buffer );
        }
        else if ( widget->getName() == "new_char_btn" && event == "push" )
        {
            ui->doModal( newCharDlg );
        }
    }
}

namespace GameUI
{
    RegistrationDlg::RegistrationDlg( int x, int y )
            : Window( 0, "Account Registration", x, y, 400, 192 ), onCancel( 0 ), onConfirm( 0 )
    {
        addWidget( new GameUI::Label( 40, 40, "login:" ) );
        GameUI::Input* loginInput = new GameUI::Input( 140, 40, 200, 24 );
        loginInput->setName( "login_input" );
        addWidget( loginInput );

        addWidget( new GameUI::Label( 40, 70, "password:" ) );
        GameUI::Input* passInput = new GameUI::Input( 140, 70, 200, 24 );
        passInput->setName( "pass_input" );
        addWidget( passInput );

        GameUI::Button* confirmButton = new GameUI::Button( 40, 128, 128, 32, "register" );
        confirmButton->setName( "confirm_btn" );
        confirmButton->setOnPush( this );
        addWidget( confirmButton );

        GameUI::Button* cancelButton = new GameUI::Button( 232, 128, 128, 32, "cancel" );
        cancelButton->setName( "cancel_btn" );
        cancelButton->setOnPush( this );
        addWidget( cancelButton );
    }

    RegistrationDlg::~RegistrationDlg()
    {
    }

    void RegistrationDlg::setOnCancel( EventListener* listener )
    {
        onCancel = listener;
    }

    void RegistrationDlg::setOnConfirm( EventListener* listener )
    {
        onConfirm = listener;
    }

    void RegistrationDlg::uiEvent( Widget* widget, const li::String& event )
    {
        if ( widget->getName() == "cancel_btn" && event == "push" && onCancel )
            onCancel->uiEvent( this, "cancel" );

        if ( widget->getName() == "confirm_btn" && event == "push" && onConfirm )
            onConfirm->uiEvent( this, "confirm" );
    }

    CharacterCreationDlg::CharacterCreationDlg( int x, int y )
            : Window( 0, "Create Character", x, y, 400, 192 ), onCancel( 0 ), onConfirm( 0 )
    {
        addWidget( new GameUI::Label( 40, 40, "name:" ) );
        GameUI::Input* nameInput = new GameUI::Input( 140, 40, 200, 24 );
        nameInput->setName( "name_input" );
        add( nameInput );

        //addWidget( new GameUI::Label( 40, 70, "race:" ) );

        GameUI::Button* confirmButton = new GameUI::Button( 40, 128, 128, 32, "create" );
        confirmButton->setName( "confirm_btn" );
        confirmButton->setOnPush( this );
        add( confirmButton );

        GameUI::Button* cancelButton = new GameUI::Button( 232, 128, 128, 32, "cancel" );
        cancelButton->setName( "cancel_btn" );
        cancelButton->setOnPush( this );
        add( cancelButton );
    }

    CharacterCreationDlg::~CharacterCreationDlg()
    {
    }

    void CharacterCreationDlg::setOnCancel( EventListener* listener )
    {
        onCancel = listener;
    }

    void CharacterCreationDlg::setOnConfirm( EventListener* listener )
    {
        onConfirm = listener;
    }

    void CharacterCreationDlg::uiEvent( Widget* widget, const li::String& event )
    {
        if ( widget->getName() == "cancel_btn" && event == "push" && onCancel )
            onCancel->uiEvent( this, "cancel" );

        if ( widget->getName() == "confirm_btn" && event == "push" && onConfirm )
            onConfirm->uiEvent( this, "confirm" );
    }
}
