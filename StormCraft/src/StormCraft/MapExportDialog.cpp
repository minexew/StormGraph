#include "MapExportDialog.hpp"

//(*InternalHeaders(MapExportDialog)
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/filedlg.h>
#include <wx/filename.h>

//(*IdInit(MapExportDialog)
const long MapExportDialog::ID_TEXTCTRL1 = wxNewId();
const long MapExportDialog::ID_BUTTON1 = wxNewId();
const long MapExportDialog::ID_STATICLINE1 = wxNewId();
const long MapExportDialog::ID_STATICTEXT1 = wxNewId();
const long MapExportDialog::ID_CHECKBOX1 = wxNewId();
const long MapExportDialog::ID_CHECKBOX2 = wxNewId();
const long MapExportDialog::ID_STATICLINE2 = wxNewId();
const long MapExportDialog::ID_STATICTEXT2 = wxNewId();
const long MapExportDialog::ID_STATICTEXT3 = wxNewId();
const long MapExportDialog::ID_STATICTEXT4 = wxNewId();
const long MapExportDialog::ID_SPINCTRL1 = wxNewId();
const long MapExportDialog::ID_STATICTEXT5 = wxNewId();
const long MapExportDialog::ID_TEXTCTRL2 = wxNewId();
const long MapExportDialog::ID_STATICLINE3 = wxNewId();
const long MapExportDialog::ID_STATICTEXT7 = wxNewId();
const long MapExportDialog::ID_CHECKBOX4 = wxNewId();
const long MapExportDialog::ID_STATICTEXT9 = wxNewId();
const long MapExportDialog::ID_STATICTEXT8 = wxNewId();
const long MapExportDialog::ID_SPINCTRL2 = wxNewId();
const long MapExportDialog::ID_STATICLINE5 = wxNewId();
const long MapExportDialog::ID_STATICTEXT6 = wxNewId();
const long MapExportDialog::ID_CHECKBOX3 = wxNewId();
const long MapExportDialog::ID_STATICLINE4 = wxNewId();
const long MapExportDialog::ID_BUTTON2 = wxNewId();
const long MapExportDialog::ID_BUTTON3 = wxNewId();
//*)

BEGIN_EVENT_TABLE(MapExportDialog,wxDialog)
	//(*EventTable(MapExportDialog)
	//*)
END_EVENT_TABLE()

MapExportDialog::MapExportDialog( wxWindow* parent, World* world, wxWindowID id, const wxPoint& pos, const wxSize& size )
        : world( world )
{
	//(*Initialize(MapExportDialog)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("Export Map"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("id"));
	SetClientSize(wxSize(361,144));
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer2 = new wxFlexGridSizer(1, 3, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("Save as:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	pathEdit = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(306,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer2->Add(pathEdit, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	browseButton = new wxButton(this, ID_BUTTON1, _("Browse..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(browseButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticLine2 = new wxStaticLine(this, ID_STATICLINE1, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE1"));
	FlexGridSizer1->Add(StaticLine2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT1, _("General options"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	wxFont StaticText2Font(wxDEFAULT,wxDEFAULT,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StaticText2->SetFont(StaticText2Font);
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	includeGeometryCheckBox = new wxCheckBox(this, ID_CHECKBOX1, _("Include map geometry"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	includeGeometryCheckBox->SetValue(true);
	FlexGridSizer1->Add(includeGeometryCheckBox, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	cloneFsCheckBox = new wxCheckBox(this, ID_CHECKBOX2, _("Clone selected file systems"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	cloneFsCheckBox->SetValue(true);
	FlexGridSizer1->Add(cloneFsCheckBox, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticLine1 = new wxStaticLine(this, ID_STATICLINE2, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE2"));
	FlexGridSizer1->Add(StaticLine1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT2, _("BSP options"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	wxFont StaticText3Font(wxDEFAULT,wxDEFAULT,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StaticText3->SetFont(StaticText3Font);
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT3, _("Warning: Changes may affect rendering performance"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(1);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT4, _("Node polygon limit:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer4->Add(StaticText5, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	bspPolygonLimit = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxSize(150,21), 0, 1, 9001, 1, _T("ID_SPINCTRL1"));
	bspPolygonLimit->SetValue(_T("1"));
	FlexGridSizer4->Add(bspPolygonLimit, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT5, _("Node volume limit:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer4->Add(StaticText6, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	bspVolumeLimit = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer4->Add(bspVolumeLimit, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	StaticLine3 = new wxStaticLine(this, ID_STATICLINE3, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE3"));
	FlexGridSizer1->Add(StaticLine3, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT7, _("Ctree2 options"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	wxFont StaticText8Font(wxDEFAULT,wxDEFAULT,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StaticText8->SetFont(StaticText8Font);
	FlexGridSizer1->Add(StaticText8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	generateCtree2 = new wxCheckBox(this, ID_CHECKBOX4, _("Generate 2D collision tree"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
	generateCtree2->SetValue(false);
	FlexGridSizer1->Add(generateCtree2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText10 = new wxStaticText(this, ID_STATICTEXT9, _("Warning: Changes may affect collision detection performance"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer1->Add(StaticText10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT8, _("Node lineseg limit:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer5->Add(StaticText9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ctree2SegLimit = new wxSpinCtrl(this, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 32768, 1, _T("ID_SPINCTRL2"));
	ctree2SegLimit->SetValue(_T("1"));
	FlexGridSizer5->Add(ctree2SegLimit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticLine5 = new wxStaticLine(this, ID_STATICLINE5, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE5"));
	FlexGridSizer1->Add(StaticLine5, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT6, _("Lighting options"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	wxFont StaticText7Font(wxDEFAULT,wxDEFAULT,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StaticText7->SetFont(StaticText7Font);
	FlexGridSizer1->Add(StaticText7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	generateLightmapsCheckBox = new wxCheckBox(this, ID_CHECKBOX3, _("Generate light maps"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	generateLightmapsCheckBox->SetValue(false);
	FlexGridSizer1->Add(generateLightmapsCheckBox, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticLine4 = new wxStaticLine(this, ID_STATICLINE4, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE4"));
	FlexGridSizer1->Add(StaticLine4, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	ok = new wxButton(this, ID_BUTTON2, _("DO IT!"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer3->Add(ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	cancel = new wxButton(this, ID_BUTTON3, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer3->Add(cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MapExportDialog::onBrowse);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MapExportDialog::onOk);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MapExportDialog::onCancel);
	//*)

	pathEdit->SetValue( ( const char* ) world->exportSettings.fileName );
	includeGeometryCheckBox->SetValue( world->exportSettings.exportGeometry );
	cloneFsCheckBox->SetValue( world->exportSettings.cloneFileSystems );
	bspPolygonLimit->SetValue( world->exportSettings.bspPolygonLimit );
	bspVolumeLimit->SetValue( ( const char* ) world->exportSettings.bspVolumeLimit.toString() );
	generateCtree2->SetValue( world->exportSettings.generateCtree2 );
	ctree2SegLimit->SetValue( world->exportSettings.ctree2SegLimit );
}

MapExportDialog::~MapExportDialog()
{
	//(*Destroy(MapExportDialog)
	//*)
}

void MapExportDialog::onBrowse( wxCommandEvent& event )
{
    wxFileName fileName( pathEdit->GetValue() );

    wxFileDialog fileDlg( this, "Save package as...", fileName.GetPath( wxPATH_GET_VOLUME ), fileName.GetFullName(),
        "Moxillan packages (*.mox)|*.mox|All files|*.*", wxFD_SAVE );

    if ( fileDlg.ShowModal() != wxID_OK )
        return;

    pathEdit->SetValue( fileDlg.GetPath() );
}

void MapExportDialog::onCancel( wxCommandEvent& event )
{
    EndDialog( wxID_CANCEL );
}

void MapExportDialog::onOk( wxCommandEvent& event )
{
    if ( pathEdit->IsEmpty() )
        return;

    world->exportSettings.fileName = ( const char* ) pathEdit->GetValue();
	world->exportSettings.exportGeometry = includeGeometryCheckBox->GetValue();
	world->exportSettings.cloneFileSystems = cloneFsCheckBox->GetValue();
	world->exportSettings.bspPolygonLimit = bspPolygonLimit->GetValue();
    world->exportSettings.bspVolumeLimit = ( String ) ( const char* ) bspVolumeLimit->GetValue();
    world->exportSettings.generateCtree2 = generateCtree2->GetValue();
    world->exportSettings.ctree2SegLimit = ctree2SegLimit->GetValue();

    EndDialog( wxID_OK );
}
