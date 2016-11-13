
#include <Moxillan/Package.hpp>
#include <Moxillan/PackageBuilder.hpp>

#include <littl.hpp>
#include <littl/Main.hpp>

using namespace li;

namespace Moxillan
{
    static void main( const char* app, const List<String>& args )
    {
        if ( args[0] == "build" )
        {
            Object<Moxillan::IDirectoryNode> rootDir;

            int compression = 0;

            for ( unsigned i = 2; i < args.getLength(); i++ )
            {
                if ( args[i].beginsWith( '!' ) )
                    compression = args[i].dropLeftPart( 1 );
                else if ( args[i].beginsWith( '@' ) )
                    rootDir = new Moxillan::NativeDirectoryNode( args[i].dropLeftPart( 1 ), nullptr, compression );
            }

            Moxillan::PackageBuilder::buildPackage( rootDir, File::open( args[1], true ), 0 );

            //uint64_t written = pkg->build();
            //printf( "%llu bytes written.\n", written );
        }
        /*else if ( args[0] == "extract" )
        {
            // Extract one or more objects from an existing package

            Moxillan::Package* pkg = new Moxillan::Package( open( args[1], false ) );

            for ( unsigned i = 2; i < args.getLength(); i++ )
            {
                Moxillan::Node* object = pkg->getTopNode()->find( args[i] );

                if ( object )
                    pkg->extractNode( object );
                else
                    printf( "Moxillan: object `%s` not found in package `%s`!\n", args[i].c_str(), args[1].c_str() );
            }

            delete pkg;
        }
        else if ( args[0] == "extract-all" )
        {
            // Extract the whole package

            Moxillan::Package* pkg = new Moxillan::Package( open( args[1], false ) );

            if ( pkg->getTopNode() )
                pkg->getTopNode()->print();

            pkg->extract( args[2] );
            delete pkg;
        }
        else if ( args[0] == "list" )
        {
            // List package contents

            Moxillan::Package* pkg = new Moxillan::Package( open( args[1], false ) );

            if ( pkg->getTopNode() )
                pkg->getTopNode()->print();

            delete pkg;
        }*/
    }
}

li_main( Moxillan::main )
