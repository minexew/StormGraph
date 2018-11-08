#include "MaterialManagerDialog.hpp"

//(*InternalHeaders(MaterialManagerDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(MaterialManagerDialog)
const long MaterialManagerDialog::ID_TREECTRL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(MaterialManagerDialog,wxDialog)
	//(*EventTable(MaterialManagerDialog)
	//*)
END_EVENT_TABLE()

MaterialManagerDialog::MaterialManagerDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(MaterialManagerDialog)
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;
	
	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxSize(517,394));
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	TreeCtrl1 = new wxTreeCtrl(this, ID_TREECTRL1, wxDefaultPosition, wxSize(219,302), wxTR_DEFAULT_STYLE, wxDefaultValidator, _T("ID_TREECTRL1"));
	FlexGridSizer1->Add(TreeCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->SetSizeHints(this);
	//*)
}

MaterialManagerDialog::~MaterialManagerDialog()
{
	//(*Destroy(MaterialManagerDialog)
	//*)
}

