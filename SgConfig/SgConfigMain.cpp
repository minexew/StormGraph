/***************************************************************
 * Name:      SgConfigMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Xeatheran Minexew ()
 * Created:   2011-04-27
 * Copyright: Xeatheran Minexew ()
 * License:
 **************************************************************/

#include "SgConfigMain.h"
#include <wx/msgdlg.h>

//(*InternalHeaders(SgConfigDialog)
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/dir.h>

class AppNameData : public wxClientData
{
    public:
        wxString appName;

        AppNameData( const wxString& appName ) : appName( appName )
        {
        }

        virtual ~AppNameData()
        {
        }
};

//(*IdInit(SgConfigDialog)
const long SgConfigDialog::ID_STATICTEXT1 = wxNewId();
const long SgConfigDialog::ID_LISTBOX1 = wxNewId();
const long SgConfigDialog::ID_STATICTEXT12 = wxNewId();
const long SgConfigDialog::ID_STATICTEXT2 = wxNewId();
const long SgConfigDialog::ID_COMBO1 = wxNewId();
const long SgConfigDialog::ID_STATICTEXT15 = wxNewId();
const long SgConfigDialog::ID_TEXTCTRL3 = wxNewId();
const long SgConfigDialog::ID_BUTTON4 = wxNewId();
const long SgConfigDialog::ID_STATICTEXT3 = wxNewId();
const long SgConfigDialog::ID_TEXTCTRL1 = wxNewId();
const long SgConfigDialog::ID_STATICTEXT4 = wxNewId();
const long SgConfigDialog::ID_TEXTCTRL2 = wxNewId();
const long SgConfigDialog::ID_STATICTEXT5 = wxNewId();
const long SgConfigDialog::ID_STATICTEXT6 = wxNewId();
const long SgConfigDialog::ID_CHECKBOX1 = wxNewId();
const long SgConfigDialog::ID_STATICTEXT7 = wxNewId();
const long SgConfigDialog::ID_CHECKBOX2 = wxNewId();
const long SgConfigDialog::ID_STATICTEXT8 = wxNewId();
const long SgConfigDialog::ID_CHOICE2 = wxNewId();
const long SgConfigDialog::ID_STATICTEXT10 = wxNewId();
const long SgConfigDialog::ID_STATICTEXT9 = wxNewId();
const long SgConfigDialog::ID_CHOICE1 = wxNewId();
const long SgConfigDialog::ID_STATICTEXT11 = wxNewId();
const long SgConfigDialog::ID_STATICTEXT14 = wxNewId();
const long SgConfigDialog::ID_STATICTEXT13 = wxNewId();
const long SgConfigDialog::ID_CHECKBOX3 = wxNewId();
const long SgConfigDialog::ID_BUTTON2 = wxNewId();
const long SgConfigDialog::ID_BUTTON1 = wxNewId();
const long SgConfigDialog::ID_BUTTON3 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SgConfigDialog,wxDialog)
    //(*EventTable(SgConfigDialog)
    //*)
END_EVENT_TABLE()

SgConfigDialog::SgConfigDialog(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(SgConfigDialog)
    wxFlexGridSizer* BoxSizer1;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer5;
    wxBoxSizer* BoxSizer2;
    
    Create(parent, id, _("SgConfig"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxTAB_TRAVERSAL, _T("id"));
    BoxSizer1 = new wxFlexGridSizer(1, 2, 0, 0);
    BoxSizer1->AddGrowableCol(0);
    BoxSizer1->AddGrowableRow(0);
    FlexGridSizer1 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(1);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Please choose an application from the list below:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    appList = new wxListBox(this, ID_LISTBOX1, wxDefaultPosition, wxSize(263,169), 0, 0, 0, wxDefaultValidator, _T("ID_LISTBOX1"));
    wxFont appListFont(10,wxSWISS,wxFONTSTYLE_NORMAL,wxBOLD,false,_T("Segoe UI"),wxFONTENCODING_DEFAULT);
    appList->SetFont(appListFont);
    FlexGridSizer1->Add(appList, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer1->Add(FlexGridSizer1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer2 = new wxFlexGridSizer(11, 2, 0, 0);
    StaticText12 = new wxStaticText(this, ID_STATICTEXT12, _("Graphics settings"), wxDefaultPosition, wxSize(139,13), 0, _T("ID_STATICTEXT12"));
    wxFont StaticText12Font(wxDEFAULT,wxDEFAULT,wxFONTSTYLE_NORMAL,wxBOLD,true,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText12->SetFont(StaticText12Font);
    FlexGridSizer2->Add(StaticText12, 1, wxALL|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Graphics driver:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    driverList = new wxComboBox(this, ID_COMBO1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_COMBO1"));
    driverList->Disable();
    FlexGridSizer2->Add(driverList, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText14 = new wxStaticText(this, ID_STATICTEXT15, _("Driver parameters:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT15"));
    FlexGridSizer2->Add(StaticText14, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer5->AddGrowableCol(0);
    driverParams = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
    driverParams->Disable();
    FlexGridSizer5->Add(driverParams, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    driverParamsInfo = new wxButton(this, ID_BUTTON4, _("\?"), wxDefaultPosition, wxSize(24,23), 0, wxDefaultValidator, _T("ID_BUTTON4"));
    driverParamsInfo->Disable();
    FlexGridSizer5->Add(driverParamsInfo, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer5, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Display resolution:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 4, 0, 0);
    displayXEdit = new wxSpinCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(60,21), 0, 0, 32767, 0, _T("ID_TEXTCTRL1"));
    displayXEdit->Disable();
    FlexGridSizer3->Add(displayXEdit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("x"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer3->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    displayYEdit = new wxSpinCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxSize(60,21), 0, 0, 32767, 0, _T("ID_TEXTCTRL2"));
    displayYEdit->Disable();
    FlexGridSizer3->Add(displayYEdit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("pixels"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer3->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Fullscreen:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer2->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    fullscreenCheckBox = new wxCheckBox(this, ID_CHECKBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    fullscreenCheckBox->SetValue(false);
    fullscreenCheckBox->Disable();
    FlexGridSizer2->Add(fullscreenCheckBox, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Vertical sync:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer2->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    vsyncCheckBox = new wxCheckBox(this, ID_CHECKBOX2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
    vsyncCheckBox->SetValue(false);
    vsyncCheckBox->Disable();
    FlexGridSizer2->Add(vsyncCheckBox, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Anti-Aliasing:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    FlexGridSizer2->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    aaChoice = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    aaChoice->Append(_("disabled"));
    aaChoice->Append(_("2x"));
    aaChoice->Append(_("4x"));
    aaChoice->Append(_("8x"));
    aaChoice->Disable();
    FlexGridSizer2->Add(aaChoice, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText10 = new wxStaticText(this, ID_STATICTEXT10, _("Advanced settings"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
    wxFont StaticText10Font(wxDEFAULT,wxDEFAULT,wxFONTSTYLE_NORMAL,wxBOLD,true,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText10->SetFont(StaticText10Font);
    FlexGridSizer2->Add(StaticText10, 1, wxALL|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
    FlexGridSizer2->Add(20,20,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText9 = new wxStaticText(this, ID_STATICTEXT9, _("Texture level of detail:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    FlexGridSizer2->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    textureLodChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    textureLodChoice->Append(_("Level 0 (Full Resolution)"));
    textureLodChoice->Append(_("Level 1"));
    textureLodChoice->Append(_("Level 2 (Low Resolution)"));
    textureLodChoice->Disable();
    FlexGridSizer2->Add(textureLodChoice, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText11 = new wxStaticText(this, ID_STATICTEXT11, _("Developer options"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
    wxFont StaticText11Font(wxDEFAULT,wxDEFAULT,wxFONTSTYLE_NORMAL,wxBOLD,true,wxEmptyString,wxFONTENCODING_DEFAULT);
    StaticText11->SetFont(StaticText11Font);
    FlexGridSizer2->Add(StaticText11, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    developerOptionsStatus = new wxStaticText(this, ID_STATICTEXT14, _("unknown"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT14"));
    wxFont developerOptionsStatusFont(wxDEFAULT,wxDEFAULT,wxFONTSTYLE_ITALIC,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    developerOptionsStatus->SetFont(developerOptionsStatusFont);
    FlexGridSizer2->Add(developerOptionsStatus, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText13 = new wxStaticText(this, ID_STATICTEXT13, _("Show console:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT13"));
    FlexGridSizer2->Add(StaticText13, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    showConsoleCheckBox = new wxCheckBox(this, ID_CHECKBOX3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
    showConsoleCheckBox->SetValue(false);
    showConsoleCheckBox->Disable();
    FlexGridSizer2->Add(showConsoleCheckBox, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    defaultsButton = new wxButton(this, ID_BUTTON2, _("Defaults"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    defaultsButton->Disable();
    BoxSizer2->Add(defaultsButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    saveButton = new wxButton(this, ID_BUTTON1, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    saveButton->Disable();
    BoxSizer2->Add(saveButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    runButton = new wxButton(this, ID_BUTTON3, _("Save && Launch!"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    runButton->Disable();
    wxFont runButtonFont(wxDEFAULT,wxDEFAULT,wxFONTSTYLE_NORMAL,wxBOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    runButton->SetFont(runButtonFont);
    BoxSizer2->Add(runButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(BoxSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer1->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(BoxSizer1);
    BoxSizer1->Fit(this);
    BoxSizer1->SetSizeHints(this);
    
    Connect(ID_LISTBOX1,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&SgConfigDialog::onAppListSelect);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SgConfigDialog::onDriverParamsInfoClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SgConfigDialog::onDefaultsButtonClick);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SgConfigDialog::onSaveButtonClick);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SgConfigDialog::onRunButtonClick);
    Connect(wxID_ANY,wxEVT_INIT_DIALOG,(wxObjectEventFunction)&SgConfigDialog::OnInit);
    //*)
}

SgConfigDialog::~SgConfigDialog()
{
    //(*Destroy(SgConfigDialog)
    //*)
}

void SgConfigDialog::OnQuit(wxCommandEvent& event)
{
    Close();
}

void SgConfigDialog::OnInit(wxInitDialogEvent& event)
{
    loadDriverList();

    wxDir dir( wxGetCwd() );

    if ( !dir.IsOpened() )
    {
        //setError();
        return;
    }

    wxString filename;
    bool cont = dir.GetFirst( &filename, "*.cfx2" );

    while ( cont )
    {
        cfx2_Node* doc = cfx2_load_document( filename );
        cfx2_Node* appNode = cfx2_find_child( doc, "StormGraphApp" );

        if ( appNode && appNode->text )
            appList->Append( appNode->text, new AppNameData( filename ) );

        cfx2_release_node( doc );

        cont = dir.GetNext( &filename );
    }
}

void SgConfigDialog::onAppListSelect(wxCommandEvent& event)
{
    AppNameData* appNameData = ( AppNameData* ) event.GetClientObject();
    currentApp = appNameData->appName;

    loadSettings( false );

    driverList->Enable();
    driverParams->Enable();
    driverParamsInfo->Enable();

    displayXEdit->Enable();
    displayYEdit->Enable();

    fullscreenCheckBox->Enable();
    vsyncCheckBox->Enable();

    aaChoice->Enable();
    textureLodChoice->Enable();

    defaultsButton->Enable();
    saveButton->Enable();
    runButton->Enable( !this->executable.IsEmpty() );

    if ( devMode )
        developerOptionsStatus->SetLabel( "available" );
    else
        developerOptionsStatus->SetLabel( "not available" );

    showConsoleCheckBox->Enable( devMode );
}

void SgConfigDialog::onDefaultsButtonClick(wxCommandEvent& event)
{
    loadSettings( true );
}

int SgConfigDialog::getInt( cfx2_Node* node, const char* name, bool defaults )
{
    cfx2_Node* child = cfx2_query_node( node, name, 0 );

    if ( child && !defaults && child->text )
        return strtol( child->text, 0, 0 );

    if ( child && defaults )
    {
        long value;

        if ( !cfx2_get_node_attrib_int( child, "default", &value ) )
            return value;
    }

    return 0;
}

const char* SgConfigDialog::getString( cfx2_Node* node, const char* name, bool defaults )
{
    cfx2_Node* child = cfx2_query_node( node, name, 0 );

    if ( child && !defaults && child->text )
        return child->text;

    if ( child && defaults )
    {
        const char* value;

        if ( !cfx2_get_node_attrib( child, "default", &value ) )
            return value;
    }

    return "";
}

void SgConfigDialog::loadDriverList()
{
    cfx2_Node* doc = cfx2_load_document( "bin/Drivers.cfx2" );

    if ( !doc )
        return;

    if ( doc->children )
        for ( unsigned i = 0; i < doc->children->length; i++ )
            driverList->Append( cfx2_item( doc->children, i, cfx2_Node* )->name );

    cfx2_release_node( doc );
}

void SgConfigDialog::loadSettings( bool defaults )
{
    cfx2_Node* doc = cfx2_load_document( currentApp );

    if ( !defaults )
    {
        cfx2_Attrib* executable = cfx2_query_attrib( doc, "StormGraphApp.executable", 0 );

        if ( executable && executable->value )
            this->executable = executable->value;
        else
            this->executable.Clear();

        cfx2_Attrib* devMode = cfx2_query_attrib( doc, "StormGraphApp.devMode", 0 );

        if ( devMode && devMode->value )
            this->devMode = strtoul( devMode->value, 0, 0 ) != 0;
        else
            this->devMode = false;
    }

    driverList->SetValue( getString( doc, "StormGraph/driver", defaults ) );
    driverParams->SetValue( getString( doc, "StormGraph/driverParams", defaults ) );
    displayXEdit->SetValue( getInt( doc, "StormGraph/width", defaults ) );
    displayYEdit->SetValue( getInt( doc, "StormGraph/height", defaults ) );

    fullscreenCheckBox->SetValue( getInt( doc, "StormGraph/fullscreen", defaults ) );
    vsyncCheckBox->SetValue( getInt( doc, "StormGraph/vsync", defaults ) );

    switch ( getInt( doc, "StormGraph/multisample", defaults ) )
    {
        case 2: aaChoice->SetSelection( 1 ); break;
        case 4: aaChoice->SetSelection( 2 ); break;
        case 8: aaChoice->SetSelection( 3 ); break;
        default: aaChoice->SetSelection( 0 );
    }

    textureLodChoice->SetSelection( wxClip( getInt( doc, "StormGraph/textureLod", defaults ), 0, ( int )( textureLodChoice->GetCount() - 1 ) ) );

    showConsoleCheckBox->SetValue( devMode && getInt( doc, "Dev/showConsole", defaults ) );

    cfx2_release_node( doc );
}

void SgConfigDialog::onSaveButtonClick(wxCommandEvent& event)
{
    cfx2_Node* doc = cfx2_load_document( currentApp );

    cfx2_query( doc, "StormGraph/driver:" + driverList->GetValue(), true, 0 );
    cfx2_query( doc, "StormGraph/driverParams:" + driverParams->GetValue(), true, 0 );
    cfx2_query( doc, ( wxString ) "StormGraph/width:" << displayXEdit->GetValue(), true, 0 );
    cfx2_query( doc, ( wxString ) "StormGraph/height:" << displayYEdit->GetValue(), true, 0 );

    cfx2_query( doc, ( wxString ) "StormGraph/fullscreen:" << fullscreenCheckBox->GetValue(), true, 0 );
    cfx2_query( doc, ( wxString ) "StormGraph/vsync:" << vsyncCheckBox->GetValue(), true, 0 );

    switch ( aaChoice->GetSelection() )
    {
        case 1: cfx2_query( doc, "StormGraph/multisample:2", true, 0 ); break;
        case 2: cfx2_query( doc, "StormGraph/multisample:4", true, 0 ); break;
        case 3: cfx2_query( doc, "StormGraph/multisample:8", true, 0 ); break;
        default: cfx2_query( doc, "StormGraph/multisample:0", true, 0 );
    }

    cfx2_query( doc, ( wxString ) "StormGraph/textureLod:" << textureLodChoice->GetSelection(), true, 0 );

    if ( devMode )
        cfx2_query( doc, ( wxString ) "Dev/showConsole:" << showConsoleCheckBox->GetValue(), true, 0 );

    cfx2_save_document( doc, currentApp );
    cfx2_release_node( doc );
}

void SgConfigDialog::onRunButtonClick(wxCommandEvent& event)
{
    onSaveButtonClick( event );

    wxExecute( executable );
    Close();
}

void SgConfigDialog::onDriverParamsInfoClick(wxCommandEvent& event)
{
    wxMessageBox( getDriverParamsInfo( driverList->GetValue() ) );
}

wxString SgConfigDialog::getDriverParamsInfo( const char* driverName )
{
    cfx2_Node* doc = cfx2_load_document( "bin/Drivers.cfx2" );

    if ( !doc )
        return "Error: Couldn't open 'bin/Drivers.cfx2'";

    cfx2_Node* driverNode = cfx2_find_child( doc, driverName );

    if ( !driverNode )
    {
        cfx2_release_node( doc );
        return "Error: Unable to detect driver";
    }

    wxString message;

    if ( !driverNode->children || driverNode->children->length == 0 )
        message = "No configurable parameters are available for this driver.";
    else
    {
        message = "The following parameters (comma-separated) are available for this driver:\n\n";

        for ( unsigned i = 0; i < driverNode->children->length; i++ )
        {
            cfx2_Node* child = cfx2_item( driverNode->children, i, cfx2_Node* );

            message << child->name << " -- " << child->text << "\n";

            const char* example;

            if ( cfx2_get_node_attrib( child, "example", &example ) == cfx2_ok )
                message << "    example: " << example << "\n";

            message << "\n";
        }
    }

    cfx2_release_node( doc );
    return message;
}
