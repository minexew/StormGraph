
#include <StormGraph/StormGraph.hpp>

namespace StormGraph
{
    using namespace Helium;

    HeliumScene::HeliumScene( HeVM* vm, Variable object ) : vm( vm )
    {
        closeButtonEvent = object.cloneMember( "closeButtonAction" );
        keyStateChangeEvent = object.cloneMember( "keyStateChange" );
        mouseButtonEvent = object.cloneMember( "mouseButton" );
        mouseMoveEvent = object.cloneMember( "mouseMove" );
        updateEvent = object.cloneMember( "update" );
        renderEvent = object.cloneMember( "render" );

        AutoVariable isLoaded = object.cloneMember( "isLoaded" );
        if ( !isLoaded.getBoolean() )
        {
            AutoVariable load = object.cloneMember( "load" );
            AutoVariable result = vm->invoke( load, arguments, true, false );

            if ( result.isNul() )
                object.setMember( "isLoaded", Variable( 1 ) );
            else if ( !result.getBoolean() )
                throw Exception( "StormGraph.HeliumScene", "HeliumScene", "SceneLoadFailed", "Scene loading function returned false. Check the appliaction log for details." );
        }
    }

    HeliumScene::~HeliumScene()
    {
    }
    
    bool HeliumScene::closeButtonAction()
    {
        AutoVariable result = vm->invoke( closeButtonEvent, arguments, true, false );
        return result.getBoolean();
    }

    void HeliumScene::keyStateChange( unsigned short key, bool pressed, Utf8Char character )
    {
        arguments.add( Variable( key ) );
        arguments.add( Variable( pressed ) );
        arguments.add( Variable::asInteger( character ) );
        AutoVariable result = vm->invoke( keyStateChangeEvent, arguments, true, false );
    }

    void HeliumScene::mouseButton( int x, int y, bool right, bool down )
    {
        arguments.add( Variable( x ) );
        arguments.add( Variable( y ) );
        arguments.add( Variable( right ) );
        arguments.add( Variable( down ) );
        AutoVariable result = vm->invoke( mouseButtonEvent, arguments, true, false );
    }

    void HeliumScene::mouseMove( int x, int y )
    {
        arguments.add( Variable( x ) );
        arguments.add( Variable( y ) );
        AutoVariable result = vm->invoke( mouseMoveEvent, arguments, true, false );
    }

    void HeliumScene::render()
    {
        AutoVariable result1 = vm->invoke( updateEvent, arguments, true, false );
        AutoVariable result = vm->invoke( renderEvent, arguments, true, false );
    }
}
