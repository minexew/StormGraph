
extern "C"
{
#include "md5.h"
}

#define littl_GZFileStream

#include <littl.hpp>
#include <littl/cfx2.hpp>

#include <dirent.h>
#include <windows.h>

using namespace li;

static const unsigned buffer_size = 65536;

class Crawler
{
    String prefix;
    Reference<OutputStream> output;

    void entry( const String& name, uint8_t mode, InputStream* input );

    public:
        Crawler( const char* prefix, OutputStream* output );
        ~Crawler();

        void crawl( const String& name, uint8_t mode );
};

Crawler::Crawler( const char* prefix, OutputStream* output )
        : prefix( prefix ), output( output )
{
}

Crawler::~Crawler()
{
}

void Crawler::crawl( const String& name, uint8_t mode )
{
    DIR* dir = opendir( prefix + name );

    if ( !dir )
    {
        Reference<File> file = File::open( prefix + name );

        if ( isReadable( file ) )
            entry( name, mode, file );
        else
            Console::writeLine( "rehash: failed to open " + name );
    }
    else
    {
        dirent* ent;

        while ( ( ent = readdir( dir ) ) )
        {
            if ( ent->d_name[0] == '.' )
                continue;

            crawl( name + "/" + ent->d_name, mode );
        }

        closedir( dir );
    }
}

void Crawler::entry( const String& name, uint8_t mode, InputStream* input )
{
    if ( mode > 0 )
    {
        output->write<uint8_t>( mode );
        output->writeString( name );
    }

    if ( mode == 0x01 )
    {
        printf( "`%s` --\n", name.c_str() );
        return;
    }
    else if ( mode == 0x02 )
    {
        md5_t state;

        md5_init( &state );

        while ( isReadable( input ) )
        {
            uint8_t buffer[buffer_size];

            unsigned read = input->read( buffer, buffer_size );
            md5_process( &state, buffer, read );
        }

        uint8_t signature[16];

        md5_finish( &state, signature );

        char str[33];
        md5_sig_to_string( signature, str, sizeof( str ) );

        printf( "`%s` : %s\n", name.c_str(), str );

        output->write( signature, 16 );
    }
}

int main( int argc, char** argv )
{
    String inputName, prefix;

    for ( int i = 1; i < argc; i++ )
    {
        String arg = argv[i];

        if ( arg.beginsWith( "-p" ) )
            prefix = arg.dropLeft( 2 );
        else
            inputName = argv[i];
    }

    cfx2::Document doc( inputName );

    if ( !doc.isOk() )
    {
        fprintf( stderr, "%s\n", doc.getErrorDesc() );
        return errno;
    }

    cfx2::Node package = doc.findChild( "Package" );

    String outputName = prefix + package.getAttrib( "output" );
    Console::writeLine( "rehash: Building " + outputName );
    Reference<GZFileStream> output = new GZFileStream( outputName, "wb9" );

    // Common Header
    output->write<uint16_t>( 0x0002 );

    // Version 2 Header
    Object<cfx2::List> mirrors = doc.getList( "select Mirror" );
    Object<cfx2::List> roots = doc.getList( "select Exist, Ftp, Md5" );

    output->write<uint16_t>( mirrors->getLength() );
    output->write<int16_t>( -1 );

    // Mirror list
    for each_in_list_ptr ( mirrors, i )
        output->writeString( mirrors->get( i ).getText() );

    mirrors.release();

    // CRAWL!
    Crawler crawler( prefix, output->reference() );

    for each_in_list_ptr ( roots, i )
    {
        String type = roots->get( i ).getName();
        String name = roots->get( i ).getText();

        if ( type == "Exist" )
            crawler.crawl( name, 0x01 );
        else if ( type == "Md5" )
            crawler.crawl( name, 0x02 );
        else
            crawler.crawl( name, 0x00 );
    }

    roots.release();

    return 0;
}
