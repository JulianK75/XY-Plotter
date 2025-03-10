/***************************************************************
 * Name:      GUI_TestMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Julian Kummer ()
 * Created:   2025-03-04
 * Copyright: Julian Kummer ()
 * License:
 **************************************************************/

#ifdef WX_PRECOMP
#include "wx_pch.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "GUI_TestMain.h"

//helper functions
enum wxbuildinfoformat {
    short_f, long_f };

wxString wxbuildinfo(wxbuildinfoformat format)
{
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__WXMAC__)
        wxbuild << _T("-Mac");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}

BEGIN_EVENT_TABLE(GUI_TestFrame, wxFrame)
    EVT_CLOSE(GUI_TestFrame::OnClose)
    EVT_MENU(idMenuQuit, GUI_TestFrame::OnQuit)
    EVT_MENU(idMenuAbout, GUI_TestFrame::OnAbout)
    EVT_MOTION(GUI_TestFrame::OnMotion)
END_EVENT_TABLE()

GUI_TestFrame::GUI_TestFrame(wxFrame *frame, const wxString& title)
    : wxFrame(frame, -1, title)
{
#if wxUSE_MENUS
    // create a menu bar
    wxMenuBar* mbar = new wxMenuBar();
    wxMenu* fileMenu = new wxMenu(_T(""));
    fileMenu->Append(idMenuQuit, _("&Quit\tAlt-F4"), _("Quit the application"));
    mbar->Append(fileMenu, _("&File"));

    wxMenu* helpMenu = new wxMenu(_T(""));
    helpMenu->Append(idMenuAbout, _("&About\tF1"), _("Show info about this application"));
    mbar->Append(helpMenu, _("&Help"));

    SetMenuBar(mbar);
#endif // wxUSE_MENUS

#if wxUSE_STATUSBAR
    // create a status bar with some information about the used wxWidgets version
    CreateStatusBar(2);
    SetStatusText(_("Hello Code::Blocks user!"),0);
    SetStatusText(wxbuildinfo(short_f), 1);
#endif // wxUSE_STATUSBAR

#if wxUSE_BUTTON
    wxButton* button = new wxButton(this, wxID_OK, wxT("OK"), wxPoint(200, 200));
#endif // wxUSE_BUTTON

}


GUI_TestFrame::~GUI_TestFrame()
{
}

void GUI_TestFrame::OnClose(wxCloseEvent &event)
{
    Destroy();
}

void GUI_TestFrame::OnQuit(wxCommandEvent &event)
{
    Destroy();
}

void GUI_TestFrame::OnAbout(wxCommandEvent &event)
{
    wxString msg = wxbuildinfo(long_f);
    wxMessageBox(msg, _("Welcome to..."));
}

void GUI_TestFrame::OnMotion(wxMouseEvent &event)
{
    if (event.Dragging())
    {
        wxClientDC dc(this);
        wxPen pen(*wxRED, 1); // red pen of width 1
        dc.SetPen(pen);
        dc.DrawPoint(event.GetPosition());
        dc.SetPen(wxNullPen);
    }
}
