/*
    Tales of Lanthaia Game Client
    Copyright (c) 2011 The Tales of Lanthaia Project

    All rights reserved.
    Created by: Xeatheran Minexew
*/

#pragma once

#include "TolClient.hpp"

#include <Radiance/Radiance.hpp>
#include <StormGraph/GraphicsDriver.hpp>
#include <StormGraph/Scene.hpp>
#include <StormGraph/SoundDriver.hpp>

namespace TolClient
{
    static const unsigned CONNECT_TIMEOUT = 5000;
    static const unsigned LOGIN_TIMEOUT = 10000;

    class TitleScene;

    class TitleScenePreloader : public Thread
    {
        friend class InitScene;
        friend class TitleScene;

        bool failed;
        StormGraph::Exception::Saved exception;

        Reference<ITexture> progressIndicator;
        Reference<ITexturePreload> titleScreenBg;

        TitleScenePreloader( const TitleScenePreloader& );

        public:
            TitleScenePreloader();
            virtual ~TitleScenePreloader();

            virtual void run();
    };

    class LoginSession : public Thread, public Mutex
    {
        friend class TitleScene;

        public:
            /*enum Status { ready, connecting, connected, connectionFailed, timeOut, error, failed };*/

            enum Status { connecting, connected, serverDown, error, registering, loggingIn, failed, loggedIn };
            enum Task { none, loginRequest, registrationRequest };

            struct Info
            {
                Status status;
                String serverName, serverNews, errorDesc;
            };

        protected:
            TitleScene* listener;

            String uri;
            Reference<TcpSocket> socket;

            Info info;
            Task task;

            String username, password;

            void setStatus( Status status );
            void setStatus( Status status, const String& errorDesc );
            void setStatus( Status status, const String& serverName, const String& serverNews );

        public:
            LoginSession( const String& uri, TitleScene* listener );
            virtual ~LoginSession();

            void addTask( Task task, const String& username, const String& password );
            const Info& getInfo();
            virtual void run();
    };

    class TitleScene : public Mutex, public Scene, public Radiance::EventListener
    {
        protected:
            enum
            {
                leftMouseButton,
                printResources,
                maxKey
            };

            Object<TitleScenePreloader> preloader;

            enum { title, mainMenu } state;

            // Multimedia drivers
            GraphicsDriver* driver;
            Var<SoundDriver*> soundDriver;

            // Window, mouse
            Vector2<> windowSize, mouse;

            // Login session stuff
            String realm;
            Object<LoginSession> loginSession;

            MutexVar<bool> statusChanged;

            int16_t keyMappings[maxKey];
            Reference<IFont> font;

            struct
            {
                Reference<IModel> model;
                List<Transform> transforms;

                Reference<SoundSource> music;
            }
            background;

            struct
            {
                float alphaBase;
            }
            pressAnyKey;

            struct
            {
                Reference<IModel> model;
                List<Transform> transforms;
            }
            progressIndicator;

            struct
            {
                Object<Radiance::Styler> styler;
                Object<Radiance::UI> ui;

                Var<Radiance::Window*> aboutDlg, registrationDlg;
                Var<Radiance::Panel*> loginPanel, loggingInPanel, menuPanel;
                Var<Radiance::Label*> statusText;
                Var<Radiance::Image*> activityIndicator, watermark;
            }
            ui;

        private:
            TitleScene( const TitleScene& );
            const TitleScene& operator = ( const TitleScene& );

        protected:
            void messageBox( const Vector<>& size, const String& title, const String& text );

        public:
            TitleScene( GraphicsDriver* driver, const Vector2<unsigned>& windowSize, TitleScenePreloader* preloader );
            virtual ~TitleScene();

            virtual void initialize();

            virtual void onKeyState( int16_t key, Key::State state, Unicode::Char character );
            void onLoginSessionStatus();
            virtual void onMouseMoveTo( const Vector2<int>& mouse );
            virtual void onRadianceEvent( Radiance::Widget* widget, const String& eventName, void* eventProperties );

            virtual void render();
            void sessionError( const String& errorName );
            void showMainMenu();
            virtual void update( double delta );
    };
}

