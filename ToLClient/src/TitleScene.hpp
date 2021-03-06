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

#include <littl/TcpSocket.hpp>
#include <littl/Thread.hpp>

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

    class TitleScene : public Mutex, public IScene, public Radiance::EventListener
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
            IGraphicsDriver* driver;
            ISoundDriver* soundDriver = nullptr;

            // Window, mouse
            Vector2<> windowSize, mouse;

            // Login session stuff
            String realm;
            Object<LoginSession> loginSession;

            volatile bool statusChanged = false;

            int16_t keyMappings[maxKey];
            Reference<IFont> font;

            struct
            {
                Reference<IModel> model;
                List<Transform> transforms;

                Reference<ISoundSource> music;
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

                Radiance::Window* aboutDlg = nullptr, * registrationDlg = nullptr;
                Radiance::Panel* loginPanel = nullptr, * loggingInPanel = nullptr, * menuPanel = nullptr;
                Radiance::Label* statusText = nullptr;
                Radiance::Image* activityIndicator = nullptr, * watermark = nullptr;
            }
            ui;

        private:
            TitleScene( const TitleScene& );
            const TitleScene& operator = ( const TitleScene& );

        protected:
            void messageBox( const Vector<>& size, const String& title, const String& text );

        public:
            TitleScene( IGraphicsDriver* driver, const Vector2<unsigned>& windowSize, TitleScenePreloader* preloader );
            virtual ~TitleScene();

            void init() override;
            void uninit() override {}

            void onKeyState( int16_t key, Key::State state, Unicode::Char character ) override;
            void onLoginSessionStatus();
            void onMouseMoveTo( const Vector2<int>& mouse ) override;
            void onRadianceEvent( Radiance::Widget* widget, const String& eventName, void* eventProperties ) override;

            void onRender() override;
            void onUpdate( double delta ) override;

            void sessionError( const String& errorName );
            void showMainMenu();
    };
}

