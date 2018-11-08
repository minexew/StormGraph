
#include "StormCraftMain.hpp"

//(*InternalHeaders(StormCraftFrame)
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/textdlg.h>

#include "AboutDialog.hpp"
#include "FsManagerDialog.hpp"
#include "MapExportDialog.hpp"
#include "MapInfoDlg.hpp"
#include "NewWorldDialog.hpp"
#include "TmeFrame.hpp"
#include "World.hpp"

#include <StormGraph/IO/ImageWriter.hpp>

#include <Moxillan/PackageBuilder.hpp>

#include <littl/File.hpp>

//(*IdInit(StormCraftFrame)
const long StormCraftFrame::ID_GLCANVAS1 = wxNewId();
const long StormCraftFrame::ID_TREECTRL1 = wxNewId();
const long StormCraftFrame::ID_PANEL1 = wxNewId();
const long StormCraftFrame::ID_AUINOTEBOOK1 = wxNewId();
const long StormCraftFrame::ID_TREECTRL2 = wxNewId();
const long StormCraftFrame::ID_PANEL2 = wxNewId();
const long StormCraftFrame::ID_AUINOTEBOOK2 = wxNewId();
const long StormCraftFrame::idMenuNew = wxNewId();
const long StormCraftFrame::ID_MENUITEM4 = wxNewId();
const long StormCraftFrame::ID_MENUITEM5 = wxNewId();
const long StormCraftFrame::idMenuQuit = wxNewId();
const long StormCraftFrame::ID_MENUITEM8 = wxNewId();
const long StormCraftFrame::ID_MENUITEM12 = wxNewId();
const long StormCraftFrame::ID_MENUITEM14 = wxNewId();
const long StormCraftFrame::ID_MENUITEM13 = wxNewId();
const long StormCraftFrame::ID_MENUITEM7 = wxNewId();
const long StormCraftFrame::idMenuAbout = wxNewId();
const long StormCraftFrame::ID_STATUSBAR1 = wxNewId();
const long StormCraftFrame::ID_TOOLBARITEM1 = wxNewId();
const long StormCraftFrame::ID_TOOLBARITEM3 = wxNewId();
const long StormCraftFrame::ID_TOOLBARITEM4 = wxNewId();
const long StormCraftFrame::ID_TOOLBARITEM2 = wxNewId();
const long StormCraftFrame::ID_TOOLBARITEM6 = wxNewId();
const long StormCraftFrame::ID_TOOLBARITEM5 = wxNewId();
const long StormCraftFrame::ID_TOOLBAR1 = wxNewId();
const long StormCraftFrame::ID_MENUITEM2 = wxNewId();
const long StormCraftFrame::ID_MENUITEM6 = wxNewId();
const long StormCraftFrame::ID_MENUITEM9 = wxNewId();
const long StormCraftFrame::ID_MENUITEM1 = wxNewId();
const long StormCraftFrame::ID_MENUITEM11 = wxNewId();
const long StormCraftFrame::ID_MENUITEM10 = wxNewId();
const long StormCraftFrame::ID_MENUITEM3 = wxNewId();
//*)

BEGIN_EVENT_TABLE( StormCraftFrame, wxFrame )
    //(*EventTable(StormCraftFrame)
    //*)
END_EVENT_TABLE()

StormCraftFrame* StormCraftFrame::instance;

LightMapAreaDialogAdapter::LightMapAreaDialogAdapter( StormCraftFrame* frame, World* world, Resources* res, WorldNode* editNode, const char* initValue )
        : frame( frame ), world( world ), res( res ), editNode( editNode ), initValue( initValue )
{
    printf( "new LightMapAreaDialogAdapter\n" );
}

bool LightMapAreaDialogAdapter::DoShowDialog( wxPropertyGrid* propGrid, wxPGProperty* property )
{
    Object<TmeFrame> tme = new TmeFrame( world, res, frame->ctx, editNode, initValue, frame );
    tme->ShowModal();

    SetValue( ( const char* ) tme->getValue() );

    return true;
}

WX_PG_IMPLEMENT_PROPERTY_CLASS_PLAIN( LightMapAreaProperty, wxString, TextCtrlAndButton )

LightMapAreaProperty::LightMapAreaProperty( StormCraftFrame* frame, World* world, Resources* res, WorldNode* editNode,
        const wxString& label, const wxString& name, const wxString& value )
        : wxPGProperty( label, name ), frame( frame ), world( world ), res( res ), editNode( editNode )
{
    SetValue( value );
}

LightMapAreaProperty::~LightMapAreaProperty()
{
}

wxString LightMapAreaProperty::ValueToString( wxVariant& value, int argFlags ) const
{
    return value.GetString();
}

wxPGEditorDialogAdapter* LightMapAreaProperty::GetEditorDialog() const
{
    return new LightMapAreaDialogAdapter( frame, world, res, editNode, GetValue().GetString() );
}

bool LightMapAreaProperty::StringToValue( wxVariant& variant, const wxString& text, int argFlags ) const
{
    if ( variant.GetString() == text )
        return false;
    else
    {
        variant = text;
        return true;
    }
}

static void writeGraphNode( OutputStream* output, WorldNode* node, bool noHdr = false )
{
    LightWorldNode* light = dynamic_cast<LightWorldNode*>( node );

    if ( light != nullptr && light->enabled )
    {
        if ( light->type == 0 )
        {
            output->write<uint8_t>( 2 );
            output->write<Vector<float>>( light->position );
            output->write<Vector<float>>( light->direction );
            output->write<uint32_t>( light->ambient.toRgbaUint32() );
            output->write<uint32_t>( light->diffuse.toRgbaUint32() );
            output->write<uint32_t>( light->diffuse.toRgbaUint32() );
            output->write<float>( light->range );

            output->write<float>( light->fov );
            output->write<uint8_t>( light->cubeShadowMapping ? 1 : 0 );
            output->write<uint32_t>( light->shadowMapDetail );
        }
        else if ( light->type == 1 )
        {
            output->write<uint8_t>( 1 );
            output->write<Vector<float>>( light->direction );
            output->write<uint32_t>( light->ambient.toRgbaUint32() );
            output->write<uint32_t>( light->diffuse.toRgbaUint32() );
            output->write<uint32_t>( light->diffuse.toRgbaUint32() );
        }
    }

    GroupWorldNode* group = dynamic_cast<GroupWorldNode*>( node );

    if ( group != nullptr )
    {
        if ( !noHdr )
            output->write<uint8_t>( 10 );

        iterate2 ( child, group->children )
            writeGraphNode( output, child );

        output->write<uint8_t>( 0 );
    }
}

StormCraftFrame::StormCraftFrame( wxWindow* parent, wxWindowID id ) : multipleViews( false ), lmbDown(false), mmbDown(false), rmbDown(false),
        selectedItemData( nullptr ), dragItemData( nullptr )
{
    instance = this;

    //(*Initialize(StormCraftFrame)
    wxMenuItem* MenuItem2;
    wxMenuItem* MenuItem1;
    wxBoxSizer* BoxSizer2;
    wxMenu* Menu1;
    wxMenuItem* MenuItem3;
    wxBoxSizer* BoxSizer1;
    wxMenuBar* MenuBar1;
    wxMenu* Menu2;

    Create(parent, wxID_ANY, _("StormCraft World Editor"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    SetClientSize(wxSize(1129,538));
    {
    	wxIcon FrameIcon;
    	FrameIcon.CopyFromBitmap(wxBitmap(wxImage(_T("StormCraft/LocalUI/16x16/apps/internet-web-browser.png"))));
    	SetIcon(FrameIcon);
    }
    auiManager = new wxAuiManager(this, wxAUI_MGR_DEFAULT);
    int GLCanvasAttributes_1[] = {
    	WX_GL_RGBA,
    	WX_GL_DOUBLEBUFFER,
    	WX_GL_DEPTH_SIZE,      16,
    	WX_GL_STENCIL_SIZE,    0,
    	0, 0 };
    canvas = new wxGLCanvas(this, ID_GLCANVAS1, wxPoint(324,156), wxDefaultSize, 0, _T("ID_GLCANVAS1"), GLCanvasAttributes_1);
    auiManager->AddPane(canvas, wxAuiPaneInfo().Name(_T("canvasPane")).CenterPane().Caption(_("Main View")).Floatable().Movable(false));
    AuiNotebook1 = new wxAuiNotebook(this, ID_AUINOTEBOOK1, wxPoint(9,301), wxDefaultSize, wxAUI_NB_DEFAULT_STYLE);
    AuiNotebook1->SetMinSize(wxSize(220,50));
    Panel1 = new wxPanel(AuiNotebook1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    worldExplorerTree = new wxTreeCtrl(Panel1, ID_TREECTRL1, wxDefaultPosition, wxDefaultSize, wxTR_EDIT_LABELS|wxTR_FULL_ROW_HIGHLIGHT|wxTR_DEFAULT_STYLE, wxDefaultValidator, _T("ID_TREECTRL1"));
    BoxSizer1->Add(worldExplorerTree, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel1->SetSizer(BoxSizer1);
    BoxSizer1->Fit(Panel1);
    BoxSizer1->SetSizeHints(Panel1);
    AuiNotebook1->AddPage(Panel1, _("World Explorer"), true, wxBitmap(wxImage(_T("StormCraft/LocalUI/16x16/apps/internet-web-browser.png"))));
    auiManager->AddPane(AuiNotebook1, wxAuiPaneInfo().Name(_T("notebookPane")).DefaultPane().Caption(_("World")).PinButton().Left().MinSize(wxSize(220,50)));
    AuiNotebook2 = new wxAuiNotebook(this, ID_AUINOTEBOOK2, wxPoint(819,203), wxDefaultSize, wxAUI_NB_DEFAULT_STYLE);
    AuiNotebook2->SetMinSize(wxSize(200,0));
    Panel2 = new wxPanel(AuiNotebook2, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    TreeCtrl1 = new wxTreeCtrl(Panel2, ID_TREECTRL2, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE, wxDefaultValidator, _T("ID_TREECTRL2"));
    BoxSizer2->Add(TreeCtrl1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel2->SetSizer(BoxSizer2);
    BoxSizer2->Fit(Panel2);
    BoxSizer2->SetSizeHints(Panel2);
    AuiNotebook2->AddPage(Panel2, _("Materials"));
    auiManager->AddPane(AuiNotebook2, wxAuiPaneInfo().Name(_T("resourcesPane")).DefaultPane().Caption(_("Resources")).PinButton().Right().MinSize(wxSize(200,0)));
    auiManager->Update();
    MenuBar1 = new wxMenuBar();
    Menu1 = new wxMenu();
    MenuItem3 = new wxMenuItem(Menu1, idMenuNew, _("New world...\tCtrl-N"), _("Create a new world"), wxITEM_NORMAL);
    Menu1->Append(MenuItem3);
    MenuItem5 = new wxMenuItem(Menu1, ID_MENUITEM4, _("Open world...\tCtrl-O"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem5);
    MenuItem6 = new wxMenuItem(Menu1, ID_MENUITEM5, _("Save\tCtrl-S"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem6);
    MenuItem1 = new wxMenuItem(Menu1, idMenuQuit, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    Menu1->Append(MenuItem1);
    MenuBar1->Append(Menu1, _("&File"));
    Menu4 = new wxMenu();
    MenuItem7 = new wxMenuItem(Menu4, ID_MENUITEM8, _("Break into faces..."), wxEmptyString, wxITEM_NORMAL);
    Menu4->Append(MenuItem7);
    MenuItem11 = new wxMenuItem(Menu4, ID_MENUITEM12, _("Duplicate object"), wxEmptyString, wxITEM_NORMAL);
    Menu4->Append(MenuItem11);
    Menu4->AppendSeparator();
    MenuItem13 = new wxMenuItem(Menu4, ID_MENUITEM14, _("Rotate around Z..."), wxEmptyString, wxITEM_NORMAL);
    Menu4->Append(MenuItem13);
    Menu4->AppendSeparator();
    MenuItem12 = new wxMenuItem(Menu4, ID_MENUITEM13, _("Edit map info..."), wxEmptyString, wxITEM_NORMAL);
    Menu4->Append(MenuItem12);
    MenuBar1->Append(Menu4, _("Edit"));
    Menu3 = new wxMenu();
    MenuItem4 = new wxMenuItem(Menu3, ID_MENUITEM7, _("Create package..."), wxEmptyString, wxITEM_NORMAL);
    Menu3->Append(MenuItem4);
    MenuBar1->Append(Menu3, _("Tools"));
    Menu2 = new wxMenu();
    MenuItem2 = new wxMenuItem(Menu2, idMenuAbout, _("About\tF1"), _("Show info about this application"), wxITEM_NORMAL);
    Menu2->Append(MenuItem2);
    MenuBar1->Append(Menu2, _("Help"));
    SetMenuBar(MenuBar1);
    statusBar = new wxStatusBar(this, ID_STATUSBAR1, 0, _T("ID_STATUSBAR1"));
    int __wxStatusBarWidths_1[1] = { -1 };
    int __wxStatusBarStyles_1[1] = { wxSB_NORMAL };
    statusBar->SetFieldsCount(1,__wxStatusBarWidths_1);
    statusBar->SetStatusStyles(1,__wxStatusBarStyles_1);
    SetStatusBar(statusBar);
    toolBar = new wxToolBar(this, ID_TOOLBAR1, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL|wxNO_BORDER, _T("ID_TOOLBAR1"));
    ToolBarItem1 = toolBar->AddTool(ID_TOOLBARITEM1, _("New world..."), wxBitmap(wxImage(_T("StormCraft/LocalUI/16x16/actions/document-new.png"))), wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString);
    ToolBarItem2 = toolBar->AddTool(ID_TOOLBARITEM3, _("Open world..."), wxBitmap(wxImage(_T("StormCraft/LocalUI/16x16/actions/document-open.png"))), wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString);
    ToolBarItem3 = toolBar->AddTool(ID_TOOLBARITEM4, _("Save"), wxBitmap(wxImage(_T("StormCraft/LocalUI/16x16/actions/document-save.png"))), wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString);
    toolBar->AddSeparator();
    ToolBarItem4 = toolBar->AddTool(ID_TOOLBARITEM2, _("Configure FileSystems..."), wxBitmap(wxImage(_T("StormCraft/LocalUI/16x16/mimetypes/package-x-generic.png"))), wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString);
    ToolBarItem5 = toolBar->AddTool(ID_TOOLBARITEM6, _("Light Mapping Editor"), wxBitmap(wxImage(_T("StormCraft/LocalUI/LightBulb_16.png"))), wxNullBitmap, wxITEM_NORMAL, wxEmptyString, wxEmptyString);
    toolBar->AddSeparator();
    ToolBarItem6 = toolBar->AddTool(ID_TOOLBARITEM5, _("Multiple Viewports"), wxBitmap(wxImage(_T("StormCraft/LocalUI/16x16/apps/preferences-system-windows.png"))), wxNullBitmap, wxITEM_CHECK, wxEmptyString, wxEmptyString);
    toolBar->Realize();
    SetToolBar(toolBar);
    addGroupMenuItem = new wxMenuItem((&groupWorldNodeItemMenu), ID_MENUITEM2, _("Add new Group"), wxEmptyString, wxITEM_NORMAL);
    addGroupMenuItem->SetBitmap(wxBitmap(wxImage(_T("StormCraft/LocalUI/16x16/status/folder-open.png"))));
    groupWorldNodeItemMenu.Append(addGroupMenuItem);
    addCuboidMenuItem = new wxMenuItem((&groupWorldNodeItemMenu), ID_MENUITEM6, _("Add new Cuboid"), wxEmptyString, wxITEM_NORMAL);
    addCuboidMenuItem->SetBitmap(wxBitmap(wxImage(_T("StormCraft/LocalUI/Cube_16.png"))));
    groupWorldNodeItemMenu.Append(addCuboidMenuItem);
    MenuItem8 = new wxMenuItem((&groupWorldNodeItemMenu), ID_MENUITEM9, _("Add new Face"), wxEmptyString, wxITEM_NORMAL);
    MenuItem8->SetBitmap(wxBitmap(wxImage(_T("StormCraft/LocalUI/Face_16.png"))));
    groupWorldNodeItemMenu.Append(MenuItem8);
    addLightMenuItem = new wxMenuItem((&groupWorldNodeItemMenu), ID_MENUITEM1, _("Add new Light"), wxEmptyString, wxITEM_NORMAL);
    addLightMenuItem->SetBitmap(wxBitmap(wxImage(_T("StormCraft/LocalUI/16x16/status/weather-clear.png"))));
    groupWorldNodeItemMenu.Append(addLightMenuItem);
    MenuItem10 = new wxMenuItem((&groupWorldNodeItemMenu), ID_MENUITEM11, _("Add new Polygon"), wxEmptyString, wxITEM_NORMAL);
    MenuItem10->SetBitmap(wxBitmap(wxImage(_T("StormCraft/LocalUI/Polygon_16.png"))));
    groupWorldNodeItemMenu.Append(MenuItem10);
    MenuItem9 = new wxMenuItem((&groupWorldNodeItemMenu), ID_MENUITEM10, _("Add new StaticMesh"), wxEmptyString, wxITEM_NORMAL);
    groupWorldNodeItemMenu.Append(MenuItem9);
    addTerrainMenuItem = new wxMenuItem((&groupWorldNodeItemMenu), ID_MENUITEM3, _("Add new Terrain"), wxEmptyString, wxITEM_NORMAL);
    addTerrainMenuItem->SetBitmap(wxBitmap(wxImage(_T("StormCraft/LocalUI/16x16/apps/internet-web-browser.png"))));
    groupWorldNodeItemMenu.Append(addTerrainMenuItem);

    canvas->Connect(wxEVT_PAINT,(wxObjectEventFunction)&StormCraftFrame::onCanvasPaint,0,this);
    canvas->Connect(wxEVT_LEFT_DOWN,(wxObjectEventFunction)&StormCraftFrame::onLmbDown,0,this);
    canvas->Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&StormCraftFrame::onLmbUp,0,this);
    canvas->Connect(wxEVT_MIDDLE_DOWN,(wxObjectEventFunction)&StormCraftFrame::onMmbDown,0,this);
    canvas->Connect(wxEVT_MIDDLE_UP,(wxObjectEventFunction)&StormCraftFrame::onMmbUp,0,this);
    canvas->Connect(wxEVT_RIGHT_DOWN,(wxObjectEventFunction)&StormCraftFrame::onRmbDown,0,this);
    canvas->Connect(wxEVT_RIGHT_UP,(wxObjectEventFunction)&StormCraftFrame::onRmbUp,0,this);
    canvas->Connect(wxEVT_MOTION,(wxObjectEventFunction)&StormCraftFrame::onMouseMove,0,this);
    canvas->Connect(wxEVT_ENTER_WINDOW,(wxObjectEventFunction)&StormCraftFrame::onCanvasMouseEnter,0,this);
    canvas->Connect(wxEVT_MOUSEWHEEL,(wxObjectEventFunction)&StormCraftFrame::onMouseWheel,0,this);
    Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_BEGIN_DRAG,(wxObjectEventFunction)&StormCraftFrame::onWorldExplorerTreeBeginDrag);
    Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_END_DRAG,(wxObjectEventFunction)&StormCraftFrame::onWorldExplorerTreeEndDrag);
    Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_END_LABEL_EDIT,(wxObjectEventFunction)&StormCraftFrame::onWorldExplorerTreeItemRenamed);
    Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_SEL_CHANGED,(wxObjectEventFunction)&StormCraftFrame::onWorldExplorerTreeSelectionChanged);
    Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_KEY_DOWN,(wxObjectEventFunction)&StormCraftFrame::onWorldExplorerTreeKeyDown);
    Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_ITEM_MENU,(wxObjectEventFunction)&StormCraftFrame::onWorldExplorerTreeItemMenu);
    Connect(idMenuNew,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&StormCraftFrame::onFileNew);
    Connect(ID_MENUITEM4,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&StormCraftFrame::onFileOpen);
    Connect(ID_MENUITEM5,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&StormCraftFrame::onFileSave);
    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&StormCraftFrame::OnQuit);
    Connect(ID_MENUITEM8,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&StormCraftFrame::onBreakIntoFaces);
    Connect(ID_MENUITEM12,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&StormCraftFrame::onDuplicateObject);
    Connect(ID_MENUITEM14,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&StormCraftFrame::onRotateZ);
    Connect(ID_MENUITEM13,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&StormCraftFrame::onEditMapInfo);
    Connect(ID_MENUITEM7,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&StormCraftFrame::onMapExport);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&StormCraftFrame::OnAbout);
    Connect(ID_TOOLBARITEM1,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&StormCraftFrame::onFileNew);
    Connect(ID_TOOLBARITEM3,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&StormCraftFrame::onFileOpen);
    Connect(ID_TOOLBARITEM4,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&StormCraftFrame::onFileSave);
    Connect(ID_TOOLBARITEM2,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&StormCraftFrame::onConfigureFilesystems);
    Connect(ID_TOOLBARITEM6,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&StormCraftFrame::onEditLightMapping);
    Connect(ID_TOOLBARITEM5,wxEVT_COMMAND_TOOL_CLICKED,(wxObjectEventFunction)&StormCraftFrame::onMultipleViewsToggle);
    Connect(ID_MENUITEM2,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&StormCraftFrame::onAddGroup);
    Connect(ID_MENUITEM6,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&StormCraftFrame::onAddCuboid);
    Connect(ID_MENUITEM9,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&StormCraftFrame::onAddFace);
    Connect(ID_MENUITEM1,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&StormCraftFrame::onAddLight);
    Connect(ID_MENUITEM11,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&StormCraftFrame::onAddPolygon);
    Connect(ID_MENUITEM10,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&StormCraftFrame::onAddStaticMesh);
    Connect(ID_MENUITEM3,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&StormCraftFrame::onAddTerrain);
    //*)

    propertyGrid = new wxPropertyGrid( this );
    propertyGrid->Connect( wxEVT_PG_CHANGED, ( wxObjectEventFunction ) &StormCraftFrame::onPropertyGridValueChanged, 0, this );
    auiManager->AddPane( propertyGrid, wxAuiPaneInfo().Name( _T( "propertyGridPane" ) ).DefaultPane().Caption( _( "Properties" ) ).PinButton().Left() );

    eventLog = new wxHtmlWindow( this );
    eventLog->AppendToPage( "<font face=\"Verdana\" size=2>" );
    auiManager->AddPane( eventLog, wxAuiPaneInfo().Name( _T( "eventLog" ) ).Caption( "Event Log" )
            .Float().FloatingPosition( GetPosition().x, GetPosition().y + GetSize().y ).FloatingSize( 600, 300 ).Hide().MinSize( 300, 100 ).Right() );

    auiManager->Update();

    ctx = new wxGLContext( canvas );
    ctx->SetCurrent( *canvas );

    try
    {
        Common::setLogEventCallback( logEvent );

        // Set Display Mode
        // Only initialize the structure to some meaningful values
        //  -- no SetVideoMode will happen anyway, as the driver is rendererOnly-mode

        DisplayMode displayMode;

        displayMode.width = 0;
        displayMode.height = 0;
        displayMode.windowTitle = nullptr;
        displayMode.fullscreen = false;
        displayMode.vsync = false;
        displayMode.multisamplingLevel = false;

        graphicsDriver->setDisplayMode( &displayMode );

        //LevelOfDetail lod = { 3 };
        //graphicsDriver->setLevelOfDetail( &lod );

        projectionBuffer = graphicsDriver->createProjectionInfoBuffer();

        loadResources();
    }
    catch ( Exception ex )
    {
        Common::displayException( ex, false );

        Close();
    }
}

StormCraftFrame::~StormCraftFrame()
{
    printf( "destroying StormCraftFrame\n" );

    Common::setLogEventCallback( nullptr );

    // Has to be done for some reason
    // (wxWidgets buggy much?)
    RemoveEventHandler( GetEventHandler() );

    //(*Destroy(StormCraftFrame)
    //*)
}

wxTreeItemId StormCraftFrame::addWorldExplorerTreeNode( WorldNodeTreeItemData* parent, WorldNode* node, bool select, bool editLabel )
{
    WorldNodeTreeItemData* itemData = new WorldNodeTreeItemData( parent, node );
    node->itemId = worldExplorerTree->AppendItem( parent != nullptr ? parent->GetId() : wxTreeItemId(), node->getName().c_str(), -1, -1, itemData );

    GroupWorldNode* group = dynamic_cast<GroupWorldNode*>( node );

    if ( group != nullptr )
        iterate ( group->children )
            addWorldExplorerTreeNode( itemData, group->children.current(), false, false );

    if ( select )
    {
        worldExplorerTree->Expand( node->itemId );
        worldExplorerTree->SelectItem( node->itemId );
    }

    if ( editLabel )
        worldExplorerTree->EditLabel( node->itemId );

    return node->itemId;
}

void StormCraftFrame::buildPropertyPane()
{
    WorldNode* node = selectedItemData->node;

    currentNodeProperties.clear();
    node->getProperties( currentNodeProperties );

    updateProperties( currentNodeProperties );
}

unsigned StormCraftFrame::getViewportAt( const Vector2<unsigned>& pos )
{
    if ( multipleViews )
    {
        unsigned x = pos.x / ( canvas->GetSize().x / 2 );
        unsigned y = pos.y / ( canvas->GetSize().y / 2 );

        return y * 2 + x;
    }
    else
        return 0;
}

void StormCraftFrame::loadResources()
{
    res = new Resources;
    res->resMgr = sg->createResourceManager( "StormCraft/resourceManager", sg->getFileSystem()->reference() );
    res->resMgr->addPath( "" );

    res->welcome.texture = res->resMgr->loadTexture( "StormCraft/UI/Welcome.png" );
    res->welcome.material = graphicsDriver->createSolidMaterial( "welcomeMaterial", Colour::white(), res->welcome.texture->reference() );

    CuboidCreationInfo bbox( Vector<float>( 1.0f, 1.0f, 1.0f ), Vector<float>( 0.5f, 0.5f, 0.5f ), false, false, true, graphicsDriver->getSolidMaterial() );
    res->boundingBox = graphicsDriver->createCuboid( "StormCraft/boundingBox", &bbox );

    res->lightbulb = res->resMgr->getModel( "StormCraft/Models/lightbulb.ms3d" );

    res->diamondMaterial = graphicsDriver->createSolidMaterial( "diamondMaterial", Colour( 0.33f, 0.66f, 1.0f, 0.8f ), nullptr );

    static const float diamondVertices[] = { 0.0f, 0.0f, -0.866f, 0.0f, 0.0f, 0.866f, -0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f };
    static const unsigned diamondIndices[] = { 0, 3, 2, 0, 4, 3, 0, 5, 4, 0, 2, 5, 1, 2, 3, 1, 3, 4, 1, 4, 5, 1, 5, 2 };

    MeshCreationInfo3 diamondMesh { MeshFormat::triangleList, MeshLayout::indexed, res->diamondMaterial->reference(),
            6, 24, diamondVertices, nullptr, { nullptr }, nullptr, diamondIndices };

    MeshCreationInfo3* diamondMeshPtr = &diamondMesh;
    res->diamond = graphicsDriver->createModelFromMemory( "diamond", &diamondMeshPtr, 1 );

    float pointBoxA = 0.015f;
    CuboidCreationInfo2 creationInfo( Vector<>(), Vector<>( pointBoxA, pointBoxA, pointBoxA ), Vector<>( pointBoxA / 2, pointBoxA / 2, pointBoxA / 2 ) );
    res->pointBox = graphicsDriver->createCuboid( "StormCraft.pointBox", creationInfo, graphicsDriver->createSolidMaterial( "StormCraft.solidMaterial", Colour::grey( 0.66f, 0.6f ), nullptr ) );

    static const float xArrow1Vertices[] = { 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f };
    static const float xArrow2Vertices[] = { 1.0f, 0.0f, 0.0f, 0.5f, -0.1f, 0.1f, 0.5f, 0.1f, 0.1f, 0.5f, 0.1f, -0.1f, 0.5f, -0.1f, -0.1f };
    static const float yArrow1Vertices[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f };
    static const float yArrow2Vertices[] = { 0.0f, 1.0f, 0.0f, -0.1f, 0.5f, 0.1f, 0.1f, 0.5f, 0.1f, 0.1f, 0.5f, -0.1f, -0.1f, 0.5f, -0.1f };
    static const float zArrow1Vertices[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f };
    static const float zArrow2Vertices[] = { 0.0f, 0.0f, 1.0f, -0.1f, 0.1f, 0.5f, 0.1f, 0.1f, 0.5f, 0.1f, -0.1f, 0.5f, -0.1f, -0.1f, 0.5f };
    static const unsigned arrowIndices[] = { 0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 1, 3, 2, 1, 1, 4, 3 };

    static const float xyPlaneVertices[] = { 0.0f, 0.0f, 0.0f, 0.3f, 0.0f, 0.0f, 0.0f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0.0f, 0.3f, 0.0f, 0.0f };
    static const float xzPlaneVertices[] = { 0.0f, 0.0f, 0.0f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.0f, 0.0f };
    static const float yzPlaneVertices[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0.0f, 0.0f, 0.0f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0.0f, 0.3f, 0.0f };

    Reference<IMaterial> redMaterial = graphicsDriver->createSolidMaterial( "redMaterial", Colour( 1.0f, 0.0f, 0.0f, 0.6f ), nullptr );
    Reference<IMaterial> greenMaterial = graphicsDriver->createSolidMaterial( "greenMaterial", Colour( 0.0f, 1.0f, 0.0f, 0.6f ), nullptr );
    Reference<IMaterial> blueMaterial = graphicsDriver->createSolidMaterial( "blueMaterial", Colour( 0.0f, 0.0f, 1.0f, 0.6f ), nullptr );

    Reference<IMaterial> rgMaterial = graphicsDriver->createSolidMaterial( "rgMaterial", Colour( 1.0f, 1.0f, 0.0f, 0.6f ), nullptr );
    Reference<IMaterial> rbMaterial = graphicsDriver->createSolidMaterial( "rbMaterial", Colour( 1.0f, 0.0f, 1.0f, 0.6f ), nullptr );
    Reference<IMaterial> gbMaterial = graphicsDriver->createSolidMaterial( "gbMaterial", Colour( 0.0f, 1.0f, 1.0f, 0.6f ), nullptr );

    MeshCreationInfo3 xArrow1 { MeshFormat::lineList, MeshLayout::linear, redMaterial->reference(), 2, 0, xArrow1Vertices, nullptr, {}, nullptr, nullptr };
    MeshCreationInfo3 xArrow2 { MeshFormat::triangleList, MeshLayout::indexed, redMaterial->reference(), 5, 18, xArrow2Vertices, nullptr, {}, nullptr, arrowIndices };
    MeshCreationInfo3 yArrow1 { MeshFormat::lineList, MeshLayout::linear, greenMaterial->reference(), 2, 0, yArrow1Vertices, nullptr, {}, nullptr, nullptr };
    MeshCreationInfo3 yArrow2 { MeshFormat::triangleList, MeshLayout::indexed, greenMaterial->reference(), 5, 18, yArrow2Vertices, nullptr, {}, nullptr, arrowIndices };
    MeshCreationInfo3 zArrow1 { MeshFormat::lineList, MeshLayout::linear, blueMaterial->reference(), 2, 0, zArrow1Vertices, nullptr, {}, nullptr, nullptr };
    MeshCreationInfo3 zArrow2 { MeshFormat::triangleList, MeshLayout::indexed, blueMaterial->reference(), 5, 18, zArrow2Vertices, nullptr, {}, nullptr, arrowIndices };

    MeshCreationInfo3 xyPlane { MeshFormat::triangleList, MeshLayout::linear, rgMaterial->reference(), 6, 0, xyPlaneVertices, nullptr, {}, nullptr, nullptr };
    MeshCreationInfo3 xzPlane { MeshFormat::triangleList, MeshLayout::linear, rbMaterial->reference(), 6, 0, xzPlaneVertices, nullptr, {}, nullptr, nullptr };
    MeshCreationInfo3 yzPlane { MeshFormat::triangleList, MeshLayout::linear, gbMaterial->reference(), 6, 0, yzPlaneVertices, nullptr, {}, nullptr, nullptr };

    List<MeshCreationInfo3*> gizmoMeshes[6];
    gizmoMeshes[0].add( &xyPlane );
    gizmoMeshes[1].add( &xzPlane );
    gizmoMeshes[2].add( &yzPlane );
    gizmoMeshes[3].add( &xArrow1 );
    gizmoMeshes[3].add( &xArrow2 );
    gizmoMeshes[4].add( &yArrow1 );
    gizmoMeshes[4].add( &yArrow2 );
    gizmoMeshes[5].add( &zArrow1 );
    gizmoMeshes[5].add( &zArrow2 );

    for ( size_t i = 0; i < lengthof( gizmoMeshes ); i++ )
        res->gizmo[i] = graphicsDriver->createModelFromMemory( "gizmo[]", gizmoMeshes[i].getPtr(), gizmoMeshes[i].getLength() );

    gizmoTransforms[0] = Transform( Transform::scale, Vector<>() );
    gizmoTransforms[1] = Transform( Transform::translate, Vector<>() );

    res->worldLight = graphicsDriver->createDirectionalLight( Vector<>( 0.0f, 0.0f, -1.0f ), Colour(), Colour::grey( 0.6f ), Colour::grey( 0.6f ) );

    res->font = res->resMgr->getFont( "Common/Fonts/DejaVuSans.ttf", 14, IFont::bold );
    res->gui = sg->getGuiDriver()->createGui( Vector<>(), Vector<>( canvas->GetSize().x, canvas->GetSize().y ) );
}

void StormCraftFrame::logEvent( const char* className, const char* event )
{
    instance->eventLog->AppendToPage( "<font color=blue><b>" );
    instance->eventLog->AppendToPage( className );
    instance->eventLog->AppendToPage( "</b></font>&nbsp;" );
    instance->eventLog->AppendToPage( ( const char* ) String( event ).replaceAll( "\n", "<br>" ) );
    instance->eventLog->AppendToPage( "<br>" );

    instance->auiManager->GetPane( instance->eventLog ).Show();
    instance->auiManager->Update();
}

Vector<> StormCraftFrame::getCameraScale() const
{
    return Vector<>( 1.0f, 1.0f, 1.0f ) * world->rootNode->camera[currentViewport].getDistance();
}

void StormCraftFrame::OnAbout( wxCommandEvent& event )
{
    AboutDialog( this ).ShowModal();

    //wxString msg = wxbuildinfo(long_f);
    //wxMessageBox(msg, _("Welcome to..."));
}

void StormCraftFrame::onAddFace( wxCommandEvent& event )
{
    GroupWorldNode* parent = static_cast<GroupWorldNode*>( selectedItemData->node );

    try
    {
        Object<WorldNode> node = new FaceWorldNode( world, res );
        node->startup();
        parent->add( node );

        addWorldExplorerTreeNode( selectedItemData, node.detach(), true, true );
    }
    catch ( Exception ex )
    {
        Common::displayException( ex, true );
    }
}

void StormCraftFrame::onAddGroup( wxCommandEvent& event )
{
    GroupWorldNode* parent = static_cast<GroupWorldNode*>( selectedItemData->node );

    WorldNode* node = new GroupWorldNode( world, res );
    parent->add( node );

    addWorldExplorerTreeNode( selectedItemData, node, true, true );
}

void StormCraftFrame::onAddLight( wxCommandEvent& event )
{
    GroupWorldNode* parent = static_cast<GroupWorldNode*>( selectedItemData->node );

    try
    {
        Object<WorldNode> node = new LightWorldNode( world, res );
        node->startup();
        parent->add( node );

        addWorldExplorerTreeNode( selectedItemData, node.detach(), true, true );
    }
    catch ( Exception ex )
    {
        Common::displayException( ex, true );
    }
}

void StormCraftFrame::onAddPolygon( wxCommandEvent& event )
{
    GroupWorldNode* parent = static_cast<GroupWorldNode*>( selectedItemData->node );

    try
    {
        Object<WorldNode> node = new PolygonWn( world, res );
        node->startup();
        parent->add( node );

        addWorldExplorerTreeNode( selectedItemData, node.detach(), true, true );
    }
    catch ( Exception ex )
    {
        Common::displayException( ex, true );
    }
}

void StormCraftFrame::onAddStaticMesh( wxCommandEvent& event )
{
    GroupWorldNode* parent = static_cast<GroupWorldNode*>( selectedItemData->node );

    try
    {
        Object<WorldNode> node = new StaticMeshWn( world, res );
        node->startup();
        parent->add( node );

        addWorldExplorerTreeNode( selectedItemData, node.detach(), true, true );
    }
    catch ( Exception ex )
    {
        Common::displayException( ex, true );
    }
}

void StormCraftFrame::onAddTerrain( wxCommandEvent& event )
{
    GroupWorldNode* parent = static_cast<GroupWorldNode*>( selectedItemData->node );

    try
    {
        Object<WorldNode> node = new TerrainWorldNode( world, res );
        node->startup();
        parent->add( node );

        addWorldExplorerTreeNode( selectedItemData, node.detach(), true, true );
    }
    catch ( Exception ex )
    {
        Common::displayException( ex, true );
    }
}

void StormCraftFrame::onAddCuboid( wxCommandEvent& event )
{
    GroupWorldNode* parent = static_cast<GroupWorldNode*>( selectedItemData->node );

    try
    {
        Object<WorldNode> node = new CuboidWorldNode( world, res );
        node->startup();
        parent->add( node );

        addWorldExplorerTreeNode( selectedItemData, node.detach(), true, true );
    }
    catch ( Exception ex )
    {
        Common::displayException( ex, true );
    }
}

void StormCraftFrame::onBreakIntoFaces( wxCommandEvent& event )
{
    if ( selectedItemData == nullptr )
        return;

    WorldNode* node = selectedItemData->node;

    try
    {
        size_t numFaces = 0;
        bool canDo = node->canBreakIntoFaces( numFaces );

        if ( !canDo )
        {
            wxMessageBox( "This operation is unavailable for the selected object.", "Break into faces", wxOK | wxICON_ERROR );
            return;
        }

        if ( wxMessageBox( ( const char* ) ( "The selected object will be broken into " + String::formatInt( numFaces ) + " faces.\n\nProceed?" ),
                "Break into faces", wxYES_NO | wxICON_QUESTION ) != wxYES )
            return;

        WorldNodeTreeItemData* parent = selectedItemData->parent;
        worldExplorerTree->Delete( selectedItemData->GetId() );
        selectedItemData = nullptr;

        List<PolygonWn*> faces;

        node->breakIntoFaces( faces );

        iterate2 ( face, faces )
        {
            face->startup();
            static_cast<GroupWorldNode*>( parent->node )->add( face );

            addWorldExplorerTreeNode( parent, face, false, false );
        }

        static_cast<GroupWorldNode*>( parent->node )->remove( node );
    }
    catch ( Exception ex )
    {
        Common::displayException( ex, true );
    }
}

void StormCraftFrame::onCanvasMouseEnter( wxMouseEvent& event )
{
    //canvas->SetFocus();
}

void StormCraftFrame::onCanvasPaint( wxPaintEvent& event )
{
    try
    {
        const Vector2<unsigned> canvasSize( canvas->GetSize().x, canvas->GetSize().y );

        ctx->SetCurrent( *canvas );
        IEventListener* listener = graphicsDriver->getEventListener();

        clock_t begin = clock();

        if ( world == nullptr )
        {
            graphicsDriver->setClearColour( Colour::white() );
            listener->onFrameBegin();

            graphicsDriver->setViewport( Vector2<int>(), canvasSize, canvasSize );

            Vector2<unsigned> dimensions = res->welcome.texture->getDimensions().getXy();

            graphicsDriver->set2dMode( 1.0f, -1.0f );
            graphicsDriver->drawRectangle( ( canvasSize - dimensions ) / 2, dimensions, Colour::white(), res->welcome.texture );
        }
        else
        {
            graphicsDriver->setClearColour( Colour() );
            listener->onFrameBegin();

            renderCanvas( false, Vector2<unsigned>() );
        }

        graphicsDriver->setRenderFlag( RenderFlag::culling, true );
        graphicsDriver->setRenderFlag( RenderFlag::wireframe, false );
        graphicsDriver->setViewport( Vector2<int>(), canvasSize, canvasSize );

        res->gui->onRender();

        clock_t end = clock();

        graphicsDriver->set2dMode( 1.0f, -1.0f );
        graphicsDriver->drawStats();
        res->font->renderString( 5, 5, "\\#789\\ rendered in " + String::formatFloat( ( end - begin ) * 1000.0f / CLOCKS_PER_SEC ) + " ms", Colour(), IFont::left | IFont::top );

        listener->onFrameEnd();

        canvas->SwapBuffers();
    }
    catch ( Exception ex )
    {
        Close();

        Common::displayException( ex, false );
    }
}

void StormCraftFrame::onConfigureFilesystems( wxCommandEvent& event )
{
    if ( world == nullptr )
        return;

    Object<FsManagerDialog> dlg = new FsManagerDialog( world, this );

    if ( dlg->ShowModal() != wxID_OK )
        return;

    rescanDirectories();
}

void StormCraftFrame::onDuplicateObject( wxCommandEvent& event )
{
    if ( selectedItemData == nullptr )
        return;

    WorldNode* node = selectedItemData->node;

    try
    {
        WorldNode* duplicate = node->clone();

        if ( duplicate != nullptr )
        {
            WorldNodeTreeItemData* parent = selectedItemData->parent;

            dynamic_cast<GroupWorldNode*>( parent->node )->add( duplicate );

            addWorldExplorerTreeNode( parent, duplicate, true, true );
        }
    }
    catch ( Exception ex )
    {
        Common::displayException( ex, true );
    }
}

void StormCraftFrame::onEditLightMapping( wxCommandEvent& event )
{
    if ( world == nullptr )
        return;

    Object<TmeFrame> tme = new TmeFrame( world, res, ctx, nullptr, String(), this );
    tme->ShowModal();
}

void StormCraftFrame::onEditMapInfo( wxCommandEvent& event )
{
    if ( world == nullptr )
        return;

    Object<MapInfoDlg> dialog = new MapInfoDlg( world, this );
    dialog->ShowModal();
}

void StormCraftFrame::onFileOpen( wxCommandEvent& event )
{
    wxFileDialog fileDlg( this, "Open world...", "", "", "StormCraft World files (*.world)|*.world|All files|*.*" );

    if ( fileDlg.ShowModal() != wxID_OK )
        return;

    try
    {
        // load the cfx2
        cfx2::Document doc( fileDlg.GetPath() );

        cfx2::Node root = doc.findChild( "Root" );

        if ( !root )
        {
            wxMessageBox( "The project file is corrupted or invalid.", "Error", wxOK | wxICON_ERROR );
            return;
        }

        // Allocate structures
        world = new World;
        world->gizmoObject = nullptr;
        world->fileName = fileDlg.GetPath();

        world->mapInfoDoc = doc.findChild( "MapInfo" ).clone();

        if ( !world->mapInfoDoc )
            world->mapInfoDoc.create( "MapInfo" );

        // File Systems
        Object<cfx2::List> fileSystems = doc.getList( "select FileSystem" );

        iterate ( *fileSystems )
            world->fileSystems.add( World::Fs { fileSystems->current().getText(), String::toInt( fileSystems->current().getAttrib( "cloneInPackage" ) ) > 0 } );

        // Map Export Settings
        world->exportSettings.fileName = doc.queryValue( "ExportSettings.fileName" );
        world->exportSettings.exportGeometry = String::toBool( doc.queryValue( "ExportSettings.exportGeometry" ) );
        world->exportSettings.cloneFileSystems = String::toBool( doc.queryValue( "ExportSettings.cloneFileSystems" ) );
        world->exportSettings.bspPolygonLimit = String::toInt( doc.queryValue( "ExportSettings.bspPolygonLimit" ) );
        world->exportSettings.bspVolumeLimit = Vector<float>( ( String ) doc.queryValue( "ExportSettings.bspVolumeLimit" ) );
        world->exportSettings.generateCtree2 = String::toBool( doc.queryValue( "ExportSettings.generateCtree2" ) );
        world->exportSettings.ctree2SegLimit = String::toInt( doc.queryValue( "ExportSettings.ctree2SegLimit" ) );

        // LightMaps
        Object<cfx2::List> lightMaps = doc.getList( "select LightMap" );

        iterate ( *lightMaps )
            world->lightMapDescs.add( World::LightMapDesc { lightMaps->current().getText(), ( String ) lightMaps->current().getAttrib( "resolution" ),
                    ( Image::StorageFormat ) String::toInt( lightMaps->current().getAttrib( "format" ) ) } );

        // Done loading

        rescanDirectories();

        world->rootNode = new RootWorldNode( world, res, root );
        world->rootNode->startup();
    }
    catch ( Exception ex )
    {
        Common::displayException( ex, true );
    }

    onWorldOpen();
}

void StormCraftFrame::onFileNew( wxCommandEvent& event )
{
    NewWorldDialog dlg( this );
    wxFileDialog fileDlg( this, "Save world as...", "", "", "StormCraft World files (*.world)|*.world|All files|*.*", wxFD_SAVE );

    if ( dlg.ShowModal() != wxID_OK || fileDlg.ShowModal() != wxID_OK )
        return;

    world = new World;
    world->gizmoObject = nullptr;
    world->fileName = fileDlg.GetPath();
    wxRename( world->fileName.c_str(), ( world->fileName + ".bak" ).c_str() );

    world->fileSystems.add( World::Fs { "native", false } );
    rescanDirectories();

    world->rootNode = new RootWorldNode( world, res, "World" );
    world->rootNode->startup();

    world->exportSettings.exportGeometry = true;
    world->exportSettings.cloneFileSystems = true;
    world->exportSettings.bspPolygonLimit = 500;
    world->exportSettings.bspVolumeLimit = Vector<float>( 50.0f, 50.0f, 50.0f );
    world->exportSettings.generateCtree2 = true;
    world->exportSettings.ctree2SegLimit = 5;

    world->mapInfoDoc.create();

    onWorldOpen();
}

void StormCraftFrame::onFileSave( wxCommandEvent& event )
{
    save();
}

void StormCraftFrame::onGuiMenuCommand( const MenuCommandEvent& event )
{
    bool changed = false;

    if ( event.menu == res->viewMenu )
    {
        if ( event.itemIndex == res->topCmd )
        {
            world->rootNode->camera[menuView] = RootWorldNode::topCamera();
            changed = true;
        }
        else if ( event.itemIndex == res->frontCmd )
        {
            world->rootNode->camera[menuView] = RootWorldNode::frontCamera();
            changed = true;
        }
        else if ( event.itemIndex == res->leftCmd )
        {
            world->rootNode->camera[menuView] = RootWorldNode::leftCamera();
            changed = true;
        }
    }

    if ( changed && selectedItemData && selectedItemData->node == world->rootNode )
    {
        List<Property> cameraProperties;

        world->rootNode->getCameraProperties( cameraProperties );
        updateProperties( cameraProperties );
    }
}

void StormCraftFrame::onGuiMenuToggle( const MenuToggleEvent& event )
{
    if ( event.menu == res->viewMenu )
    {
        if ( event.itemIndex == res->perspectiveTgl )
            world->rootNode->viewports[menuView].perspective = event.value;
        else if ( event.itemIndex == res->cullingTgl )
            world->rootNode->viewports[menuView].culling = event.value;
        else if ( event.itemIndex == res->wireframeTgl )
            world->rootNode->viewports[menuView].wireframe = event.value;
        else if ( event.itemIndex == res->shadedTgl )
            world->rootNode->viewports[menuView].shaded = event.value;
    }
}

void StormCraftFrame::onLmbDown( wxMouseEvent& event )
{
    do
    {
        if ( res->gui->onMouseButton( MouseButton::left, true, Vector2<int>( event.GetX(), event.GetY() ) ) )
            break;

        if ( world == nullptr )
            break;

        for ( size_t i = 0; i < lengthof( gizmoId ); i++ )
            gizmoId[i] = 0;

        unsigned id = renderCanvas( true, Vector2<unsigned>( event.GetX(), event.GetY() ) );

        //printf( "And the winner is.... %u!\n", id );

        if ( id != 0 && gizmoVisible )
        {
            if ( id == gizmoId[0] )
                drag = Vector<>( 1.0f, 1.0f, 0.0f );
            else if ( id == gizmoId[1] )
                drag = Vector<>( 1.0f, 0.0f, 1.0f );
            else if ( id == gizmoId[2] )
                drag = Vector<>( 0.0f, 1.0f, 1.0f );
            else if ( id == gizmoId[3] )
                drag = Vector<>( 1.0f, 0.0f, 0.0f );
            else if ( id == gizmoId[4] )
                drag = Vector<>( 0.0f, 1.0f, 0.0f );
            else if ( id == gizmoId[5] )
                drag = Vector<>( 0.0f, 0.0f, 1.0f );
        }

        if ( drag.getLength() < 0.01f )
        {
            for ( size_t i = 0; i < lengthof( gizmoVisible ); i++ )
                gizmoVisible[i] = false;

            world->gizmoObject = nullptr;
            world->rootNode->onPickingFinished( id, this );

            //drag = Vector<>();
        }

        if ( world->gizmoObject != nullptr )
            worldExplorerTree->SelectItem( world->gizmoObject->itemId );

        mouseMoveOrigin = Vector2<int>( event.GetX(), event.GetY() );
        lmbDown = true;

        dragView = getViewportAt( mouseMoveOrigin );
        selectCamera( dragView );
        graphicsDriver->getProjectionInfo( projectionBuffer );
    }
    while ( false );

    canvas->SetFocus();
    canvas->Refresh( false );
}

void StormCraftFrame::onLmbUp( wxMouseEvent& event )
{
    do
    {
        if ( res->gui->onMouseButton( MouseButton::left, false, Vector2<int>( event.GetX(), event.GetY() ) ) )
            break;

        if ( world == nullptr )
            break;

        drag = Vector<>();
        lmbDown = false;
    }
    while ( false );

    canvas->Refresh( false );
}

void StormCraftFrame::onMapExport( wxCommandEvent& event )
{
    if ( world == nullptr )
        return;

    //if ( wxMessageBox( "The world will be saved now.\n\nProceed?", "Export Map", wxYES_NO | wxICON_QUESTION, this ) != wxYES )
    //    return;

    try
    {
        //save();

        MapExportDialog dlg( this, world );

        if ( dlg.ShowModal() != wxID_OK )
            return;

        world->tmpDir = wxStandardPaths::Get().GetTempDir() + "/StormCraft_tmp_" + String::formatInt( clock() );

        wxFileName::Mkdir( ( const char* ) world->tmpDir );

        Object<Moxillan::DirectoryNode> rootDir = new Moxillan::DirectoryNode();

        world->mapInfoDoc.save( world->tmpDir + "/MapInfo.cfx2" );
        rootDir->add( new Moxillan::NativeFileNode( world->tmpDir + "/MapInfo.cfx2" ) );

        iterate2 ( i, world->lightMapDescs )
        {
            World::LightMapDesc& desc = i;

            World::LightMap lightMap { desc.name, new Image };
            lightMap.image->format = Image::Format::rgb;
            lightMap.image->size = desc.resolution;
            lightMap.image->data.resize( lightMap.image->size.x * lightMap.image->size.y * 3 );

            world->lightMaps.add( ( World::LightMap&& ) lightMap );
        }

        if ( world->exportSettings.exportGeometry )
        {
            const String& bspFileName = world->tmpDir + "/_BSP";

            Reference<OutputStream> output = File::open( bspFileName, true );

            Object<BspTree> geometry = world->rootNode->buildGeometry();
            BspWriter().save( geometry, output.detach() );
            geometry.release();

            rootDir->add( new Moxillan::NativeFileNode( bspFileName, nullptr, 9 ) );

            /*if ( world->numLightMaps > 0 )
            {
                Object<Moxillan::DirectoryNode> dir = new Moxillan::DirectoryNode( "_LIGHTMAPS" );

                for ( unsigned i = 0; i < world->numLightMaps; i++ )
                    dir->add( new Moxillan::NativeFileNode( world->tmpDir + "/lightmap_" + String::formatInt( i, -1, String::hexadecimal ), String::formatInt( i, -1, String::hexadecimal ), 9 ) );

                rootDir->add( dir.detach() );
            }*/
        }

        if ( world->exportSettings.generateCtree2 )
        {
            const String& ct2FileName = world->tmpDir + "/_Ctree2";

            Reference<OutputStream> output = File::open( ct2FileName, true );

            Object<Ct2Node> ctree2 = world->rootNode->buildCtree2();

            if ( ctree2 != nullptr )
            {
                Object<Ctree2Writer> ct2Writer = Ctree2Writer::create();
                ct2Writer->write( ctree2, output.detach() );
                ct2Writer.release();

                ctree2.release();
            }

            rootDir->add( new Moxillan::NativeFileNode( ct2FileName, nullptr, 9 ) );
        }

        // Export dynamic objects
        {
            const String& sgFileName = world->tmpDir + "/_SCENEGRAPH";

            Reference<OutputStream> output = File::open( sgFileName, true );
            output->write<Colour>( world->rootNode->sceneAmbient );

            writeGraphNode( output, world->rootNode, true );

            rootDir->add( new Moxillan::NativeFileNode( sgFileName, nullptr, 9 ) );
        }

        Moxillan::DirectoryNode* lightMapDir = nullptr;

        iterate2 ( i, world->lightMaps )
        {
            World::LightMap& lightMap = i;
            String filePath = world->tmpDir + "/" + lightMap.name;

            if ( lightMapDir == nullptr )
            {
                lightMapDir = new Moxillan::DirectoryNode( "_LIGHTMAPS" );
                rootDir->add( lightMapDir );
            }

            Reference<File> lightMapFile = File::open( filePath, true );
            SG_assert( lightMapFile != nullptr )

            //ImageWriter::save( lightMap.image, lightMapFile, Image::StorageFormat::dxt1 );
            ImageWriter::save( lightMap.image, lightMapFile, Image::StorageFormat::original );

            delete lightMap.image;
            lightMap.image = nullptr;

            lightMapDir->add( new Moxillan::NativeFileNode( filePath, lightMap.name, 9 ) );
        }

        world->lightMaps.clear();

        Moxillan::PackageBuilder::buildPackage( rootDir, File::open( world->exportSettings.fileName, true ), 0, 9 );
    }
    catch ( Exception ex )
    {
        Common::displayException( ex, true );
    }
}

void StormCraftFrame::onMmbDown( wxMouseEvent& event )
{
    if ( world == nullptr )
        return;

    mouseMoveOrigin = Vector2<int>( event.GetX(), event.GetY() );
    mmbDown = true;

    dragView = getViewportAt( mouseMoveOrigin );
    selectCamera( dragView );
    graphicsDriver->getProjectionInfo( projectionBuffer );

    canvas->SetFocus();
}

void StormCraftFrame::onMmbUp( wxMouseEvent& event )
{
    if ( world == nullptr )
        return;

    mmbDown = false;
}

void StormCraftFrame::onMouseMove( wxMouseEvent& event )
{
    const Vector2<int> mousePos( event.GetX(), event.GetY() );

    res->gui->onMouseMoveTo( mousePos );

    if ( world == nullptr || ( !lmbDown && !mmbDown && !rmbDown ) )
        return;

    Vector2<int> delta = mousePos - mouseMoveOrigin;

    float scaleFactor = world->rootNode->viewports[dragView].perspective ? 250.0f : 2000.0f;

    if ( lmbDown )
    {
        if ( world->gizmoObject != nullptr )
        {
            dragBuffer += graphicsDriver->unproject( delta, projectionBuffer ) * drag * world->rootNode->camera[dragView].getDistance() / scaleFactor;

            Vector<> dragVect;
            bool flush = false;

            while ( dragBuffer.x > DRAG_THRESHOLD )
            {
                dragVect.x += DRAG_THRESHOLD;
                dragBuffer.x -= DRAG_THRESHOLD;
                flush = true;
            }

            while ( dragBuffer.x < -DRAG_THRESHOLD )
            {
                dragVect.x -= DRAG_THRESHOLD;
                dragBuffer.x += DRAG_THRESHOLD;
                flush = true;
            }

            while ( dragBuffer.y > DRAG_THRESHOLD )
            {
                dragVect.y += DRAG_THRESHOLD;
                dragBuffer.y -= DRAG_THRESHOLD;
                flush = true;
            }

            while ( dragBuffer.y < -DRAG_THRESHOLD )
            {
                dragVect.y -= DRAG_THRESHOLD;
                dragBuffer.y += DRAG_THRESHOLD;
                flush = true;
            }

            while ( dragBuffer.z > DRAG_THRESHOLD )
            {
                dragVect.z += DRAG_THRESHOLD;
                dragBuffer.z -= DRAG_THRESHOLD;
                flush = true;
            }

            while ( dragBuffer.z < -DRAG_THRESHOLD )
            {
                dragVect.z -= DRAG_THRESHOLD;
                dragBuffer.z += DRAG_THRESHOLD;
                flush = true;
            }

            if ( flush )
            {
                world->gizmoObject->drag( dragVect );
                gizmoTransforms[1].vector += dragVect;
            }
        }
    }
    else if ( mmbDown )
    {
        Vector<> vec = graphicsDriver->unproject( delta, projectionBuffer ) * world->rootNode->camera[dragView].getDistance() / scaleFactor;

        world->rootNode->camera[dragView].move( -vec );
    }
    else if ( rmbDown )
    {
        world->rootNode->camera[dragView].rotateXY( delta.y * M_PI_2 / 400.0f );
        world->rootNode->camera[dragView].rotateZ( -delta.x * M_PI_2 / 200.0f );
    }

    mouseMoveOrigin = mousePos;

    canvas->Refresh( false );

    if ( selectedItemData && selectedItemData->node == world->rootNode )
    {
        List<Property> cameraProperties;

        world->rootNode->getCameraProperties( cameraProperties );
        updateProperties( cameraProperties );
    }
}

void StormCraftFrame::onMouseWheel( wxMouseEvent& event )
{
    if ( world == nullptr )
        return;

    unsigned view = getViewportAt( Vector2<unsigned>( event.GetX(), event.GetY() ) );

    world->rootNode->camera[view].zoom( -event.GetWheelRotation() / 50.0f );

    canvas->Refresh( false );

    if ( selectedItemData && selectedItemData->node->getClassName() == "Root" )
    {
        //propertyGrid->Clear();
        buildPropertyPane();
    }
}

void StormCraftFrame::onMultipleViewsToggle( wxCommandEvent& event )
{
    multipleViews = event.IsChecked();

    canvas->Refresh( false );
}

void StormCraftFrame::onPropertyGridValueChanged( wxPropertyGridEvent& event )
{
    WorldNode* node = selectedItemData->node;

    iterate ( currentNodeProperties )
    {
        Property& property = currentNodeProperties.current();

        if ( property.name == ( const char* ) event.GetPropertyName() )
        {
            switch ( property.type )
            {
                case Property::boolean:
                    property.boolValue = event.GetPropertyValue().GetBool();
                    break;

                case Property::lightMapArea:
                case Property::text:
                    property.textValue = event.GetPropertyValue().GetString();
                    break;

                case Property::enumeration:
                    property.enumValue = event.GetPropertyValue().GetInteger();
                    break;

                case Property::floating:
                    property.floatingValue = event.GetPropertyValue().GetDouble();
                    break;

                case Property::colour:
                {
                    wxColour colour;
                    colour << event.GetPropertyValue();
                    property.colourValue = Colour( colour.Red(), colour.Green(), colour.Blue(), colour.Alpha() );
                    break;
                }

                case Property::vector:
                    property.vectorValue = Vector<>( ( const char* ) event.GetPropertyValue().GetString() );
                    break;

                case Property::vector2i:
                    property.vector2iValue = Vector2<int>( ( const char* ) event.GetPropertyValue().GetString() );
                    break;

                case Property::vector2f:
                    property.vector2fValue = Vector2<>( ( const char* ) event.GetPropertyValue().GetString() );
                    break;
            }

            if ( !node->setProperty( property ) )
            {
                eventLog->AppendToPage( ( "<font color=red><b>Warning:</b> unknown property `" + property.name + "`</font><br>" ).c_str() );
                showEventLog();
            }
            break;
        }
    }

    if ( event.GetPropertyName() == "name" )
        worldExplorerTree->SetItemText( selectedItemData->GetId(), node->getName().c_str() );

    if ( node == world->rootNode )
    {
        List<Property> cameraProperties;

        world->rootNode->getCameraProperties( cameraProperties );
        updateProperties( cameraProperties );
    }

    canvas->Refresh( false );
}

void StormCraftFrame::OnQuit( wxCommandEvent& event )
{
    Close();
}

void StormCraftFrame::onRmbDown( wxMouseEvent& event )
{
    //canvas->SetFocus();

    if ( res->gui->onMouseButton( MouseButton::right, true, Vector2<int>( event.GetX(), event.GetY() ) ) )
        ;
    else if ( world != nullptr && event.ControlDown() )
    {
        menuView = getViewportAt( Vector2<unsigned>( event.GetX(), event.GetY() ) );

        res->viewMenu = res->gui->createPopupMenu();

        res->topCmd = res->viewMenu->addCommand( "Top" );
        res->frontCmd = res->viewMenu->addCommand( "Front" );
        res->leftCmd = res->viewMenu->addCommand( "Left" );
        res->viewMenu->addSpacer();
        res->perspectiveTgl = res->viewMenu->addToggle( "Perspective", world->rootNode->viewports[menuView].perspective );
        res->cullingTgl = res->viewMenu->addToggle( "Culling", world->rootNode->viewports[menuView].culling );
        res->wireframeTgl = res->viewMenu->addToggle( "Wireframe", world->rootNode->viewports[menuView].wireframe );
        res->shadedTgl = res->viewMenu->addToggle( "Shaded", world->rootNode->viewports[menuView].shaded );

        res->viewMenu->setEventListener( this );
        res->viewMenu->show( Vector2<>( event.GetX(), event.GetY() ) );
    }
    else if ( world != nullptr )
    {
        mouseMoveOrigin = Vector2<int>( event.GetX(), event.GetY() );
        rmbDown = true;

        dragView = getViewportAt( mouseMoveOrigin );
        selectCamera( dragView );
        graphicsDriver->getProjectionInfo( projectionBuffer );
    }

    canvas->Refresh();
}

void StormCraftFrame::onRmbUp( wxMouseEvent& event )
{
    rmbDown = false;
}

void StormCraftFrame::onRotateZ( wxCommandEvent& event )
{
    if ( selectedItemData == nullptr )
        return;

    WorldNode* node = selectedItemData->node;

    node->rotate( Vector<>( 0.0f, 0.0f, -1.0f ), String::toFloat( wxGetTextFromUser( "Please enter the angle (in degrees, CCW):", "Rotate", "0", this ) ) * M_PI / 180.0f );

    canvas->Refresh( false );
}

void StormCraftFrame::onWorldExplorerTreeBeginDrag( wxTreeEvent& event )
{
    event.Allow();

    dragItemData = static_cast<WorldNodeTreeItemData*>( event.GetClientObject() );
}

void StormCraftFrame::onWorldExplorerTreeEndDrag( wxTreeEvent& event )
{
    WorldNodeTreeItemData* destData = static_cast<WorldNodeTreeItemData*>( event.GetClientObject() );

    if ( destData == nullptr )
        return;

    WorldNode* node = dragItemData->node;
    GroupWorldNode* group = dynamic_cast<GroupWorldNode*>( destData->node );

    if ( node == nullptr || group == nullptr || node == group )
        return;

    WorldNodeTreeItemData* parent = dragItemData->parent;
    worldExplorerTree->Delete( dragItemData->GetId() );

    group->add( node );

    addWorldExplorerTreeNode( destData, node, false, false );

    static_cast<GroupWorldNode*>( parent->node )->remove( node );
}

void StormCraftFrame::onWorldExplorerTreeItemMenu( wxTreeEvent& event )
{
    selectedItemData = static_cast<WorldNodeTreeItemData*>( event.GetClientObject() );
    WorldNode* node = selectedItemData->node;

    if ( dynamic_cast<GroupWorldNode*>( node ) != nullptr )
        PopupMenu( &groupWorldNodeItemMenu );
}

void StormCraftFrame::onWorldExplorerTreeKeyDown( wxTreeEvent& event )
{
    if ( event.GetKeyCode() != WXK_DELETE )
        return;

    if ( selectedItemData == nullptr )
        return;

    WorldNode* node = selectedItemData->node;

    try
    {
        if ( wxMessageBox( "Delete the selected object?", "Break into faces", wxYES_NO | wxICON_WARNING ) != wxYES )
            return;

        WorldNodeTreeItemData* parent = selectedItemData->parent;
        worldExplorerTree->Delete( selectedItemData->GetId() );
        selectedItemData = nullptr;

        dynamic_cast<GroupWorldNode*>( parent->node )->remove( node );
    }
    catch ( Exception ex )
    {
        Common::displayException( ex, true );
    }
}

void StormCraftFrame::onWorldExplorerTreeItemRenamed( wxTreeEvent& event )
{
    selectedItemData = static_cast<WorldNodeTreeItemData*>( event.GetClientObject() );
    WorldNode* node = selectedItemData->node;

    if ( !event.GetLabel().IsEmpty() )
    {
        Property rename( "name", ( String ) ( const char* ) event.GetLabel() );

        node->setProperty( rename );
        updateProperty( rename );
    }
}

void StormCraftFrame::onWorldExplorerTreeSelectionChanged( wxTreeEvent& event )
{
    // Do this first (before touching 'selectedItemData') to properly register all edits
    propertyGrid->Clear();

    selectedItemData = static_cast<WorldNodeTreeItemData*>( event.GetClientObject() );

    buildPropertyPane();
    selectedItemData->node->onSelect( this );

    canvas->Refresh( false );
}

void StormCraftFrame::onWorldOpen()
{
    worldExplorerTree->DeleteAllItems();

    if ( world->rootNode != nullptr )
        addWorldExplorerTreeNode( nullptr, world->rootNode, true, false );

    for ( size_t i = 0; i < lengthof( gizmoVisible ); i++ )
        gizmoVisible[i] = false;
}

unsigned StormCraftFrame::renderCanvas( bool picking, const Vector2<unsigned>& mouse )
{
    const Vector2<unsigned> canvasSize( canvas->GetSize().x, canvas->GetSize().y );

    unsigned pickingId = 0;

    if ( picking )
        graphicsDriver->beginPicking();

    if ( multipleViews )
    {
        if ( picking )
        {
            unsigned view = getViewportAt( mouse );

            renderScene( picking, view );
        }
        else
            for ( unsigned i = 0; i < 4; i++ )
                renderScene( false, i );
    }
    else
        renderScene( picking, 0 );

    if ( picking )
    {
        pickingId = graphicsDriver->endPicking( mouse );
        graphicsDriver->getProjectionInfo( projectionBuffer );
    }

    return pickingId;
}

void StormCraftFrame::renderScene( bool picking, unsigned view )
{
    const Vector2<unsigned> canvasSize( canvas->GetSize().x, canvas->GetSize().y );

    currentViewport = view;

    if ( multipleViews )
    {
        const Vector2<unsigned> viewport( canvasSize / 2 );

        graphicsDriver->setViewport( Vector2<int>( viewport.x * ( view % 2 ), viewport.y * ( view / 2 ) ), viewport, canvasSize );
    }
    else
        graphicsDriver->setViewport( Vector2<int>(), canvasSize, canvasSize );

    selectCamera( view );

    world->rootNode->render( this, picking );

    graphicsDriver->setRenderFlag( RenderFlag::depthTest, false );

    const float scale = world->rootNode->camera[view].getDistance() / 5.0f;
    gizmoTransforms[0].vector.x = scale;
    gizmoTransforms[0].vector.y = scale;
    gizmoTransforms[0].vector.z = scale;

    for ( size_t i = 0; i < lengthof( gizmoVisible ); i++ )
        if ( gizmoVisible[i] )
        {
            if ( picking )
                gizmoId[i] = res->gizmo[i]->pick( gizmoTransforms, lengthof( gizmoTransforms ) );
            else
                res->gizmo[i]->render( gizmoTransforms, lengthof( gizmoTransforms ) );
        }
}

void StormCraftFrame::rescanDirectories()
{
    world->resMgr.release();

    world->vfs.release();
    world->vfs = dynamic_cast<IUnionFileSystem*>( sg->createFileSystem( "union" ) );

    try
    {
        iterate ( world->fileSystems )
            world->vfs->add( sg->createFileSystem( world->fileSystems.current().name ) );
    }
    catch ( Exception ex )
    {
        Common::displayException( ex, true );
    }

    // TODO: Rescan directories
    world->resMgr = sg->createResourceManager( "world/resMgr", world->vfs->reference() );
    world->resMgr->addPath( "" );
}

void StormCraftFrame::save()
{
    if ( world == nullptr )
        return;

    cfx2::Document doc;

    // File Systems
    iterate ( world->fileSystems )
    {
        const World::Fs& fs = world->fileSystems.current();

        cfx2::Node node = doc.createChild( "FileSystem", fs.name );
        node.setAttrib( "cloneInPackage", String::formatInt( fs.cloneInPackage ) );
    }

    // Map Export settings
    cfx2::Node exportSettings = doc.createChild( "ExportSettings" );
    exportSettings.setAttrib( "fileName", world->exportSettings.fileName );
    exportSettings.setAttrib( "exportGeometry", String::formatBool( world->exportSettings.exportGeometry ) );
    exportSettings.setAttrib( "cloneFileSystems", String::formatBool( world->exportSettings.cloneFileSystems ) );
    exportSettings.setAttrib( "bspPolygonLimit", String::formatInt( world->exportSettings.bspPolygonLimit ) );
    exportSettings.setAttrib( "bspVolumeLimit", world->exportSettings.bspVolumeLimit.toString() );
    exportSettings.setAttrib( "generateCtree2", String::formatBool( world->exportSettings.generateCtree2 ) );
    exportSettings.setAttrib( "ctree2SegLimit", String::formatInt( world->exportSettings.ctree2SegLimit ) );

    // LightMaps
    iterate2 ( i, world->lightMapDescs )
    {
        const World::LightMapDesc& lightMap = i;

        cfx2::Node node = doc.createChild( "LightMap", lightMap.name );
        node.setAttrib( "resolution", lightMap.resolution.toString() );
        node.setAttrib( "format", String::formatInt( ( int ) lightMap.format ) );
    }

    doc.addChild( world->mapInfoDoc.clone() );

    world->rootNode->save( doc );

    doc.save( world->fileName );
}

void StormCraftFrame::selectCamera( unsigned viewport )
{
    graphicsDriver->clearLights();

    if ( world->rootNode->viewports[viewport].perspective )
    {
        graphicsDriver->set3dMode( 0.1f, 2000.0f );
    }
    else
    {
        float vFovRange = world->rootNode->camera[viewport].getDistance() * tan( world->rootNode->viewports[viewport].fov * M_PI / 180.0f );

        const Vector2<> vFov( -vFovRange / 2, vFovRange / 2 );
        const Vector2<> hFov( vFov * ( ( float ) canvas->GetSize().x / canvas->GetSize().y ) );

        graphicsDriver->setOrthoProjection( hFov, vFov, Vector2<>( 0.0f, 1000.0f ) );
    }

    graphicsDriver->setCamera( &world->rootNode->camera[viewport] );
    graphicsDriver->setRenderFlag( RenderFlag::culling, world->rootNode->viewports[viewport].culling );
    graphicsDriver->setRenderFlag( RenderFlag::wireframe, world->rootNode->viewports[viewport].wireframe );

    if ( !world->rootNode->viewports[viewport].shaded )
        graphicsDriver->setSceneAmbient( Colour::white() );
    else
    {
        graphicsDriver->setSceneAmbient( Colour::grey( 0.4f ) );
        res->worldLight->render( nullptr, 0, false );
    }
}

void StormCraftFrame::showEventLog()
{
    wxAuiPaneInfo& eventLogPane = auiManager->GetPane( eventLog );

    if ( eventLogPane.IsOk() && !eventLogPane.IsShown() )
    {
        eventLogPane.Show();
        auiManager->Update();
    }
}

void StormCraftFrame::showGizmo( const Vector<>& pos, WorldNode* node, const bool visible[3] )
{
    gizmoTransforms[1].vector = pos;
    world->gizmoObject = node;

    gizmoVisible[0] = visible[0] && visible[1];
    gizmoVisible[1] = visible[0] && visible[2];
    gizmoVisible[2] = visible[1] && visible[2];

    for ( size_t i = 0; i < 3; i++ )
        gizmoVisible[3 + i] = visible[i];

    //drag = Vector<>( visible[0] ? 1.0f : 0.0f, visible[1] ? 1.0f : 0.0f, visible[2] ? 1.0f : 0.0f );
}

void StormCraftFrame::updateProperties( const List<Property>& propertyList )
{
    iterate2 ( property, propertyList )
        updateProperty( property );
}

void StormCraftFrame::updateProperty( const Property& property )
{
    wxPGProperty* gridProperty = propertyGrid->GetPropertyByName( property.name.c_str() );

    switch ( property.type )
    {
        case Property::boolean:
        {
            bool value = property.boolValue;

            if ( gridProperty == nullptr )
            {
                gridProperty = new wxBoolProperty( property.name.c_str(), property.name.c_str(), value );
                gridProperty->SetAttribute( wxPG_BOOL_USE_CHECKBOX, true );
                propertyGrid->Append( gridProperty );
            }
            else
                gridProperty->SetValue( value );
            break;
        }

        case Property::colour:
        {
            wxColour value( property.colourValue.getR(), property.colourValue.getG(), property.colourValue.getB(), property.colourValue.getA() );

            if ( gridProperty == nullptr )
            {
                gridProperty = new wxColourProperty( property.name.c_str(), property.name.c_str(), value );
                propertyGrid->Append( gridProperty );
            }
            else
                gridProperty->SetValue( wxVariant( wxAny( value ) ) );
            break;
        }

        case Property::enumeration:
        {
            wxPGChoices choices;

            for each_in_list ( property.enumValues, i )
                choices.Add( property.enumValues[i].c_str(), i );

            propertyGrid->Append( new wxEnumProperty( property.name.c_str(), property.name.c_str(), choices, property.enumValue ) );
            break;
        }

        case Property::floating:
        {
            if ( gridProperty == nullptr )
            {
                gridProperty = new wxFloatProperty( property.name.c_str(), property.name.c_str(), property.floatingValue );
                propertyGrid->Append( gridProperty );
            }
            else
                gridProperty->SetValue( property.floatingValue );
            break;
        }

        case Property::lightMapArea:
        {
            String value = property.textValue;

            if ( gridProperty == nullptr )
            {
                gridProperty = new LightMapAreaProperty( this, world, res, selectedItemData->node, property.name.c_str(), property.name.c_str(), value.c_str() );
                propertyGrid->Append( gridProperty );
            }
            else
                gridProperty->SetValue( value.c_str() );
            break;
        }

        case Property::text:
        {
            String value = property.textValue;

            if ( gridProperty == nullptr )
            {
                gridProperty = new wxStringProperty( property.name.c_str(), property.name.c_str(), value.c_str() );
                propertyGrid->Append( gridProperty );
            }
            else
                gridProperty->SetValue( value.c_str() );
            break;
        }

        case Property::vector:
        {
            String value = property.vectorValue.toString();

            if ( gridProperty == nullptr )
            {
                gridProperty = new wxStringProperty( property.name.c_str(), property.name.c_str(), value.c_str() );
                propertyGrid->Append( gridProperty );
            }
            else
                gridProperty->SetValue( value.c_str() );
            break;
        }

        case Property::vector2f:
        {
            String value = property.vector2fValue.toString();

            if ( gridProperty == nullptr )
            {
                gridProperty = new wxStringProperty( property.name.c_str(), property.name.c_str(), value.c_str() );
                propertyGrid->Append( gridProperty );
            }
            else
                gridProperty->SetValue( value.c_str() );
            break;
        }

        case Property::vector2i:
        {
            String value = property.vector2iValue.toString();

            if ( gridProperty == nullptr )
            {
                gridProperty = new wxStringProperty( property.name.c_str(), property.name.c_str(), value.c_str() );
                propertyGrid->Append( gridProperty );
            }
            else
                gridProperty->SetValue( value.c_str() );
            break;
        }
    }
}
