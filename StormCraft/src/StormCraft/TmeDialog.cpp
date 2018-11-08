#include "TmeDialog.h"

//(*InternalHeaders(TmeDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(TmeDialog)
//*)

BEGIN_EVENT_TABLE(TmeDialog,wxDialog)
	//(*EventTable(TmeDialog)
	//*)
END_EVENT_TABLE()

TmeDialog::TmeDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(TmeDialog)
	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	//*)
}

TmeDialog::~TmeDialog()
{
	//(*Destroy(TmeDialog)
	//*)
}

