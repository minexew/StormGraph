/***************************************************************
 * Name:      SgConfigMain.h
 * Purpose:   Defines Application Frame
 * Author:    Xeatheran Minexew ()
 * Created:   2011-04-27
 * Copyright: Xeatheran Minexew ()
 * License:
 **************************************************************/

#ifndef SGCONFIGMAIN_H
#define SGCONFIGMAIN_H

//(*Headers(SgConfigDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/listbox.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/combobox.h>
//*)

#include <confix2.h>

class SgConfigDialog: public wxDialog
{
    public:

        SgConfigDialog(wxWindow* parent,wxWindowID id = -1);
        virtual ~SgConfigDialog();

    private:
        wxString currentApp, executable;
        bool devMode;

        //(*Handlers(SgConfigDialog)
        void OnQuit(wxCommandEvent& event);
        void OnInit(wxInitDialogEvent& event);
        void onAppListSelect(wxCommandEvent& event);
        void onDefaultsButtonClick(wxCommandEvent& event);
        void onSaveButtonClick(wxCommandEvent& event);
        void onRunButtonClick(wxCommandEvent& event);
        void onDriverParamsInfoClick(wxCommandEvent& event);
        //*)

        wxString getDriverParamsInfo( const char* driverName );
        int getInt( cfx2_Node* node, const char* name, bool defaults );
        const char* getString( cfx2_Node* node, const char* name, bool defaults );
        void loadDriverList();
        void loadSettings( bool defaults );

        //(*Identifiers(SgConfigDialog)
        static const long ID_STATICTEXT1;
        static const long ID_LISTBOX1;
        static const long ID_STATICTEXT12;
        static const long ID_STATICTEXT2;
        static const long ID_COMBO1;
        static const long ID_STATICTEXT15;
        static const long ID_TEXTCTRL3;
        static const long ID_BUTTON4;
        static const long ID_STATICTEXT3;
        static const long ID_TEXTCTRL1;
        static const long ID_STATICTEXT4;
        static const long ID_TEXTCTRL2;
        static const long ID_STATICTEXT5;
        static const long ID_STATICTEXT6;
        static const long ID_CHECKBOX1;
        static const long ID_STATICTEXT7;
        static const long ID_CHECKBOX2;
        static const long ID_STATICTEXT8;
        static const long ID_CHOICE2;
        static const long ID_STATICTEXT10;
        static const long ID_STATICTEXT9;
        static const long ID_CHOICE1;
        static const long ID_STATICTEXT11;
        static const long ID_STATICTEXT14;
        static const long ID_STATICTEXT13;
        static const long ID_CHECKBOX3;
        static const long ID_BUTTON2;
        static const long ID_BUTTON1;
        static const long ID_BUTTON3;
        //*)

        //(*Declarations(SgConfigDialog)
        wxStaticText* StaticText10;
        wxStaticText* StaticText9;
        wxCheckBox* fullscreenCheckBox;
        wxComboBox* driverList;
        wxStaticText* developerOptionsStatus;
        wxStaticText* StaticText13;
        wxStaticText* StaticText2;
        wxStaticText* StaticText14;
        wxStaticText* StaticText6;
        wxStaticText* StaticText8;
        wxStaticText* StaticText11;
        wxListBox* appList;
        wxButton* saveButton;
        wxChoice* aaChoice;
        wxFlexGridSizer* FlexGridSizer2;
        wxCheckBox* showConsoleCheckBox;
        wxStaticText* StaticText1;
        wxButton* defaultsButton;
        wxStaticText* StaticText3;
        wxButton* driverParamsInfo;
        wxButton* runButton;
        wxSpinCtrl* displayXEdit;
        wxStaticText* StaticText5;
        wxStaticText* StaticText7;
        wxChoice* textureLodChoice;
        wxStaticText* StaticText12;
        wxCheckBox* vsyncCheckBox;
        wxFlexGridSizer* FlexGridSizer1;
        wxStaticText* StaticText4;
        wxSpinCtrl* displayYEdit;
        wxTextCtrl* driverParams;
        //*)

        DECLARE_EVENT_TABLE()
};

#endif // SGCONFIGMAIN_H
