#ifndef MATERIALMANAGERDIALOG_H
#define MATERIALMANAGERDIALOG_H

//(*Headers(MaterialManagerDialog)
#include <wx/treectrl.h>
#include <wx/sizer.h>
#include <wx/dialog.h>
//*)

class MaterialManagerDialog: public wxDialog
{
	public:

		MaterialManagerDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~MaterialManagerDialog();

		//(*Declarations(MaterialManagerDialog)
		wxTreeCtrl* TreeCtrl1;
		//*)

	protected:

		//(*Identifiers(MaterialManagerDialog)
		static const long ID_TREECTRL1;
		//*)

	private:

		//(*Handlers(MaterialManagerDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
