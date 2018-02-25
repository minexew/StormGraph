
#include "GameClient.hpp"

#include "UI.hpp"

namespace GameUI
{
    class CharacterCreationDlg : public Window, EventListener
    {
        EventListener* onCancel, * onConfirm;

        public:
            CharacterCreationDlg( int x, int y );
            virtual ~CharacterCreationDlg();

            void setOnCancel( EventListener* listener );
            void setOnConfirm( EventListener* listener );
            void uiEvent( Widget* widget, const li::String& event );
    };

    class RegistrationDlg : public Window, EventListener
    {
        EventListener* onCancel, * onConfirm;

        public:
            RegistrationDlg( int x, int y );
            virtual ~RegistrationDlg();

            void setOnCancel( EventListener* listener );
            void setOnConfirm( EventListener* listener );
            void uiEvent( Widget* widget, const li::String& event );
    };
}

namespace GameClient
{
    enum LoginState
    {
        Login_connecting,
        Login_ready,
        Login_error,
        Login_characters
    };

    class MainMenuScene : public Scene, GameUI::EventListener
    {
        LoginState state;

        GameUI::UI* ui;
        GameUI::Panel* loginPanel, * characterPanel;
        GameUI::Label* hello, * status;
        GameUI::RegistrationDlg* registerDlg;
        GameUI::CharacterCreationDlg* newCharDlg;

        TcpSocket* socket;
        StreamBuffer<> buffer;

        public:
            MainMenuScene();
            ~MainMenuScene();

            virtual void keyStateChange( unsigned short key, bool pressed, Utf8Char character );
            void messageBox( const char* text, const char* title, int w, GameUI::Panel* parent = 0 );
            virtual void mouseButton( int x, int y, bool right, bool down );
            virtual void mouseMove( int x, int y );
            virtual void render();
            virtual void uiEvent( GameUI::Widget* widget, const String& event );
    };
}
