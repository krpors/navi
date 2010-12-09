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

    SetTopWindow(frame);

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
    m_tree->setBase(wxT("/"));
    m_tree->setFilesVisible(false);

    m_trackTable = new TrackTable(split);

    TagReader tr(wxT("file:///home/krpors/Desktop/mp3s/un.mp3"));
    TrackInfo ti = tr.getTrackInfo();
    std::cout << ti[TrackInfo::TITLE].mb_str() << std::endl;

    split->SplitVertically(m_tree, m_trackTable);

//    m_ogg = new OGGFilePipeline(wxT(""));
}

void NaviMainFrame::initMenu() {
    wxMenu* menuFile = new wxMenu;
    
    menuFile->Append(wxID_PREFERENCES, wxT("Preferences"));

    wxMenuBar* bar = new wxMenuBar;
    bar->Append(menuFile, wxT("&File"));
    
    SetMenuBar(bar);
}

void NaviMainFrame::play(wxListEvent& event) {
    long data = event.GetData();
    TrackInfo& info = m_trackTable->getTrackInfo(data);
    wxString loc = info.getLocation();
    loc.Replace(wxT("file://"), wxEmptyString); 
    std::cout << "Selected track: " << info[TrackInfo::TITLE].mb_str() << ", path is " << loc.mb_str() << std::endl;

    delete m_ogg;
    m_ogg = new OGGFilePipeline(loc);
    m_ogg->play();

}

void NaviMainFrame::dostuff(wxTreeEvent& event) {
    const wxFileName& selectedPath = m_tree->getSelectedPath();
    std::cout << selectedPath.GetFullPath().mb_str() << std::endl;
    m_trackTable->addFromDir(selectedPath);
}

// Event table.
BEGIN_EVENT_TABLE(NaviMainFrame, wxFrame)
    EVT_TREE_ITEM_ACTIVATED(FileTree::ID_TREE, NaviMainFrame::dostuff)
    EVT_LIST_ITEM_ACTIVATED(TrackTable::ID_TRACKTABLE, NaviMainFrame::play)
END_EVENT_TABLE()


} // namespace navi 


// This is main() replacement.
IMPLEMENT_APP(navi::NaviApp)
