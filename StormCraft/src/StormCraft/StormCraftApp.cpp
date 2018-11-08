
#include "StormCraftApp.hpp"

//(*AppHeaders
#include "StormCraftMain.hpp"
#include <wx/image.h>
//*)

IEngine* sg;
IGraphicsDriver* graphicsDriver;

IMPLEMENT_APP( StormCraftApp );

StormCraftApp::~StormCraftApp()
{
    printf( "Destroying StormCraftApp\n" );
}

bool StormCraftApp::OnInit()
{
    // DAMN YOU, wxWidgets!
    setlocale( LC_NUMERIC, "C" );

    try
    {
        sg = Common::getCore( StormGraph_API_Version )->createEngine( "StormCraft", 0, nullptr );
        ::sg = sg;

        sg->addFileSystem( "mox:StormCraft.mox", false );
        sg->addFileSystem( "mox:System.mox", false );
        sg->addFileSystem( "native:" );

        sg->setVariable( "display.driver",                  sg->createStringVariable( "OpenGl" ),               true );
        sg->setVariable( "display.rendererOnly",            sg->createBoolVariable( true ),                     true );

        sg->startup();
        sg->startupGraphics();

        graphicsDriver = sg->getGraphicsDriver();
    }
    catch ( Exception& ex )
    {
        Common::displayException( ex, false );

        /*Engine::logEvent( "StormCraft", "Exception caught.\n\n"
                "<b>Function</b>: " + ex.functionName + "\n"
                "<b>Exception</b>: " + ex.getName() + "\n\n"
                "<b>Description</b>: " + ex.getDesc() + "\n\n" );*/

        return false;
    }

    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	StormCraftFrame* Frame = new StormCraftFrame( nullptr );
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)

    return true;
}
