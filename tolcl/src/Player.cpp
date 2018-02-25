
#include "Player.hpp"

namespace GameClient
{
    Player::Player( unsigned pid, const String& name, const Vector<float>& loc, float angle )
            : pid( pid ), name( name ), loc( loc ), angle( angle )
    {
        model = globalResMgr->getModel( "tolcl/model/human_0_sny.ms3d" );
        sword = globalResMgr->getModel( "tolcl/model/sword_0_sny.ms3d" );
        nameTexture = globalResMgr->getNamedFont( "ui_big" )->render( name, Colour( 1.0f, 1.0f, 1.0f ) );
    }

    Player::~Player()
    {
        model->release();
    }

    bool Player::changeModel( const String& name )
    {
        Model* newModel = globalResMgr->getModel( "tolcl/model/" + name + ".ms3d", true, false );

        if ( newModel )
        {
            model->release();
            model = newModel;
            return true;
        }
        else
            return false;
    }

    void Player::render()
    {
        model->renderBegin();
        model->translate( loc );
        model->rotate( angle, Vector<float>( 0.0f, 0.0f, 1.0f ) );
        model->render();
        model->renderEnd();

        sword->renderBegin();
        sword->translate( loc );
        sword->rotate( angle, Vector<float>( 0.0f, 0.0f, 1.0f ) );          // rotate with player
        sword->translate( Vector<float>( 0.2f, 0.15f, 0.6f ) );             // position to player
        sword->rotate( M_PI / 5.0f, Vector<float>( 0.0f, 1.0f, 0.0f ) );    // roll
        sword->rotate( M_PI / 3.0f, Vector<float>( 1.0f, 0.0f, 0.0f ) );    // yaw
        sword->render();
        sword->renderEnd();
    }

    void Player::renderName()
    {
        if ( nameTexture )
            nameTexture->renderBillboard2( loc + Vector<float>( 0.0f, 0.0f, 2.5f ), nameTexture->getWidth() / 200.0f, nameTexture->getHeight() / 200.0f );
    }
}
