
#include "Messages.hpp"
#include "WorldScene.hpp"
#include "WorldSession.hpp"

static bool lost = false;

namespace TolClient
{
    WorldSession::WorldSession( TcpSocket* socket, WorldScene* game )
            : socket( socket ), game( game )
    {
    }

    WorldSession::~WorldSession()
    {
        if ( socket )
            socket->release();
    }

    void WorldSession::chat( const String& text )
    {
        if ( !socket )
            return;

        buffer.clear();
        buffer.write<uint16_t>( world::say );
        buffer.writeString( text );
        socket->send( buffer );
    }

    void WorldSession::movement( const Vector<float>& pos, float angle )
    {
        if ( !socket )
            return;

        buffer.clear();
        buffer.write<uint16_t>( world::player_movement );
        buffer.write<float>( pos.x );
        buffer.write<float>( pos.y );
        buffer.write<float>( pos.z );
        buffer.write<float>( angle );
        socket->send( buffer );
    }

    void WorldSession::process()
    {
        if ( !socket )
            return;

        if ( !socket->isWritable() )
        {
            if ( !lost )
                game->write( "\\r\\B\\S\\ Connection Lost." );

            lost = true;

            //socket->release();
            //socket = 0;

            return;
        }

        if ( socket->receive( buffer ) )
        {
            unsigned messageID = buffer.read<uint16_t>();

            switch ( messageID )
            {
                case world::welcome:
                {
                    playerPid = buffer.read<uint16_t>();
                    String name = buffer.readString();
                    String location = buffer.readString();
                    /*unsigned race = */buffer.read<uint16_t>();
                    /*unsigned classID = */buffer.read<uint16_t>();
                    /*unsigned level = */buffer.read<uint16_t>();
                    unsigned zoneID = buffer.read<uint16_t>();
                    float x = buffer.read<float>();
                    float y = buffer.read<float>();
                    float z = buffer.read<float>();
                    float orientation = buffer.read<float>();
                    unsigned goldAmount = buffer.read<uint32_t>();

                    printf( "Server Hello: %s, %s, %u, [%g %g %g %g], %u\n", name.c_str(), location.c_str(), zoneID, x, y, z, orientation, goldAmount );
                    game->characterInfo( playerPid, name, Vector<float>( x, y, z ), orientation );
                    break;
                }

                case world::chat_message:
                {
                    /*unsigned channel = */buffer.read<uint16_t>();
                    String from = buffer.readString();
                    String message = buffer.readString();

                    game->write( from + ": " + message );
                    break;
                }

                case world::player_left_area:
                    game->removePlayer( buffer.read<uint16_t>() );
                    break;

                case world::player_list:
                {
                    unsigned playersNear = buffer.read<uint16_t>();

                    for ( unsigned i = 0; i < playersNear; i++ )
                    {
                        unsigned pid = buffer.read<uint16_t>();
                        String name = buffer.readString();
                        /*unsigned race = */buffer.read<uint16_t>();
                        /*unsigned classID = */buffer.read<uint16_t>();
                        /*unsigned level = */buffer.read<uint16_t>();
                        float x = buffer.read<float>();
                        float y = buffer.read<float>();
                        float z = buffer.read<float>();
                        float orientation = buffer.read<float>();

                        game->newPlayer( pid, name, Vector<float>( x, y, z ), orientation );
                    }
                    break;
                }

                case world::player_location:
                {
                    unsigned pid = buffer.read<uint16_t>();
                    float x = buffer.read<float>();
                    float y = buffer.read<float>();
                    float z = buffer.read<float>();
                    float orientation = buffer.read<float>();

                    if ( playerPid == pid )
                        printf( "!!! WTF !!! received movement notify for OWN character! fix that Asap!!\n" );

                    game->playerLocation( pid, Vector<float>( x, y, z ), orientation );
                    break;
                }

                case world::player_status:
                {
                    unsigned pid = buffer.read<uint16_t>();
                    String name = buffer.readString();
                    unsigned status = buffer.read<uint16_t>();

                    game->playerStatus( pid, name, status );
                    break;
                }

                case world::remove_world_obj:
                {
                    float x = buffer.read<float>();
                    float y = buffer.read<float>();

                    game->removeWorldObj( x, y );
                    break;
                }

                case world::server_message:
                    game->write( "\\S\\o\\ SERVER MESSAGE:\\w " + buffer.readString() );
                    break;

                case world::spawn_world_obj:
                {
                    String name = buffer.readString();
                    float x = buffer.read<float>();
                    float y = buffer.read<float>();
                    float orientation = buffer.read<float>();

                    game->spawnWorldObj( name, x, y, orientation );
                    break;
                }

                case world::sync_rq:
                    buffer.clear();
                    buffer.write<uint16_t>( world::sync );
                    socket->send( buffer );
                    break;

                default:
                    printf( "WARNING: unknown message id %04X\n", messageID );
            }
        }
    }
}
