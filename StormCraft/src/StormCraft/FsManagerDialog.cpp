#include "FsManagerDialog.hpp"

//(*InternalHeaders(FsManagerDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/filename.h>

#include <littl/File.hpp>

//(*IdInit(FsManagerDialog)
const long FsManagerDialog::ID_LISTBOX1 = wxNewId();
const long FsManagerDialog::ID_STATICTEXT1 = wxNewId();
const long FsManagerDialog::ID_CHOICE1 = wxNewId();
const long FsManagerDialog::ID_STATICTEXT2 = wxNewId();
const long FsManagerDialog::ID_TEXTCTRL1 = wxNewId();
const long FsManagerDialog::ID_BUTTON1 = wxNewId();
const long FsManagerDialog::ID_CHECKBOX1 = wxNewId();
const long FsManagerDialog::ID_BUTTON2 = wxNewId();
const long FsManagerDialog::ID_BUTTON6 = wxNewId();
const long FsManagerDialog::ID_BUTTON3 = wxNewId();
const long FsManagerDialog::ID_STATICLINE1 = wxNewId();
const long FsManagerDialog::ID_BUTTON4 = wxNewId();
const long FsManagerDialog::ID_BUTTON5 = wxNewId();
//*)

BEGIN_EVENT_TABLE(FsManagerDialog,wxDialog)
	//(*EventTable(FsManagerDialog)
	//*)
END_EVENT_TABLE()

extern IEngine* sg;
extern IGraphicsDriver* driver;

static String absoluteToRelative( const String& absolutePath, const String& relativeTo )
{
    // Based on a horrible code from
    // http://mrpmorris.blogspot.com/2007/05/convert-absolute-path-to-relative-path.html

    List<String> absoluteDirs, relativeDirs;

    absolutePath.replaceAll( '\\', '/' ).parse( absoluteDirs, '/' );
    relativeTo.replaceAll( '\\', '/' ).parse( relativeDirs, '/' );

    size_t length = minimum( absoluteDirs.getLength(), relativeDirs.getLength() );

    int lastCommonRoot = -1;

    for ( size_t index = 0; index < length; index++ )
        if ( absoluteDirs[index] == relativeDirs[index] )
            lastCommonRoot = index;
        else
            break;

    if ( lastCommonRoot < 0 )
        return absolutePath;

    String relativePath;

    for ( size_t index = lastCommonRoot + 1; index < relativeDirs.getLength(); index++ )
        //if ( !relativeDirs[index].isEmpty() )
        relativePath += "/..";

    for ( size_t index = lastCommonRoot + 1; index < absoluteDirs.getLength(); index++ )
        relativePath += "/" + absoluteDirs[index];

    return relativePath.dropLeftPart( 1 );
}

FsManagerDialog::FsManagerDialog( World* world, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size )
        : world( world )
{
	//(*Initialize(FsManagerDialog)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, _("Manage FileSystems"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	SetClientSize(wxSize(550,345));
	FlexGridSizer1 = new wxFlexGridSizer(8, 1, 0, 0);
	fsList = new wxListBox(this, ID_LISTBOX1, wxDefaultPosition, wxSize(455,170), 0, 0, 0, wxDefaultValidator, _T("ID_LISTBOX1"));
	FlexGridSizer1->Add(fsList, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(2, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Protocol:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	fsDriverList = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer2->Add(fsDriverList, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Path:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(1, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	pathEdit = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer3->Add(pathEdit, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Button1_ = new wxButton(this, ID_BUTTON1, _("Browse..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer3->Add(Button1_, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer3, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	cloneCheckBox = new wxCheckBox(this, ID_CHECKBOX1, _("Clone in map package"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	cloneCheckBox->SetValue(false);
	FlexGridSizer1->Add(cloneCheckBox, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 6, 0, 0);
	Button2_ = new wxButton(this, ID_BUTTON2, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer4->Add(Button2_, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button6 = new wxButton(this, ID_BUTTON6, _("Edit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
	FlexGridSizer4->Add(Button6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button3_ = new wxButton(this, ID_BUTTON3, _("Remove"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer4->Add(Button3_, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticLine1 = new wxStaticLine(this, ID_STATICLINE1, wxDefaultPosition, wxSize(-1,15), wxLI_VERTICAL, _T("ID_STATICLINE1"));
	FlexGridSizer4->Add(StaticLine1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button4_ = new wxButton(this, ID_BUTTON4, _("OK"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer4->Add(Button4_, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Button5_ = new wxButton(this, ID_BUTTON5, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
	FlexGridSizer4->Add(Button5_, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_LISTBOX1,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&FsManagerDialog::onFsListSelect);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FsManagerDialog::onBrowse);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FsManagerDialog::onAdd);
	Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FsManagerDialog::onEdit);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FsManagerDialog::onRemove);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FsManagerDialog::onOk);
	Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FsManagerDialog::onCancel);
	//*)

    List<IEngine::RegisteredFsDriver> drivers;

	sg->listFileSystemDrivers( drivers );

	iterate ( drivers )
	{
	    DriverInfo driver;

	    driver.protocol = drivers.current().protocol;
	    drivers.current().driver->getDriverInfo( &driver.info );

	    fsDriverList->Append( ( driver.protocol + " (" + driver.info.description + ")" ).c_str() );
	    this->drivers.add( driver );
	}

    iterate ( world->fileSystems )
        fsList->Append( ( const char* ) world->fileSystems.current().name );
}

FsManagerDialog::~FsManagerDialog()
{
	//(*Destroy(FsManagerDialog)
	//*)
}

void FsManagerDialog::onAdd( wxCommandEvent& event )
{
    int driverIndex = fsDriverList->GetSelection();

    if ( driverIndex >= 0 )
    {
        String entry = ( String ) drivers[driverIndex].protocol + ":" + ( const char* ) pathEdit->GetValue();

        fsList->Append( entry.c_str() );
    }
}

void FsManagerDialog::onBrowse( wxCommandEvent& event )
{
    int driverIndex = fsDriverList->GetSelection();

    if ( driverIndex < 0 )
        return;

    String path;

    if ( String::equals( drivers[driverIndex].protocol, "native" ) )
    {
        wxDirDialog dialog( this, "Filesystem base directory:", pathEdit->GetValue() );

        if ( dialog.ShowModal() != wxID_OK )
            return;

        path = absoluteToRelative( ( const char* ) dialog.GetPath(), File::getDirectoryFromPath( world->fileName ) );

        if ( !path.endsWith( '/' ) && !path.endsWith( '\\' ) )
            path += '/';
    }
    else
    {
        wxFileName fileName( pathEdit->GetValue() );

        wxFileDialog dialog( this, "Select a Moxillan package:", fileName.GetPath( wxPATH_GET_VOLUME ), fileName.GetFullName(),
                "Moxillan packages (*.mox)|*.mox|All files|*.*" );

        if ( dialog.ShowModal() != wxID_OK )
            return;

        path = absoluteToRelative( ( const char* ) dialog.GetPath(), File::getDirectoryFromPath( world->fileName ) ) + "/";
    }

    pathEdit->SetValue( ( const char* ) path );
}

void FsManagerDialog::onCancel( wxCommandEvent& event )
{
    EndModal( wxID_CANCEL );
}

void FsManagerDialog::onFsListSelect( wxCommandEvent& event )
{
    String protocol, path;

    path = ( const char* ) event.GetString();

    int colonPos = path.findChar( ':' );

    if ( colonPos <= 0 )
        protocol = "native";
    else
    {
        protocol = path.leftPart( colonPos );
        path = path.dropLeftPart( colonPos + 1 );
    }

    int driverIndex = -1;

    iterate ( drivers )
        if ( protocol == drivers.current().protocol )
        {
            driverIndex = drivers.iter();
            break;
        }

    fsDriverList->SetSelection( driverIndex );
    pathEdit->SetValue( path.c_str() );
}

void FsManagerDialog::onOk( wxCommandEvent& event )
{
    world->fileSystems.clear();

    for ( size_t i = 0; i < fsList->GetCount(); i++ )
        world->fileSystems.add( World::Fs { ( const char* ) fsList->GetString( i ), cloneCheckBox->GetValue() } );

    EndModal( wxID_OK );
}

void FsManagerDialog::onRemove( wxCommandEvent& event )
{
    int selection = fsList->GetSelection();

    if ( selection >= 0 )
        fsList->Delete( selection );
}

void FsManagerDialog::onEdit(wxCommandEvent& event)
{
    int driverIndex = fsDriverList->GetSelection();
    int selection = fsList->GetSelection();

    if ( driverIndex >= 0 && selection >= 0 )
    {
        String entry = ( String ) drivers[driverIndex].protocol + ":" + ( const char* ) pathEdit->GetValue();

        fsList->Delete( selection );
        fsList->Insert( entry.c_str(), selection );
    }
}
