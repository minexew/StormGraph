#ifndef TMEFRAME_H
#define TMEFRAME_H

//(*Headers(TmeFrame)
#include <wx/sizer.h>
#include <wx/listbox.h>
#include <wx/glcanvas.h>
#include <wx/aui/aui.h>
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "World.hpp"

extern IEngine* sg;
extern IGraphicsDriver* graphicsDriver;

struct Mapping
{
    String name;
    Vector2<float> uv[2];

    Text* text;
};

class TmeFrame: public wxDialog
{
	public:
		TmeFrame( World* world, Resources* res, wxGLContext* ctx, WorldNode* editNode, const String& initValue, wxWindow* parent,
                wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize );
		virtual ~TmeFrame();

        String getValue() const;

		//(*Declarations(TmeFrame)
		wxAuiManager* auiManager;
		wxPanel* Panel1;
		wxAuiNotebook* AuiNotebook1;
		wxButton* createNew;
		wxGLCanvas* canvas;
		wxListBox* lightMapList;
		//*)

	protected:
		//(*Identifiers(TmeFrame)
		static const long ID_GLCANVAS1;
		static const long ID_LISTBOX1;
		static const long ID_BUTTON1;
		static const long ID_PANEL1;
		static const long ID_AUINOTEBOOK1;
		//*)

	private:
        wxGLContext* ctx;
        WorldNode* editNode;
        int selection;

        Vector2<unsigned> canvasSize, area, grid, origin;
        unsigned scale;

        World* world;
        Resources* res;
        Reference<IFont> labelFont;

        List<Mapping> selectionMappings;
        Mapping currentMapping;

        Vector2<unsigned> handle[4], currentMappingPos, currentMappingSize, dragOrigin;
        int handleDrag;

        Object<wxWindowDisabler> windowDisabler;
        wxPropertyGrid* propertyGrid;

		//(*Handlers(TmeFrame)
		void onCanvasPaint( wxPaintEvent& event );
		void onCreateNew( wxCommandEvent& event );
		void onListSelect( wxCommandEvent& event );
		void onPropertyGridValueChanged( wxPropertyGridEvent& event );
		void onLmbDown(wxMouseEvent& event);
		void onLmbUp(wxMouseEvent& event);
		void onMouseMove(wxMouseEvent& event);
		void onCanvasResize(wxSizeEvent& event);
		//*)

        void addMappings( GroupWorldNode* node );
		void onSelect();
		bool parse( const String& lightMapping, Mapping& mapping );
		void updateSizes( bool all );

		DECLARE_EVENT_TABLE()
};

#endif
