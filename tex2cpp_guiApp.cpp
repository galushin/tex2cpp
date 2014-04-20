/***************************************************************
 * Name:      tex2cpp_guiApp.cpp
 * Purpose:   Code for Application Class
 * Author:     ()
 * Created:   2014-04-20
 * Copyright:  ()
 * License:
 **************************************************************/

#include "tex2cpp_guiApp.h"


#include "NewFrame.h"
#include <wx/image.h>

IMPLEMENT_APP(tex2cpp_guiApp);

bool tex2cpp_guiApp::OnInit()
{
    wxInitAllImageHandlers();
    auto frame = new NewFrame(nullptr);
    SetTopWindow(frame);
    frame->Show();
    return true;
}
