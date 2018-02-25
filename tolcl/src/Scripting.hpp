#pragma once

#include "GameClient.hpp"

#include <misl.hpp>

namespace GameClient
{
    class GameScene;

    class ScriptErrorListener : public misl::ErrorListener
    {
        GameScene* game;
        bool wasError;

        public:
            ScriptErrorListener( GameScene* game );
            virtual ~ScriptErrorListener();
            void clearErrors() { wasError = false; }
            virtual void error( int line, const char* title, const char* desc );
            bool wasSuccessful() const { return !wasError; }
    };

    class Scripting
    {
        misl::Environment* env;
        ScriptErrorListener* errorListener;

        public:
            Scripting( GameScene* game );
            ~Scripting();

            bool execute( InputStream* input );
    };
}
