#ifndef NEWFRAME_H
#define NEWFRAME_H

//(*Headers(NewFrame)
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/splitter.h>
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/frame.h>
//*)

class NewFrame: public wxFrame
{
	public:

		NewFrame(wxWindow* parent,wxWindowID id=wxID_ANY);
		virtual ~NewFrame();

		//(*Declarations(NewFrame)
		wxTextCtrl* tex;
		wxButton* Button1;
		wxPanel* Panel1;
		wxButton* Button2;
		wxPanel* Panel2;
		wxSplitterWindow* SplitterWindow1;
		wxTextCtrl* cpp;
		//*)

	protected:

		//(*Identifiers(NewFrame)
		static const long ID_TEXTCTRL1;
		static const long ID_PANEL1;
		static const long ID_TEXTCTRL2;
		static const long ID_PANEL2;
		static const long ID_SPLITTERWINDOW1;
		//*)

	private:

		//(*Handlers(NewFrame)
		void OnRefresh(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
