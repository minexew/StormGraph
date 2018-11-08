#include "AboutDialog.hpp"

#include <StormGraph/Engine.hpp>
#include <StormGraph/GraphicsDriver.hpp>

//(*InternalHeaders(AboutDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(AboutDialog)
const long AboutDialog::ID_HTMLWINDOW1 = wxNewId();
//*)

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
	Create(parent, id, _("About StormCraft..."), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("id"));
	SetClientSize(wxSize(500,300));
	Move(wxDefaultPosition);
	htmlWindow = new wxHtmlWindow(this, ID_HTMLWINDOW1, wxPoint(0,0), wxSize(500,300), wxHW_SCROLLBAR_AUTO, _T("ID_HTMLWINDOW1"));
	//*)

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

