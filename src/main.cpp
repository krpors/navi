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

const wxString secsToMins(int secs) {
    wxString s;
    int mins = secs / 60;
    int leftover = secs % 60;

    if (mins <= 9) {
        s.Append(wxString::Format(wxT("%i"), mins).Pad(1, '0', false));
    } else {
        s.Append(wxString::Format(wxT("%i"), mins));
    }
    
    s.Append(wxT(":"));

    if (leftover <= 9) {
        s.Append(wxString::Format(wxT("%i"), leftover).Pad(1, '0', false));
    } else {
        s.Append(wxString::Format(wxT("%i"), leftover));
    }

    return s;
}

bool NaviApp::OnInit() {
    gst_init(NULL, NULL);
    wxInitAllImageHandlers();

    NaviMainFrame* frame = new NaviMainFrame;
    frame->SetSize(800, 600);
    frame->Center();
    frame->Show();
    SetTopWindow(frame);

    std::cout << secsToMins(640).mb_str() << std::endl;
    std::cout << secsToMins(530).mb_str() << std::endl;
    std::cout << secsToMins(552).mb_str() << std::endl;
    std::cout << secsToMins(123).mb_str() << std::endl;

    //http://scfire-dtc-aa01.stream.aol.com:80/stream/1025
    //m_p = new GenericPipeline(wxT("http://scfire-dtc-aa01.stream.aol.com:80/stream/1025"));
    //m_p->play();
    return true;
}

//==============================================================================

NaviMainFrame::NaviMainFrame() :
        wxFrame((wxFrame*) NULL, wxID_ANY, wxT("Navi")),
        m_noteBook(NULL),
        m_dirTraversalThread(NULL) {

    // XXX: images are now just for demonstration purposes.
    // attempt to initialize image lists:
    m_imageList = new wxImageList(16, 16);
    m_imageList->Add(wxArtProvider::GetIcon(wxART_HARDDISK));
    m_imageList->Add(wxArtProvider::GetIcon(wxART_ADD_BOOKMARK));
    m_imageList->Add(wxArtProvider::GetIcon(wxT("gtk-network")));

    // create our menu here
    initMenu();
    wxSplitterWindow* split = new wxSplitterWindow(this, wxID_ANY);

    m_noteBook = new wxNotebook(split, wxID_ANY);
    m_noteBook->AssignImageList(m_imageList);

    m_dirBrowser = new DirBrowserContainer(m_noteBook);
    m_dirBrowser->getDirBrowser()->setBase(wxT("/home/krpors/Desktop"));
    m_dirBrowser->getDirBrowser()->setFilesVisible(false);

    m_noteBook->AddPage(m_dirBrowser, wxT("Browser"), true, 0);
    m_noteBook->AddPage(new wxButton(m_noteBook, wxID_ANY, wxT("template")), wxT("Favorites"), false, 1);
    m_noteBook->AddPage(new wxButton(m_noteBook, wxID_ANY, wxT("template")), wxT("Streams"), false, 2);

    wxPanel* lol = createNavPanel(split);
    // prevent 'unsplitting', i.e. when double clicking, this would make the
    // splitter dissapear.
    split->SetMinimumPaneSize(20);
    split->SplitVertically(m_noteBook, lol);

    CreateStatusBar();

    // create the track status handler event handling stuff. This thing
    // is created on the heap, without a parent wxWindow. this pointer is
    // given still though, but by destroying this frame, the trackstatushandler
    // instance will not be destroyed automatically. Not that it matters,
    // since after we destroyed this frame, the program should end anyway.
    m_trackStatusHandler = new TrackStatusHandler(this);
    // Push that event handler, otherwise events will not be propagated to
    // this new track status handler.
    PushEventHandler(m_trackStatusHandler);
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

wxPanel* NaviMainFrame::createNavPanel(wxWindow* parent) {
    wxPanel* panel = new wxPanel(parent);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(sizer);

    m_navigation = new NavigationContainer(panel, this);
    m_trackTable = new TrackTable(panel);

    sizer->Add(m_navigation, wxSizerFlags().Expand().Border(5));
    sizer->Add(m_trackTable, wxSizerFlags(1).Expand().Border(5));

    return panel;
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
    
    if (m_dirTraversalThread != NULL) {
        m_dirTraversalThread->setActive(false);
        // wait for thread to finish doing its work.
        /*wxThread::ExitCode code = */
        m_dirTraversalThread->Wait();
    }
    // XXX: deleting all items does not seem to work reliably, i.e. always some
    // 'residue' seem to be left behind from the previous directory crap.
    m_trackTable->DeleteAllItems();

    m_dirTraversalThread = new DirTraversalThread(m_trackTable, selectedPath);
    wxThreadError err = m_dirTraversalThread->Create();
    if (err != wxTHREAD_NO_ERROR) {
        wxMessageBox(wxT("Couldn't create thread!"));
    }

    err = m_dirTraversalThread->Run();

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

TrackTable* NaviMainFrame::getTrackTable() const {
    return m_trackTable;
}

NavigationContainer* NaviMainFrame::getNavigationContainer() const {
    return m_navigation;
}

// Event table.
BEGIN_EVENT_TABLE(NaviMainFrame, wxFrame)
    EVT_SIZE(NaviMainFrame::onResize)
    EVT_TREE_ITEM_ACTIVATED(DirBrowser::ID_NAVI_DIR_BROWSER, NaviMainFrame::dostuff)
    EVT_MENU(wxID_ABOUT, NaviMainFrame::onAbout)
END_EVENT_TABLE()


//================================================================================

TrackStatusHandler::TrackStatusHandler(NaviMainFrame* frame) throw() :
        m_mainFrame(frame),
        m_playedTrack(NULL),
        m_pipeline(NULL) {
}

Pipeline* TrackStatusHandler::getPipeline() const throw() {
    return m_pipeline;
}

void TrackStatusHandler::onPlay(wxCommandEvent& event) {
    if (m_pipeline != NULL) {
        if (m_pipeline->getState() == Pipeline::STATE_PLAYING) {
            pause();
        } else if (m_pipeline->getState() == Pipeline::STATE_PAUSED) {
            unpause();
        }
    }
}

void TrackStatusHandler::onStop(wxCommandEvent& event) {
    stop();
}


// XXX: I am not entirely sure if this pointer usage (tt->getPrev etc) is 
// robust and whatnot. 

void TrackStatusHandler::onPrev(wxCommandEvent& event) {
    TrackTable* tt = m_mainFrame->getTrackTable();
    TrackInfo* info = tt->getPrev(true);
    // prevent segfaults here pl0x
    if (info != NULL) {
        m_playedTrack = info;
        play();
    }
}

void TrackStatusHandler::onNext(wxCommandEvent& event) {
    TrackTable* tt = m_mainFrame->getTrackTable();
    TrackInfo* info = tt->getNext(true);
    // prevent segfaults here pl0x
    if (info != NULL) {
        std::cout << "Invoked by wxThread." << std::endl;
        m_playedTrack = info;
        play();
    }
}

void TrackStatusHandler::onPosChange(wxScrollEvent& event) {
    m_scrolling = true;
    if (event.GetEventType() == wxEVT_SCROLL_CHANGED) {
        if (m_pipeline != NULL) {
            m_pipeline->seekSeconds(event.GetPosition());
            m_scrolling = false;
        }
    }
}

void TrackStatusHandler::onListItemActivate(wxListEvent& event) {
    // 'data' holds the selected index of the list control.
    long data = event.GetData();    
    TrackTable* tt = m_mainFrame->getTrackTable();
    TrackInfo& trax = tt->getTrackInfo(data);
    m_playedTrack = &trax;

    play();
}

void TrackStatusHandler::play() throw() {
    wxASSERT(m_playedTrack != NULL);

    NavigationContainer* nav = m_mainFrame->getNavigationContainer();
    
    // m_playedTrack CAN BE NULL, not sure??
    const wxString& loc = m_playedTrack->getLocation();
    if (m_pipeline != NULL) {
        // if a pipeline already exists, stop it, delete it, nullify it, and
        // start a new pipeline.
        m_pipeline->stop();

        // Since we are using pipeline listeners (callbacks) which return the
        // 'this' pointer, we must ensure that the pipeline does not get deleted
        // too 'soon'. Or else we get segfaults your mother will be jealous of.
        // Therefore, we lock the mutex before deleting it. See the 
        // Pipeline::fireX functions to see how the mutex is positioned.
        s_pipelineListenerMutex.Lock(); 
        delete m_pipeline;
        m_pipeline = NULL;
        s_pipelineListenerMutex.Unlock(); 
    }

    try {
        m_pipeline = new GenericPipeline(loc);
        // subscribe to pipeline events here:
        m_pipeline->addListener(this);
    } catch (const AudioException& ex) {
        // TODO: test if this exception thing works. Oh, and
        // prettify it with a better icon and such.
        wxMessageDialog dlg(m_mainFrame, ex.getAsWxString(), wxT("Error"), wxOK);
        dlg.ShowModal();
    }

    m_pipeline->play();
    nav->setStopButtonEnabled(true);
    nav->setPlayPauseButtonEnabled(true);
    nav->setPauseVisible();
    nav->setTrack(m_playedTrack);
    nav->setSeekerValues(0, m_pipeline->getDurationSeconds(), true);
}

void TrackStatusHandler::unpause() throw() {
    NavigationContainer* nav = m_mainFrame->getNavigationContainer();

    if (m_pipeline != NULL) {
        m_pipeline->play();
        nav->setPauseVisible();
    } 
}

void TrackStatusHandler::pause() throw() {
    NavigationContainer* nav = m_mainFrame->getNavigationContainer();

    if (m_pipeline != NULL) {
        m_pipeline->pause();
        nav->setPlayVisible();
    } 
}

void TrackStatusHandler::stop() throw() {
    NavigationContainer* nav = m_mainFrame->getNavigationContainer();

    if (m_pipeline != NULL) {
        m_pipeline->stop();
        nav->setStopButtonEnabled(false);
        nav->setPlayPauseButtonEnabled(false);
        nav->setPlayVisible();
        nav->setSeekerValues(0, 1, false);
        nav->setTrack(NULL); // this will reset the 'display'.

        // same story as play(): mutexes.
        s_pipelineListenerMutex.Lock(); 
        delete m_pipeline;
        m_pipeline = NULL;
        s_pipelineListenerMutex.Unlock(); 
    }    
}

void TrackStatusHandler::pipelineStreamEnd(Pipeline* const pipeline) throw() {
    // XXX: this function is called from a gst thread.
    wxCommandEvent evt(NAVI_EVENT_TRACK_NEXT);
    AddPendingEvent(evt);
}

void TrackStatusHandler::pipelineError(Pipeline* const pipeline, const wxString& error) throw() {
    // never had a pipeline error before, need to test this though.
    std::cout << "Error: " << error.mb_str() << std::endl;
}

void TrackStatusHandler::pipelinePosChanged(Pipeline* const pipeline, unsigned int pos, unsigned int len) throw() {
    wxCommandEvent evt(NAVI_EVENT_POS_CHANGED);
    StreamPositionData* d = new StreamPositionData(pos, len);
    evt.SetClientObject(d);
    AddPendingEvent(evt);
}

void TrackStatusHandler::doUpdateSlider(wxCommandEvent& evt) {
    // called because of AddPendingEvent()
    StreamPositionData* derpity = static_cast<StreamPositionData*>(evt.GetClientObject());
    // !m_scrolling determines whether we are currently dragging the slider.
    // If so, do not dynamically update the seeker values.
    if (derpity != NULL && !m_scrolling) {
       NavigationContainer* nav = m_mainFrame->getNavigationContainer();
       nav->setSeekerValues(derpity->m_pos, derpity->m_max);
    }
    delete derpity;
}

BEGIN_EVENT_TABLE(TrackStatusHandler, wxEvtHandler)
    EVT_BUTTON(NavigationContainer::ID_MEDIA_PLAY, TrackStatusHandler::onPlay)
    EVT_BUTTON(NavigationContainer::ID_MEDIA_STOP, TrackStatusHandler::onStop)
    EVT_COMMAND_SCROLL(NavigationContainer::ID_MEDIA_SEEKER, TrackStatusHandler::onPosChange)
    EVT_LIST_ITEM_ACTIVATED(TrackTable::ID_TRACKTABLE, TrackStatusHandler::onListItemActivate)
    EVT_COMMAND(wxID_ANY, NAVI_EVENT_POS_CHANGED, TrackStatusHandler::doUpdateSlider)
    EVT_COMMAND(wxID_ANY, NAVI_EVENT_STREAM_STOP, TrackStatusHandler::onStop)
    EVT_COMMAND(wxID_ANY, NAVI_EVENT_TRACK_NEXT, TrackStatusHandler::onNext)
    EVT_BUTTON(NavigationContainer::ID_MEDIA_PREV, TrackStatusHandler::onPrev)
    EVT_BUTTON(NavigationContainer::ID_MEDIA_NEXT, TrackStatusHandler::onNext)
END_EVENT_TABLE()

} // namespace navi 


// This is main() replacement.
IMPLEMENT_APP(navi::NaviApp)
