#include "NewWorldDialog.hpp"

//(*InternalHeaders(NewWorldDialog)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(NewWorldDialog)
const long NewWorldDialog::ID_RADIOBOX1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(NewWorldDialog,wxDialog)
	//(*EventTable(NewWorldDialog)
	//*)
END_EVENT_TABLE()

NewWorldDialog::NewWorldDialog(wxWindow* parent,wxWindowID id)
{
	//(*Initialize(NewWorldDialog)
	wxFlexGridSizer* flexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, _("New World..."), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("id"));
	flexGridSizer1 = new wxFlexGridSizer(2, 1, 0, 0);
	flexGridSizer1->AddGrowableCol(0);
	flexGridSizer1->AddGrowableRow(0);
	wxString __wxRadioBoxChoices_1[1] =
	{
		_("StormCraft World Project")
	};
	radioBox = new wxRadioBox(this, ID_RADIOBOX1, _("Select world type:"), wxDefaultPosition, wxDefaultSize, 1, __wxRadioBoxChoices_1, 1, 0, wxDefaultValidator, _T("ID_RADIOBOX1"));
	flexGridSizer1->Add(radioBox, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	flexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(flexGridSizer1);
	flexGridSizer1->Fit(this);
	flexGridSizer1->SetSizeHints(this);

	Connect(wxID_ANY,wxEVT_INIT_DIALOG,(wxObjectEventFunction)&NewWorldDialog::OnInit);
	//*)
}

NewWorldDialog::~NewWorldDialog()
{
	//(*Destroy(NewWorldDialog)
	//*)
}


void NewWorldDialog::OnInit(wxInitDialogEvent& event)
{
}
