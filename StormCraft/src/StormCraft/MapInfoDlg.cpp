/*
    Copyright (c) 2011 Xeatheran Minexew

    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

#include "MapInfoDlg.hpp"

//(*InternalHeaders(MapInfoDlg)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(MapInfoDlg)
const long MapInfoDlg::ID_STATICTEXT1 = wxNewId();
const long MapInfoDlg::ID_TEXTCTRL1 = wxNewId();
const long MapInfoDlg::ID_BUTTON1 = wxNewId();
const long MapInfoDlg::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(MapInfoDlg,wxDialog)
	//(*EventTable(MapInfoDlg)
	//*)
END_EVENT_TABLE()

MapInfoDlg::MapInfoDlg( World* world, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size )
        : world( world )
{
	//(*Initialize(MapInfoDlg)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Map display name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	displayNameCtrl = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(displayNameCtrl, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button1_ = new wxButton(this, ID_BUTTON1, _("OK"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer1->Add(Button1_, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button2_ = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer1->Add(Button2_, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MapInfoDlg::onOk);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MapInfoDlg::onCancel);
	//*)

	String displayName = world->mapInfoDoc.queryValue( "Properties/displayName" );
	displayNameCtrl->SetValue( displayName.c_str() );
}

MapInfoDlg::~MapInfoDlg()
{
    //(*Destroy(MapInfoDlg)
	//*)
}

void MapInfoDlg::onOk( wxCommandEvent& event )
{
    world->mapInfoDoc.query( "Properties/displayName:" + ( String ) displayNameCtrl->GetValue().c_str() );

    Close();
}

void MapInfoDlg::onCancel( wxCommandEvent& event )
{
    Close();
}
