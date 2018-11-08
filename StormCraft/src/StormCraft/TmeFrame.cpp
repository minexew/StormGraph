#include "TmeFrame.hpp"

//(*InternalHeaders(TmeFrame)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <StormGraph/ResourceManager.hpp>

//(*IdInit(TmeFrame)
const long TmeFrame::ID_GLCANVAS1 = wxNewId();
const long TmeFrame::ID_LISTBOX1 = wxNewId();
const long TmeFrame::ID_BUTTON1 = wxNewId();
const long TmeFrame::ID_PANEL1 = wxNewId();
const long TmeFrame::ID_AUINOTEBOOK1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(TmeFrame, wxDialog)
	//(*EventTable(TmeFrame)
	//*)
END_EVENT_TABLE()

static const Vector2<unsigned> handleSize( 8, 8 );
static const Vector2<unsigned> padding( 20, 20 );

TmeFrame::TmeFrame( World* world, Resources* res, wxGLContext* ctx, WorldNode* editNode, const String& initValue, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size )
        : ctx( ctx ), editNode( editNode ), world( world ), res( res ), handleDrag( -1 )
{
    //windowDisabler = new wxWindowDisabler( this );

	//(*Initialize(TmeFrame)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("Light Mapping Editor"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX|wxMINIMIZE_BOX, _T("id"));
	SetClientSize(wxSize(886,509));
	Move(wxDefaultPosition);
	auiManager = new wxAuiManager(this, wxAUI_MGR_DEFAULT);
	int GLCanvasAttributes_1[] = {
		WX_GL_RGBA,
		WX_GL_DOUBLEBUFFER,
		WX_GL_DEPTH_SIZE,      16,
		WX_GL_STENCIL_SIZE,    0,
		0, 0 };
	canvas = new wxGLCanvas(this, ID_GLCANVAS1, wxPoint(198,12), wxDefaultSize, 0, _T("ID_GLCANVAS1"), GLCanvasAttributes_1);
	auiManager->AddPane(canvas, wxAuiPaneInfo().Name(_T("Mapping View")).CenterPane().Caption(_("Mapping View")));
	AuiNotebook1 = new wxAuiNotebook(this, ID_AUINOTEBOOK1, wxPoint(8,300), wxDefaultSize, wxAUI_NB_DEFAULT_STYLE);
	AuiNotebook1->SetMinSize(wxSize(200,0));
	Panel1 = new wxPanel(AuiNotebook1, ID_PANEL1, wxPoint(9,323), wxSize(117,400), wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer1 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	lightMapList = new wxListBox(Panel1, ID_LISTBOX1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_LISTBOX1"));
	FlexGridSizer1->Add(lightMapList, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	createNew = new wxButton(Panel1, ID_BUTTON1, _("Create new"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer1->Add(createNew, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel1->SetSizer(FlexGridSizer1);
	FlexGridSizer1->SetSizeHints(Panel1);
	AuiNotebook1->AddPage(Panel1, _("Light Maps"));
	auiManager->AddPane(AuiNotebook1, wxAuiPaneInfo().Name(_T("PaneName")).Caption(_("Pane caption")).CaptionVisible().PinButton().Left().MinSize(wxSize(200,0)));
	auiManager->Update();

	canvas->Connect(wxEVT_PAINT,(wxObjectEventFunction)&TmeFrame::onCanvasPaint,0,this);
	canvas->Connect(wxEVT_LEFT_DOWN,(wxObjectEventFunction)&TmeFrame::onLmbDown,0,this);
	canvas->Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&TmeFrame::onLmbUp,0,this);
	canvas->Connect(wxEVT_MOTION,(wxObjectEventFunction)&TmeFrame::onMouseMove,0,this);
	canvas->Connect(wxEVT_SIZE,(wxObjectEventFunction)&TmeFrame::onCanvasResize,0,this);
	Connect(ID_LISTBOX1,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&TmeFrame::onListSelect);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TmeFrame::onCreateNew);
	//*)

    propertyGrid = new wxPropertyGrid( this );
    propertyGrid->Connect( wxEVT_PG_CHANGED, ( wxObjectEventFunction ) &TmeFrame::onPropertyGridValueChanged, 0, this );
    auiManager->AddPane( propertyGrid, wxAuiPaneInfo().Name( _T( "propertyGridPane" ) ).DefaultPane().Caption( _( "Properties" ) ).PinButton().Left() );

    auiManager->Update();

    ctx->SetCurrent( *canvas );
    labelFont = res->resMgr->getFont( "Common/Fonts/DejaVuSans.ttf", 11, IFont::normal );

    selection = -1;

    if ( editNode != nullptr )
        parse( initValue, currentMapping );

    printf( "editNode = %p, initValue = '%s'\n", editNode, initValue.c_str() );

    iterate2 ( i, world->lightMapDescs )
    {
        World::LightMapDesc& lightMap = i;

        printf( "(%s|%s)\n", lightMap.name.c_str(), currentMapping.name.c_str() );

        if ( editNode != nullptr && lightMap.name == currentMapping.name )
            selection = i.getIndex();

        lightMapList->Append( lightMap.name.c_str() );
    }

    onSelect();
    updateSizes( true );

	/*canvas = new wxGLCanvas( this, ID_GLCANVAS1, GLCanvasAttributes_1, wxPoint(236,285), wxDefaultSize, 0, _T("ID_GLCANVAS1") );
	AuiManager1->AddPane(canvas, wxAuiPaneInfo().Name(_T("Mapping View")).CenterPane().Caption(_("Mapping View")));

	canvas->Connect( wxEVT_PAINT, ( wxObjectEventFunction ) &TmeFrame::onCanvasPaint, 0, this );*/
}

TmeFrame::~TmeFrame()
{
	//(*Destroy(TmeFrame)
	//*)

	// Has to be done for some reason
    // (wxWidgets buggy much?)
    RemoveEventHandler( GetEventHandler() );
}

void TmeFrame::addMappings( GroupWorldNode* node )
{
    iterate2 ( child, node->children )
    {
        if ( child == editNode )
            continue;

        LitWorldNode* asLit = dynamic_cast<LitWorldNode*>( *child );

        if ( asLit != nullptr )
        {
            Mapping mapping;

            if ( parse( asLit->lightMapArea, mapping ) && mapping.name == currentMapping.name )
            {
                mapping.text = labelFont->layoutText( child->name, Colour::white(), IFont::centered | IFont::middle );
                selectionMappings.add( ( Mapping&& ) mapping );
            }
        }

        GroupWorldNode* asGroup = dynamic_cast<GroupWorldNode*>( *child );

        if ( asGroup != nullptr )
            addMappings( asGroup );
    }
}

String TmeFrame::getValue() const
{
    return currentMapping.name + "(" + currentMapping.uv[0].toString() + ";" + currentMapping.uv[1].toString() + ")";
}

void TmeFrame::onCanvasPaint( wxPaintEvent& event )
{
    try
    {
        ctx->SetCurrent( *canvas );
        IEventListener* listener = graphicsDriver->getEventListener();

        graphicsDriver->setClearColour( Colour::white() );
        listener->onFrameBegin();

        graphicsDriver->setViewport( Vector2<int>(), canvasSize, canvasSize );

        if ( selection >= 0 || ( unsigned ) selection < world->lightMapDescs.getLength() )
        {
            //World::LightMapDesc& desc = world->lightMapDescs[selection];

            graphicsDriver->set2dMode( 1.0f, -1.0f );

            const Colour gridColour = Colour::grey( 0.5f );
            const Colour mappingColour( 1.0f, 0.0f, 0.0f, 0.6f );
            const Colour mappingOutlineColour( 1.0f, 0.0f, 0.0f, 0.8f );
            const Colour editMappingColour( 0.0f, 0.75f, 1.0f, 0.8f );
            const Colour editMappingOutlineColour( 0.0f, 0.75f, 1.0f, 0.9f );

            for ( unsigned x = 0; x <= grid.x; x++ )
                graphicsDriver->drawLine( origin + Vector2<unsigned>( x * scale, 0 ), origin + Vector2<unsigned>( x * scale, grid.y * scale ), gridColour );

            for ( unsigned y = 0; y <= grid.y; y++ )
                graphicsDriver->drawLine( origin + Vector2<unsigned>( 0, y * scale ), origin + Vector2<unsigned>( grid.x * scale, y * scale ), gridColour );

            iterate2 ( i, selectionMappings )
            {
                Mapping& mapping = i;

                const Vector2<unsigned> pos( origin + mapping.uv[0] * grid * scale );
                const Vector2<unsigned> size( ( mapping.uv[1] - mapping.uv[0] ) * grid * scale );

                graphicsDriver->drawRectangle( pos, size, mappingColour, nullptr );
                graphicsDriver->drawRectangleOutline( pos, size, mappingOutlineColour, nullptr );
                labelFont->drawText( pos + size / 2, mapping.text );
            }

            if ( editNode != nullptr )
            {
                const Colour handleColour = Colour::white();
                const Colour handleOutlineColour = Colour::grey( 0.4f );

                graphicsDriver->drawRectangle( currentMappingPos, currentMappingSize, editMappingColour, nullptr );
                graphicsDriver->drawRectangleOutline( currentMappingPos, currentMappingSize, editMappingOutlineColour, nullptr );

                for ( size_t i = 0; i < lengthof( handle ); i++ )
                {
                    graphicsDriver->drawRectangle( handle[i], handleSize, handleColour, nullptr );
                    graphicsDriver->drawRectangleOutline( handle[i], handleSize, handleOutlineColour, nullptr );
                }

                labelFont->drawString( currentMappingPos + currentMappingSize / 2, ( ( currentMapping.uv[1] - currentMapping.uv[0] ) * grid ).toString(),
                        Colour::white(), IFont::centered | IFont::middle );
            }
        }

        //sg->onRender();

        listener->onFrameEnd();

        canvas->SwapBuffers();
    }
    catch ( Exception ex )
    {
        Close();

        Common::displayException( ex, false );
    }
}

void TmeFrame::onCanvasResize( wxSizeEvent& event )
{
    updateSizes( true );
}

void TmeFrame::onCreateNew( wxCommandEvent& event )
{
    World::LightMapDesc lightMap { "Unnamed_Lightmap", Vector2<unsigned>( 64, 64 ) };

    world->lightMapDescs.add( lightMap );

    selection = lightMapList->Append( lightMap.name.c_str() );
    lightMapList->Select( selection );

    onSelect();
}

void TmeFrame::onListSelect( wxCommandEvent& event )
{
    selection = lightMapList->GetSelection();

    onSelect();
}

void TmeFrame::onLmbDown( wxMouseEvent& event )
{
    if ( editNode == nullptr )
        return;

    dragOrigin = Vector2<unsigned>( event.GetX(), event.GetY() );

    for ( size_t i = 0; i < lengthof( handle ); i++ )
    {
        if ( !( dragOrigin < handle[i] ) && !( dragOrigin > handle[i] + handleSize ) )
        {
            handleDrag = i;
            break;
        }
    }
}

void TmeFrame::onLmbUp( wxMouseEvent& event )
{
    handleDrag = -1;
}

void TmeFrame::onMouseMove( wxMouseEvent& event )
{
    if ( handleDrag < 0 )
        return;

    float cellX = round( ( int )( event.GetX() - origin.x ) / ( float ) scale );
    float cellY = round( ( int )( event.GetY() - origin.y ) / ( float ) scale );

    const Vector2<> cell( maximum<>( minimum<float>( cellX, grid.x ), 0.0f ), maximum<>( minimum<float>( cellY, grid.y ), 0.0f ) );

    if ( handleDrag == 0 )
        currentMapping.uv[0] = cell / grid;
    else if ( handleDrag == 1 )
    {
        currentMapping.uv[0].x = cell.x / grid.x;
        currentMapping.uv[1].y = cell.y / grid.y;
    }
    else if ( handleDrag == 2 )
        currentMapping.uv[1] = cell / grid;
    else if ( handleDrag == 3 )
    {
        currentMapping.uv[1].x = cell.x / grid.x;
        currentMapping.uv[0].y = cell.y / grid.y;
    }

    updateSizes( false );
}

void TmeFrame::onPropertyGridValueChanged( wxPropertyGridEvent& event )
{
    if ( selection < 0 || ( unsigned ) selection >= world->lightMapDescs.getLength() )
        return;

    if ( String::equals( event.GetPropertyName(), "name" ) )
    {
        world->lightMapDescs[selection].name = event.GetPropertyValue().GetString();

        lightMapList->Delete( selection );
        lightMapList->Insert( world->lightMapDescs[selection].name.c_str(), selection );
    }
    else if ( String::equals( event.GetPropertyName(), "resolution" ) )
        world->lightMapDescs[selection].resolution = Vector2<unsigned>( ( String ) event.GetPropertyValue().GetString() );
}

void TmeFrame::onSelect()
{
    // Update current selection

    if ( selection < 0 || ( unsigned ) selection >= world->lightMapDescs.getLength() )
        return;

    const World::LightMapDesc& lightMap = world->lightMapDescs[selection];

    propertyGrid->Clear();

    propertyGrid->Append( new wxStringProperty( "name", "name", lightMap.name.c_str() ) );
    propertyGrid->Append( new wxStringProperty( "resolution", "resolution", lightMap.resolution.toString().c_str() ) );

    currentMapping.name = lightMap.name;

    iterate2 ( i, selectionMappings )
    {
        Mapping& mapping = i;

        if ( mapping.text != nullptr )
            labelFont->releaseText( mapping.text );
    }

    selectionMappings.clear();

    addMappings( world->rootNode );

    canvas->Refresh( false );
}

bool TmeFrame::parse( const String& lightMapping, Mapping& currentMapping )
{
    int lparen = lightMapping.findChar( '(' );

    if ( lparen < 0 )
        return false;

    int semicolon = lightMapping.findChar( ';', lparen + 1 );

    if ( semicolon < 0 )
        return false;

    currentMapping.name = lightMapping.left( lparen );
    currentMapping.uv[0] = lightMapping.left( semicolon ).dropLeft( lparen + 1 );
    currentMapping.uv[1] = lightMapping.dropLeft( semicolon + 1 );
    currentMapping.text = nullptr;

    return true;
}

void TmeFrame::updateSizes( bool all )
{
    if ( all )
    {
        canvasSize = Vector2<unsigned>( canvas->GetSize().x, canvas->GetSize().y );
        area = Vector2<unsigned>( canvasSize - padding * 2 );
        grid = Vector2<unsigned>( 64, 64 );
        origin = Vector2<unsigned>( ( canvasSize - grid * scale ) / 2 );

        scale = minimum( area.x / grid.x, area.y / grid.y );
    }

    if ( editNode != nullptr )
    {
        if ( currentMapping.uv[0].x > currentMapping.uv[1].x )
        {
            float tmp = currentMapping.uv[0].x;
            currentMapping.uv[0].x = currentMapping.uv[1].x;
            currentMapping.uv[1].x = tmp;
        }

        if ( currentMapping.uv[0].y > currentMapping.uv[1].y )
        {
            float tmp = currentMapping.uv[0].y;
            currentMapping.uv[0].y = currentMapping.uv[1].y;
            currentMapping.uv[1].y = tmp;
        }

        currentMappingPos = Vector2<unsigned>( origin + currentMapping.uv[0] * grid * scale );
        currentMappingSize = Vector2<unsigned>( ( currentMapping.uv[1] - currentMapping.uv[0] ) * grid * scale );

        handle[0] = Vector2<unsigned>( minimum( currentMappingPos.x - handleSize.x / 2, currentMappingPos.x + currentMappingSize.x / 2 - handleSize.x ),
                minimum( currentMappingPos.y - handleSize.y / 2, currentMappingPos.y + currentMappingSize.y / 2 - handleSize.y ) );

        handle[1] = Vector2<unsigned>( minimum( currentMappingPos.x - handleSize.x / 2, currentMappingPos.x + currentMappingSize.x / 2 - handleSize.x ),
                maximum( currentMappingPos.y + currentMappingSize.y - handleSize.y / 2, currentMappingPos.y + currentMappingSize.y / 2 ) );

        handle[2] = Vector2<unsigned>( maximum( currentMappingPos.x + currentMappingSize.x - handleSize.x / 2, currentMappingPos.x + currentMappingSize.x / 2 ),
                maximum( currentMappingPos.y + currentMappingSize.y - handleSize.y / 2, currentMappingPos.y + currentMappingSize.y / 2 ) );

        handle[3] = Vector2<unsigned>( maximum( currentMappingPos.x + currentMappingSize.x - handleSize.x / 2, currentMappingPos.x + currentMappingSize.x / 2 ),
                minimum( currentMappingPos.y - handleSize.y / 2, currentMappingPos.y + currentMappingSize.y / 2 - handleSize.y ) );
    }

    canvas->Refresh( false );
}
