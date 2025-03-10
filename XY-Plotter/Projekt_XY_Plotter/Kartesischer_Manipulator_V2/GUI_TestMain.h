/***************************************************************
 * Name:      GUI_TestMain.h
 * Purpose:   Defines Application Frame
 * Author:    Julian Kummer ()
 * Created:   2025-03-04
 * Copyright: Julian Kummer ()
 * License:
 **************************************************************/

#ifndef GUI_TESTMAIN_H
#define GUI_TESTMAIN_H

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "GUI_TestApp.h"

class GUI_TestFrame: public wxFrame
{
    public:
        GUI_TestFrame(wxFrame *frame, const wxString& title);
        ~GUI_TestFrame();
        void OnMotion(wxMouseEvent& event);
    private:
        enum
        {
            idMenuQuit = 1000,
            idMenuAbout
        };
        void OnClose(wxCloseEvent& event);
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        DECLARE_EVENT_TABLE()
};


#endif // GUI_TESTMAIN_H
