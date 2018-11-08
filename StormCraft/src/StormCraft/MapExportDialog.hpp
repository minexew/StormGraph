#pragma once

//(*Headers(MapExportDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include "World.hpp"

class MapExportDialog: public wxDialog
{
	public:
		MapExportDialog( wxWindow* parent, World* world, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize );
		virtual ~MapExportDialog();

		//(*Declarations(MapExportDialog)
		wxStaticText* StaticText10;
		wxStaticText* StaticText9;
		wxButton* browseButton;
		wxTextCtrl* pathEdit;
		wxTextCtrl* bspVolumeLimit;
		wxSpinCtrl* ctree2SegLimit;
		wxCheckBox* generateLightmapsCheckBox;
		wxCheckBox* generateCtree2;
		wxStaticText* StaticText2;
		wxStaticText* StaticText6;
		wxCheckBox* cloneFsCheckBox;
		wxStaticText* StaticText8;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxStaticLine* StaticLine4;
		wxStaticLine* StaticLine2;
		wxSpinCtrl* bspPolygonLimit;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxCheckBox* includeGeometryCheckBox;
		wxStaticLine* StaticLine3;
		wxStaticLine* StaticLine1;
		wxButton* cancel;
		wxStaticText* StaticText4;
		wxButton* ok;
		wxStaticLine* StaticLine5;
		//*)

	protected:
		//(*Identifiers(MapExportDialog)
		static const long ID_TEXTCTRL1;
		static const long ID_BUTTON1;
		static const long ID_STATICLINE1;
		static const long ID_STATICTEXT1;
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
		static const long ID_STATICLINE2;
		static const long ID_STATICTEXT2;
		static const long ID_STATICTEXT3;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICLINE3;
		static const long ID_STATICTEXT7;
		static const long ID_CHECKBOX4;
		static const long ID_STATICTEXT9;
		static const long ID_STATICTEXT8;
		static const long ID_SPINCTRL2;
		static const long ID_STATICLINE5;
		static const long ID_STATICTEXT6;
		static const long ID_CHECKBOX3;
		static const long ID_STATICLINE4;
		static const long ID_BUTTON2;
		static const long ID_BUTTON3;
		//*)

	private:
		//(*Handlers(MapExportDialog)
		void onOk(wxCommandEvent& event);
		void onCancel(wxCommandEvent& event);
		void onBrowse(wxCommandEvent& event);
		//*)

        World* world;

		DECLARE_EVENT_TABLE()
};
