//      main.cpp
//      
//      Copyright 2010 Kevin Pors <krpors@users.sf.net>
//      
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//      
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//      
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.

#include "main.hpp"

#include <wx/splitter.h>
#include <iostream>

namespace navi {

bool NaviApp::OnInit() {
    gst_init(NULL, NULL);

    NaviMainFrame* frame = new NaviMainFrame;
    frame->Center();
    frame->Show();

    return true;
}



//==============================================================================

NaviMainFrame::NaviMainFrame() :
        wxFrame((wxFrame*) NULL, wxID_ANY, wxT("Navi")),
        m_ogg(NULL) {

    // create our menu here
    initMenu();
    wxSplitterWindow* split = new wxSplitterWindow(this, wxID_ANY);

    m_tree = new FileTree(split);
    m_tree->setBase(wxT("/home/krpors/Desktop/")); 
    m_tree->setFilesVisible(false);

    m_tracks = new TrackTable(split);

    std::cout << "Before" << std::endl;
    try {
        for(int i = 0; i < 100; i++) {
            std::cout << "Creating tagreader" << std::endl;
            TagReader t(wxT("file:///home/krpors/Desktop/oggs/bb.ogg"));
        }
    } catch (AudioException& ex) {
        std::cout << "Exception caught" << std::endl;
    }
    std::cout << "After" << std::endl;

    split->SplitVertically(m_tree, m_tracks);
}

void NaviMainFrame::initMenu() {
    wxMenu* menuFile = new wxMenu;
    
    menuFile->Append(wxID_PREFERENCES, wxT("Preferences"));

    wxMenuBar* bar = new wxMenuBar;
    bar->Append(menuFile, wxT("&File"));
    
    SetMenuBar(bar);
}

void NaviMainFrame::play(wxCommandEvent& event) {
    wxString path = m_tree->getSelectedPath().GetFullPath();
    if (path.EndsWith(wxT(".ogg"))) {
        if (m_ogg == NULL) {
            m_ogg = new OGGFilePipeline(path);
            m_ogg->play();
        } else {
            m_ogg->stop();
            delete m_ogg;
            m_ogg = new OGGFilePipeline(path);
            m_ogg->play();

        }
    }
}

// Event table.
BEGIN_EVENT_TABLE(NaviMainFrame, wxFrame)
    EVT_BUTTON(wxID_ANY, NaviMainFrame::play)
END_EVENT_TABLE()


} // namespace navi 


// This is main() replacement.
IMPLEMENT_APP(navi::NaviApp)
