#pragma once

#include <wx/defs.h>

//(*Headers(NewWorldDialog)





class NewWorldDialog: public wxDialog
{
	public:

		NewWorldDialog( wxWindow* parent, wxWindowID id = wxID_ANY );
		virtual ~NewWorldDialog();

		//(*Declarations(NewWorldDialog)



	protected:

		//(*Identifiers(NewWorldDialog)



	private:

		//(*Handlers(NewWorldDialog)
		void OnInit(wxInitDialogEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};