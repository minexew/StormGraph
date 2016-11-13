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

#include <Moxillan/Node.hpp>

#include <littl/Exception.hpp>
#include <littl/FileName.hpp>

namespace Moxillan
{
    NativeDirectoryNode::NativeDirectoryNode( const char* nativeDirectoryName, const char* directoryName, int compression, Directory* dir )
            : nativeDirectoryName( nativeDirectoryName ), compression( compression ), iterator( 0 )
    {
        this->directoryName = ( directoryName != nullptr ) ? ( String ) directoryName : FileName( nativeDirectoryName ).getFileName();

        Object<Directory> nativeDir = ( dir == nullptr ) ? Directory::open( nativeDirectoryName ) : dir;

        if ( nativeDir == nullptr )
            throw Exception( "Moxillan.NativeDirectoryNode.NativeDirectoryNode", "DirOpenError", "Failed to open directory " + FileName::format( nativeDirectoryName ) );

        List<String> items;
        nativeDir->list( items );

        iterate ( items )
        {
            if ( !items.current().beginsWith( '.' ) )
            {
                String itemName = ( String ) nativeDirectoryName + "/" + items.current();

                Directory* dir = Directory::open( itemName );

                if ( dir != nullptr )
                    nodes.add( new NativeDirectoryNode( itemName, nullptr, inherit, dir ) );
                else
                    nodes.add( new NativeFileNode( itemName ) );
            }
        }
    }

    NativeDirectoryNode::~NativeDirectoryNode()
    {
        iterate ( nodes )
            delete nodes.current();
    }

    int NativeDirectoryNode::getCompression()
    {
        return compression;
    }

    String NativeDirectoryNode::getName()
    {
        return directoryName;
    }

    size_t NativeDirectoryNode::iterableGetLength() const
    {
        return nodes.getLength();
    }

    INode* NativeDirectoryNode::iterableGetItem( size_t index )
    {
        return nodes[index];
    }
}
