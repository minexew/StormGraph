
#include "GameScene.hpp"
#include "Scripting.hpp"

namespace GameClient
{
    ScriptErrorListener::ScriptErrorListener( GameScene* game ) : game( game )
    {
    }

    ScriptErrorListener::~ScriptErrorListener()
    {
    }

    void ScriptErrorListener::error( int line, const char* title, const char* desc )
    {
        game->write( ( String )"\\r" "line [" + line + "] " + title + ": " + desc );
        wasError = true;
    }

    Scripting::Scripting( GameScene* game )
    {
        class Bind : public misl::Callback
        {
            GameScene* game;

            public:
                Bind( GameScene* game ) : game( game )
                {
                }

                virtual ~Bind()
                {
                }

                virtual misl::Value call( misl::Environment* env, List<misl::Value>& args )
                {
                    game->bind( args[0].asString(), args[1].asString() );
                    return misl::Value();
                }
        };

        class Chat : public misl::Callback
        {
            GameScene* game;

            public:
                Chat( GameScene* game ) : game( game )
                {
                }

                virtual ~Chat()
                {
                }

                virtual misl::Value call( misl::Environment* env, List<misl::Value>& args )
                {
                    String text;

                    iterate ( args )
                        text += args.current().asString() + " ";

                    game->say( text );
                    return misl::Value();
                }
        };

        class Devmode : public misl::Callback
        {
            GameScene* game;

            public:
                Devmode( GameScene* game ) : game( game )
                {
                }

                virtual ~Devmode()
                {
                }

                virtual misl::Value call( misl::Environment* env, List<misl::Value>& args )
                {
                    game->setDevMode( args[0].asNumber() > 0 );
                    return misl::Value();
                }
        };

        class GetPlayerName : public misl::Callback
        {
            GameScene* game;

            public:
                GetPlayerName( GameScene* game ) : game( game )
                {
                }

                virtual ~GetPlayerName()
                {
                }

                virtual misl::Value call( misl::Environment* env, List<misl::Value>& args )
                {
                    return game->getPlayerName();
                }
        };

        class Say : public misl::Callback
        {
            GameScene* game;

            public:
                Say( GameScene* game ) : game( game )
                {
                }

                virtual ~Say()
                {
                }

                virtual misl::Value call( misl::Environment* env, List<misl::Value>& args )
                {
                    String text;

                    iterate ( args )
                        text += args.current().asString() + " ";

                    game->write( "\\o" + text );
                    return misl::Value();
                }
        };

        env = new misl::Environment();
        env->addCallback( "bind", new Bind( game ), "binds an action to the specified key" );
        env->addCallback( "chat", new Chat( game ), "broadcasts a message through the chat" );
        env->addCallback( "devmode", new Devmode( game ), "enable or disable developer mode" );
        env->addCallback( "getPlayerName", new GetPlayerName( game ), "returns the name of player's character" );
        env->addCallback( "say", new Say( game ), "prints the specified values to game console" );

        errorListener = new ScriptErrorListener( game );
    }

    Scripting::~Scripting()
    {
        delete env;
    }

    bool Scripting::execute( InputStream* input )
    {
        errorListener->clearErrors();

        misl::Node* script = misl::Compiler::compile( input, env, errorListener->reference() );

        if ( script )
        {
            script->execute( env );
            delete script;
        }

        return errorListener->wasSuccessful();
    }
}
