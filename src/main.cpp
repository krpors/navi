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
    wxInitAllImageHandlers();

    NaviMainFrame* frame = new NaviMainFrame;
    frame->SetSize(800, 600);
    frame->Center();
    frame->Show();
    
    SetTopWindow(frame);

    return true;
}

//==============================================================================

NaviMainFrame::NaviMainFrame() :
        wxFrame((wxFrame*) NULL, wxID_ANY, wxT("Navi")),
        m_ogg(NULL),
        m_noteBook(NULL),
        m_updateThread(NULL) {

    // XXX: images are now just for demonstration purposes.
    // attempt to initialize image lists:
    m_imageList = new wxImageList(16, 16);
    m_imageList->Add(wxArtProvider::GetIcon(wxT("gtk-open")));
    m_imageList->Add(wxArtProvider::GetIcon(wxART_ADD_BOOKMARK));
    m_imageList->Add(wxArtProvider::GetIcon(wxT("gtk-network")));

    // create our menu here
    initMenu();
    wxSplitterWindow* split = new wxSplitterWindow(this, wxID_ANY);

    m_noteBook = new wxNotebook(split, wxID_ANY);
    m_noteBook->AssignImageList(m_imageList);

    m_dirBrowser = new DirBrowserContainer(m_noteBook);
    m_dirBrowser->getDirBrowser()->setBase(wxT("/"));
    m_dirBrowser->getDirBrowser()->setFilesVisible(false);

    m_noteBook->AddPage(m_dirBrowser, wxT("Browser"), true, 0);
    m_noteBook->AddPage(new wxButton(m_noteBook, wxID_ANY, wxT("Cocks?")), wxT("Favorites"), false, 1);
    m_noteBook->AddPage(new wxButton(m_noteBook, wxID_ANY, wxT("Cocks?")), wxT("Streams"), false, 2);

    m_trackTable = new TrackTable(split);
    split->SplitVertically(m_noteBook, m_trackTable);

    CreateStatusBar();

}

void NaviMainFrame::initMenu() {
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(wxID_PREFERENCES, wxT("&Preferences"));

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT, wxT("&About"));

    wxMenuBar* bar = new wxMenuBar;
    bar->Append(menuFile, wxT("&File"));
    bar->Append(menuHelp, wxT("&Help"));
    bar->SetHelpString(wxID_ABOUT, wxT("About Navi"));
    
    SetMenuBar(bar);
}

void NaviMainFrame::play(wxListEvent& event) {
    long data = event.GetData();
    TrackInfo& info = m_trackTable->getTrackInfo(data);
    wxString loc = info.getLocation();
    loc.Replace(wxT("file://"), wxEmptyString); 
    SetStatusText(info[TrackInfo::TITLE]);

    delete m_ogg;
    m_ogg = new OGGFilePipeline(loc);
    m_ogg->play();
}

wxStatusBar* NaviMainFrame::OnCreateStatusBar(int number, long style, wxWindowID id, const wxString& name) {
    wxStatusBar* bar = new wxStatusBar(this, id, style, name);
    bar->SetStatusText(wxT("Navi started. Hey, listen!"));

    return bar;
}

void NaviMainFrame::onResize(wxSizeEvent& event) {
    Refresh();
    event.Skip();
}

void NaviMainFrame::dostuff(wxTreeEvent& event) {
    const wxFileName& selectedPath = m_dirBrowser->getDirBrowser()->getSelectedPath();
    std::cout << selectedPath.GetFullPath().mb_str() << std::endl;
    
    if (m_updateThread != NULL) {
        m_updateThread->setActive(false);
        // wait for thread to finish doing its work.
        /*wxThread::ExitCode code = */
        m_updateThread->Wait();
    }
    // XXX: deleting all items does not seem to work reliably, i.e. always some
    // 'residue' seem to be left behind from the previous directory crap.
    m_trackTable->DeleteAllItems();

    m_updateThread = new UpdateThread(m_trackTable, selectedPath);
    wxThreadError err = m_updateThread->Create();
    if (err != wxTHREAD_NO_ERROR) {
        wxMessageBox(wxT("Couldn't create thread!"));
    }

    err = m_updateThread->Run();

    if (err != wxTHREAD_NO_ERROR) {
        wxMessageBox(wxT("Couldn't run thread!"));
    }
}

void NaviMainFrame::onAbout(wxCommandEvent& event) {
    wxAboutDialogInfo info;
    info.SetName(wxT("Navi"));
    info.SetVersion(wxT("0.1a"));
    info.SetDescription(wxT("Hey, listen! Hey, listen!\nNavi is a directory based music player for Linux."));
    info.AddArtist(wxT("James 'adamorjames' Corley"));
    info.AddDeveloper(wxT("Kevin 'Azzkikr' Pors"));
    info.SetWebSite(wxT("http://github.com/krpors/navi"));

    wxBitmap bm(wxT("./data/icons/navi.png"), wxBITMAP_TYPE_PNG);
    wxIcon icon;
    icon.CopyFromBitmap(bm);
    info.SetIcon(icon);

    wxAboutBox(info);
}

// Event table.
BEGIN_EVENT_TABLE(NaviMainFrame, wxFrame)
    EVT_SIZE(NaviMainFrame::onResize)
    EVT_TREE_ITEM_ACTIVATED(DirBrowser::ID_NAVI_DIR_BROWSER, NaviMainFrame::dostuff)
    EVT_LIST_ITEM_ACTIVATED(TrackTable::ID_TRACKTABLE, NaviMainFrame::play)
    EVT_LIST_ITEM_ACTIVATED(TrackTable::ID_TRACKTABLE, NaviMainFrame::play)
    EVT_MENU(wxID_ABOUT, NaviMainFrame::onAbout)
END_EVENT_TABLE()


} // namespace navi 


// This is main() replacement.
IMPLEMENT_APP(navi::NaviApp)
