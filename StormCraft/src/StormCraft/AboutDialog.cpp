#include "AboutDialog.hpp"

#include <StormGraph/Engine.hpp>
#include <StormGraph/GraphicsDriver.hpp>

//(*InternalHeaders(AboutDialog)




//(*IdInit(AboutDialog)



BEGIN_EVENT_TABLE(AboutDialog, wxDialog)
	//(*EventTable(AboutDialog)
	//*)
END_EVENT_TABLE()

using namespace StormGraph;

extern IEngine* sg;
extern IGraphicsDriver* graphicsDriver;

AboutDialog::AboutDialog( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size )
{
	//(*Initialize(AboutDialog)






    IGraphicsDriver::Info driverInfo;

    graphicsDriver->getDriverInfo( &driverInfo );

    String text = "<center><img src=\"StormCraft/LocalUI/Logo.png\"></center><br />"
            "<font face=\"Segoe UI\" size=\"2\">"
            "<center><b>StormCraft TRUNK<br />"
            "Copyright &copy; 2011 Xeatheran Minexew</b></center><br /><br />"
            "<b>Engine:</b> " + sg->getEngineRelease() + "<br />"
            + "<b>Graphics Driver:</b> " + driverInfo.release + "<br />"
            + "<b>Renderer:</b> " + driverInfo.renderer + "<br />"
            + "</font>";
	htmlWindow->AppendToPage( text.c_str() );
}

AboutDialog::~AboutDialog()
{
	//(*Destroy(AboutDialog)
	//*)
}
