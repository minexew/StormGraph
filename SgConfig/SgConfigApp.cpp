/***************************************************************
 * Name:      SgConfigApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Xeatheran Minexew ()
 * Created:   2011-04-27
 * Copyright: Xeatheran Minexew ()
 * License:
 **************************************************************/

#include "SgConfigApp.h"

//(*AppHeaders
#include "SgConfigMain.h"
#include <wx/image.h>
//*)

IMPLEMENT_APP(SgConfigApp);

bool SgConfigApp::OnInit()
{
    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	SgConfigDialog Dlg(0);
    	SetTopWindow(&Dlg);
    	Dlg.ShowModal();
    	wxsOK = false;
    }
    //*)
    return wxsOK;

}
