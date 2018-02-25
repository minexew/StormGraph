
#include "GameClient.hpp"

namespace GameClient
{
    class Player
    {
        unsigned pid;
        String name;
        Vector<float> loc;
        float angle;

        Model* model, * sword;
        Texture* nameTexture;

        friend class GameScene;

        public:
            Player( unsigned pid, const String& name, const Vector<float>& loc, float angle );
            ~Player();

            bool changeModel( const String& name );
            void render();
            void renderName();
    };
}
