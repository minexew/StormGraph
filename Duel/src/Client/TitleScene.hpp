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

#include <StormGraph/CommandLine.hpp>
#include <StormGraph/GraphicsDriver.hpp>
#include <StormGraph/GuiDriver.hpp>
#include <StormGraph/Scene.hpp>

#include <littl/TcpSocket.hpp>
#include <littl/Thread.hpp>

#include <Shared/Server.hpp>

namespace Duel
{
    using namespace StormGraph;

    struct Map;

    class ConnectingThread : public Mutex, public Thread
    {
        public:
            enum Status { connected, connecting, startingServer, loading, ready, ready2, error, exception };

        public:
            IEngine* engine;

            String host;
            int port;

            Object<IGameServer> server;
            Reference<TcpSocket> socket;
            Object<Map> map;

            Exception::Saved savedException;

        protected:
            virtual void changeStatus( Status status, float progress, const char* mapName = nullptr )
            {
                CriticalSection cs( this );

                this->status = status;
                this->progress = progress;
                this->mapName = mapName;

                statusChanged = true;
            }

            virtual void run() override;

        public:
            volatile bool statusChanged;
            Status status;
            float progress;
            String mapName;

            ConnectingThread( IEngine* engine, const String& address );
            //virtual ~ConnectingThread() {}
    };

    class TitleScene : public ICommandListener, public IGuiEventListener, public IScene
    {
        protected:
            IEngine* sg;
            IGraphicsDriver* graphicsDriver;

            Reference<IResourceManager> resMgr;

            Object<IGui> gui;
            Object<ICommandLine> console;
            Reference<IFont> bigFont, mediumFont;

            //Object<Camera> camera;
            //world

            struct ConnectingPvt
            {
                Object<ConnectingThread> thread;

                IPanel* panel;
                IStaticText* label;
                IProgressBar* progressBar;
            }
            connecting;

            void startConnecting();

        public:
            li_ReferencedClass_override( TitleScene )

            TitleScene( IEngine* sg );
            virtual ~TitleScene();

            virtual void init() override;
            virtual bool onCommand( const List<String>& tokens ) override;
            virtual void onGuiCommand( const CommandEvent& event ) override;
            virtual void onRender() override;
            virtual void onUpdate( double delta ) override;
            virtual void uninit() override;
    };
}
