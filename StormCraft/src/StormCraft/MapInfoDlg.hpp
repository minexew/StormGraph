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

#pragma once

//(*Headers(MapInfoDlg)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include "World.hpp"

class MapInfoDlg: public wxDialog
{
	public:
		MapInfoDlg( World* world, wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize );
		virtual ~MapInfoDlg();

		//(*Declarations(MapInfoDlg)
		wxTextCtrl* displayNameCtrl;
		wxButton* Button1_;
		wxStaticText* StaticText1;
		wxButton* Button2_;
		//*)

	protected:
		//(*Identifiers(MapInfoDlg)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:
		//(*Handlers(MapInfoDlg)
		void onOk(wxCommandEvent& event);
		void onCancel(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

		World* world;
};
