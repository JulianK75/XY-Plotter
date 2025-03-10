/***************************************************************
 * Name:      GUI_TestApp.h
 * Purpose:   Defines Application Class
 * Author:    Julian Kummer ()
 * Created:   2025-03-04
 * Copyright: Julian Kummer ()
 * License:
 **************************************************************/

#ifndef GUI_TESTAPP_H
#define GUI_TESTAPP_H

#include <wx/app.h>

class GUI_TestApp : public wxApp
{
    public:
        virtual bool OnInit();
};

#endif // GUI_TESTAPP_H
