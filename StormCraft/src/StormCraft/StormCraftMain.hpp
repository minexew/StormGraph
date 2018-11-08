#pragma once

//(*Headers(StormCraftFrame)
#include <wx/treectrl.h>
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/glcanvas.h>
#include <wx/aui/aui.h>
#include <wx/toolbar.h>
#include <wx/panel.h>
#include <wx/frame.h>
#include <wx/statusbr.h>
//*)

#include <wx/html/htmprint.h>
#include <wx/html/htmlwin.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include <StormGraph/Engine.hpp>
#include <StormGraph/ResourceManager.hpp>

#include "World.hpp"

using namespace StormGraph;

extern IEngine* sg;
extern IGraphicsDriver* graphicsDriver;

class WorldNodeTreeItemData : public wxTreeItemData
{
    friend class StormCraftFrame;

    WorldNodeTreeItemData* parent;
    WorldNode* node;

    public:
        WorldNodeTreeItemData( WorldNodeTreeItemData* parent, WorldNode* node ) : parent( parent ), node( node ) {}
        virtual ~WorldNodeTreeItemData() {}
};

class LightMapAreaProperty;

class LightMapAreaDialogAdapter : public wxPGEditorDialogAdapter
{
    StormCraftFrame* frame;
    World* world;
    Resources* res;
    WorldNode* editNode;
    String initValue;

    public:
        LightMapAreaDialogAdapter( StormCraftFrame* frame, World* world, Resources* res, WorldNode* editNode, const char* initValue );

        virtual bool DoShowDialog( wxPropertyGrid* propGrid, wxPGProperty* property );
};

class LightMapAreaProperty : public wxPGProperty
{
    StormCraftFrame* frame;
    World* world;
    Resources* res;
    WorldNode* editNode;

    WX_PG_DECLARE_DOGETEDITORCLASS

    public:
        LightMapAreaProperty( StormCraftFrame* frame, World* world, Resources* res, WorldNode* editNode,
                const wxString& label, const wxString& name, const wxString& value );
        virtual ~LightMapAreaProperty();

        virtual wxString ValueToString( wxVariant& value, int argFlags = 0 ) const;
        virtual bool StringToValue( wxVariant& variant, const wxString& text, int argFlags = 0 ) const;
        virtual wxPGEditorDialogAdapter* GetEditorDialog() const;
};

class StormCraftFrame: public wxFrame, public IGuiEventListener
{
    static StormCraftFrame* instance;

    public:
        StormCraftFrame( wxWindow* parent, wxWindowID id = -1 );
        virtual ~StormCraftFrame();

    private:
        //(*Handlers(StormCraftFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void onCanvasPaint(wxPaintEvent& event);
        void onFileNew(wxCommandEvent& event);
        void onLmbDown(wxMouseEvent& event);
        void onLmbUp(wxMouseEvent& event);
        void onMouseMove(wxMouseEvent& event);
        void onMouseWheel(wxMouseEvent& event);
        void onWorldExplorerTreeItemMenu(wxTreeEvent& event);
        void onAddLight(wxCommandEvent& event);
        void onWorldExplorerTreeSelectionChanged(wxTreeEvent& event);
        void onCanvasMouseEnter(wxMouseEvent& event);
        void onAddGroup(wxCommandEvent& event);
        void onConfigureFilesystems(wxCommandEvent& event);
        void onFileOpen(wxCommandEvent& event);
        void onFileSave(wxCommandEvent& event);
        void onAddTerrain(wxCommandEvent& event);
        void onMmbDown( wxMouseEvent& event );
        void onMmbUp( wxMouseEvent& event );
        void onRmbDown( wxMouseEvent& event );
        void onRmbUp( wxMouseEvent& event );
        void onMultipleViewsToggle(wxCommandEvent& event);
        void onAddCuboid(wxCommandEvent& event);
        void onMapExport(wxCommandEvent& event);
        void onBreakIntoFaces(wxCommandEvent& event);
        void onEditLightMapping(wxCommandEvent& event);
        void onAddFace(wxCommandEvent& event);
        void onAddStaticMesh(wxCommandEvent& event);
        void onWorldExplorerTreeItemRenamed(wxTreeEvent& event);
        void onWorldExplorerTreeKeyDown(wxTreeEvent& event);
        void onAddPolygon(wxCommandEvent& event);
        void onDuplicateObject(wxCommandEvent& event);
        void onWorldExplorerTreeBeginDrag(wxTreeEvent& event);
        void onWorldExplorerTreeEndDrag(wxTreeEvent& event);
        void onEditMapInfo(wxCommandEvent& event);
        void onRotateZ(wxCommandEvent& event);
        //*)

        void onPropertyGridValueChanged( wxPropertyGridEvent& event );

        //(*Identifiers(StormCraftFrame)
        static const long ID_GLCANVAS1;
        static const long ID_TREECTRL1;
        static const long ID_PANEL1;
        static const long ID_AUINOTEBOOK1;
        static const long ID_TREECTRL2;
        static const long ID_PANEL2;
        static const long ID_AUINOTEBOOK2;
        static const long idMenuNew;
        static const long ID_MENUITEM4;
        static const long ID_MENUITEM5;
        static const long idMenuQuit;
        static const long ID_MENUITEM8;
        static const long ID_MENUITEM12;
        static const long ID_MENUITEM14;
        static const long ID_MENUITEM13;
        static const long ID_MENUITEM7;
        static const long idMenuAbout;
        static const long ID_STATUSBAR1;
        static const long ID_TOOLBARITEM1;
        static const long ID_TOOLBARITEM3;
        static const long ID_TOOLBARITEM4;
        static const long ID_TOOLBARITEM2;
        static const long ID_TOOLBARITEM6;
        static const long ID_TOOLBARITEM5;
        static const long ID_TOOLBAR1;
        static const long ID_MENUITEM2;
        static const long ID_MENUITEM6;
        static const long ID_MENUITEM9;
        static const long ID_MENUITEM1;
        static const long ID_MENUITEM11;
        static const long ID_MENUITEM10;
        static const long ID_MENUITEM3;
        //*)

        //(*Declarations(StormCraftFrame)
        wxToolBarToolBase* ToolBarItem4;
        wxMenuItem* addLightMenuItem;
        wxMenuItem* MenuItem8;
        wxAuiNotebook* AuiNotebook2;
        wxMenuItem* MenuItem7;
        wxToolBarToolBase* ToolBarItem3;
        wxMenuItem* MenuItem5;
        wxToolBar* toolBar;
        wxMenu* Menu3;
        wxMenuItem* MenuItem4;
        wxTreeCtrl* worldExplorerTree;
        wxAuiManager* auiManager;
        wxMenuItem* MenuItem11;
        wxPanel* Panel1;
        wxMenuItem* MenuItem13;
        wxMenuItem* MenuItem10;
        wxToolBarToolBase* ToolBarItem6;
        wxMenuItem* MenuItem12;
        wxToolBarToolBase* ToolBarItem1;
        wxMenuItem* addGroupMenuItem;
        wxMenuItem* addTerrainMenuItem;
        wxMenuItem* MenuItem6;
        wxTreeCtrl* TreeCtrl1;
        wxStatusBar* statusBar;
        wxAuiNotebook* AuiNotebook1;
        wxMenuItem* addCuboidMenuItem;
        wxToolBarToolBase* ToolBarItem5;
        wxGLCanvas* canvas;
        wxPanel* Panel2;
        wxMenuItem* MenuItem9;
        wxToolBarToolBase* ToolBarItem2;
        wxMenu* Menu4;
        wxMenu groupWorldNodeItemMenu;
        //*)

        public:
            List<Property> currentNodeProperties;
            wxPropertyGrid* propertyGrid;

            wxHtmlWindow* eventLog;

            wxGLContext* ctx;
            bool multipleViews;

            bool gizmoVisible[6];
            unsigned gizmoId[6];

            bool lmbDown, mmbDown, rmbDown;
            Vector2<int> mouseMoveOrigin;

            Vector<> drag, dragBuffer;
            unsigned dragView, menuView, currentViewport;
            Object<IProjectionInfoBuffer> projectionBuffer;

            Object<World> world;
            Object<Resources> res;

            Transform gizmoTransforms[2];

            WorldNodeTreeItemData *selectedItemData, *dragItemData;

        public:
            wxTreeItemId addWorldExplorerTreeNode( WorldNodeTreeItemData* parent, WorldNode* node, bool select, bool editLabel );
            void buildPropertyPane();
            Vector<> getCameraScale() const;
            unsigned getViewportAt( const Vector2<unsigned>& pos );
            void loadResources();
            static void logEvent( const char* className, const char* event );
            virtual void onGuiMenuCommand( const MenuCommandEvent& event ) override;
            virtual void onGuiMenuToggle( const MenuToggleEvent& event ) override;
            void onWorldOpen();
            unsigned renderCanvas( bool picking, const Vector2<unsigned>& mouse );
            void renderScene( bool picking, unsigned view );
            void rescanDirectories();
            void save();
            void selectCamera( unsigned view );
            void showEventLog();
            void showGizmo( const Vector<>& pos, WorldNode* node, const bool visible[3] );
            void updateProperties( const List<Property>& propertyList );
            void updateProperty( const Property& property );

        DECLARE_EVENT_TABLE()
};
