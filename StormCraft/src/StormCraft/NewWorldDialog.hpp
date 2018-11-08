#pragma once

#include <wx/defs.h>

//(*Headers(NewWorldDialog)
#include <wx/sizer.h>
#include <wx/radiobox.h>
#include <wx/dialog.h>
//*)

class NewWorldDialog: public wxDialog
{
	public:

		NewWorldDialog( wxWindow* parent, wxWindowID id = wxID_ANY );
		virtual ~NewWorldDialog();

		//(*Declarations(NewWorldDialog)
		wxRadioBox* radioBox;
		//*)

	protected:

		//(*Identifiers(NewWorldDialog)
		static const long ID_RADIOBOX1;
		//*)

	private:

		//(*Handlers(NewWorldDialog)
		void OnInit(wxInitDialogEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
