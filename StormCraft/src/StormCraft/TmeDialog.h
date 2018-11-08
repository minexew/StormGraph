#ifndef TMEDIALOG_H
#define TMEDIALOG_H

//(*Headers(TmeDialog)
#include <wx/dialog.h>
//*)

class TmeDialog: public wxDialog
{
	public:

		TmeDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~TmeDialog();

		//(*Declarations(TmeDialog)
		//*)

	protected:

		//(*Identifiers(TmeDialog)
		//*)

	private:

		//(*Handlers(TmeDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
