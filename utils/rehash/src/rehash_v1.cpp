
extern "C"
{
#include "md5.h"
}

#define littl_GZFileStream

#include <dirent.h>
#include <littl.hpp>
#include <windows.h>

using namespace li;

static const unsigned buffer_size = 65536;

List<const char*> bases, optionals;

static void hash( File& file, OutputStream* output, const String& name, unsigned flags )
{
    output->write<uint16_t>( flags );
    output->writeString( name );

    if ( flags & 1 )
    {
        printf( "`%s` --\n", name.c_str() );
        return;
    }

    md5_t state;

    md5_init( &state );

    while ( file.isReadable() )
    {
        uint8_t buffer[buffer_size];

        unsigned read = file.read( buffer, buffer_size );
        md5_process( &state, buffer, read );
    }

    uint8_t signature[16];

    md5_finish( &state, signature );

    char str[33];
    md5_sig_to_string( signature, str, sizeof( str ) );

    printf( "`%s` : %s\n", name.c_str(), str );

    output->write( signature, 16 );
}

static bool is_listed( const String& name, unsigned& flags )
{
    iterate ( bases )
        if ( name == bases.current() )
            return true;

    iterate ( optionals )
        if ( name == optionals.current() )
        {
            flags |= 1;
            return true;
        }

    return false;
}

static void rehash( OutputStream* output, const String& directory = "", unsigned flags = 0 )
{
    bool topLevel = directory.isEmpty();

    DIR* dir = opendir( topLevel ? "." : directory );

    if ( !dir )
        return;

    dirent* ent;

    while ( ( ent = readdir( dir ) ) )
    {
        if ( ent->d_name[0] == '.' )
            continue;

        String name;
        
        if ( topLevel )
        {
            flags = 0;

            if ( !is_listed( ent->d_name, flags ) )
                continue;

            name = ent->d_name;
        }
        else
            name = directory + "/" + ent->d_name;

        File file( name );

        if ( file.isReadable() )
            hash( file, output, name, flags );
        else
            rehash( output, name, flags );
    }

    closedir( dir );
}

int main( int argc, char** argv )
{
    String outputName;

    for ( int i = 1; i < argc; i++ )
    {
        if ( argv[i][0] == '+' )
            bases.add( argv[i] + 1 );
        else if ( argv[i][0] == '?' )
            optionals.add( argv[i] + 1 );
        else
            outputName = argv[i];
    }

    GZFileStream* output = new GZFileStream( outputName, "wb9" );
    output->write<uint16_t>( 0x0001 );
    //output->writeString( "tolcl-v0.1.13-r01276" );
    output->writeString( "tolcl" );
    rehash( output );
    output->release();
    
    return 0;
}
