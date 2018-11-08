#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

//(*Headers(AboutDialog)
#include <wx/dialog.h>
#include <wx/html/htmlwin.h>
//*)

class AboutDialog: public wxDialog
{
	public:
		AboutDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize );
		virtual ~AboutDialog();

		//(*Declarations(AboutDialog)
		wxHtmlWindow* htmlWindow;
		//*)

	protected:
		//(*Identifiers(AboutDialog)
		static const long ID_HTMLWINDOW1;
		//*)

	private:

		//(*Handlers(AboutDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
