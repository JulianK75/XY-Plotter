/***************************************************************
 * Name:      GUI_TestApp.cpp
 * Purpose:   Code for Application Class
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

#include "GUI_TestApp.h"
#include "GUI_TestMain.h"

IMPLEMENT_APP(GUI_TestApp);

bool GUI_TestApp::OnInit()
{
    GUI_TestFrame* frame = new GUI_TestFrame(0L, _("wxWidgets Application Template"));
    
    frame->Show();
    
    return true;
}
