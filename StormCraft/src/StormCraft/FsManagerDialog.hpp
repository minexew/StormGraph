#pragma once

//(*Headers(FsManagerDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/listbox.h>
#include <wx/statline.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include "World.hpp"

#include <StormGraph/Engine.hpp>

using namespace StormGraph;

class FsManagerDialog: public wxDialog
{
	public:
		FsManagerDialog( World* world, wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize );
		virtual ~FsManagerDialog();

		//(*Declarations(FsManagerDialog)
		wxListBox* fsList;
		wxCheckBox* cloneCheckBox;
		wxTextCtrl* pathEdit;
		wxButton* Button4_;
		wxStaticText* StaticText2;
		wxButton* Button1_;
		wxStaticText* StaticText1;
		wxButton* Button2_;
		wxButton* Button6;
		wxButton* Button5_;
		wxButton* Button3_;
		wxStaticLine* StaticLine1;
		wxChoice* fsDriverList;
		//*)

	protected:
		//(*Identifiers(FsManagerDialog)
		static const long ID_LISTBOX1;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL1;
		static const long ID_BUTTON1;
		static const long ID_CHECKBOX1;
		static const long ID_BUTTON2;
		static const long ID_BUTTON6;
		static const long ID_BUTTON3;
		static const long ID_STATICLINE1;
		static const long ID_BUTTON4;
		static const long ID_BUTTON5;
		//*)

	private:
		//(*Handlers(FsManagerDialog)
		void onCancel(wxCommandEvent& event);
		void onFsListSelect(wxCommandEvent& event);
		void onOk(wxCommandEvent& event);
		void onAdd(wxCommandEvent& event);
		void onBrowse(wxCommandEvent& event);
		void onRemove(wxCommandEvent& event);
		void onEdit(wxCommandEvent& event);
		//*)

        World* world;

        struct DriverInfo
        {
            String protocol;
            IFileSystemDriver::Info info;
        };

        List<DriverInfo> drivers;

		DECLARE_EVENT_TABLE()
};
