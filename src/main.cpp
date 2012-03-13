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

#include <iostream>

namespace navi {

class Test {
private:
    GenericPipeline* m_p;
public:
    Test() {
        wxString src = wxT("http://scfire-dtc-aa01.stream.aol.com:80/stream/1025");
        std::cout << "constructed" << std::endl;
        m_p = new GenericPipeline(src);
        m_p->play();

        TagReader* t = new TagReader(src);
        TrackInfo info = t->getTrackInfo();
        std::cout << "Info: " << info[TrackInfo::GENRE].mb_str() << std::endl;
    }

    ~Test() {
        m_p->stop();
        delete m_p;
    }
};

//==============================================================================
//
bool NaviApp::OnInit() {
    // initialize the gstreamer api here:
    gst_init(NULL, NULL);
    wxInitAllImageHandlers();

    // Initialize default preferences crap here
    NaviPreferences* prefs = NaviPreferences::createInstance(); //should be done once
    wxConfigBase::Set(prefs);

    // construct the main frame.
    NaviMainFrame* frame = new NaviMainFrame;
    frame->SetSize(800, 600);
    frame->Center();
    frame->Show();
    SetTopWindow(frame);

    //Test* t = new Test;
    return true;
}

//==============================================================================

SystrayIcon::SystrayIcon(NaviMainFrame* frame) :
    m_mainFrame(frame) {
}

void SystrayIcon::onLeftDblClick(wxTaskBarIconEvent&) {
    m_mainFrame->Show(true);
    m_mainFrame->Raise();
}

BEGIN_EVENT_TABLE(SystrayIcon, wxTaskBarIcon)
    EVT_TASKBAR_LEFT_DCLICK(SystrayIcon::onLeftDblClick)
END_EVENT_TABLE()


//==============================================================================

NaviMainFrame::NaviMainFrame() :
        wxFrame((wxFrame*) NULL, wxID_ANY, wxT("Navi")),
        m_noteBook(NULL) {
    // create our menu here 
    initMenu();

    // navigation up top (play pause buttons)
    wxPanel* panelMain = new wxPanel(this);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    panelMain->SetSizer(sizer);

    m_navigation = new NavigationContainer(panelMain, this);

    // bottom part:
    wxPanel* p = createBottom(panelMain);

    // Add the components to the sizer. Add a border of 5 px so the widgets aren't
    // 'attached' to the edges of the wxFrame itself (looks neater).
    // add navigation to the sizer
    sizer->Add(m_navigation, wxSizerFlags().Expand().Border(wxALL, 5));
    // add the bottom pieces to the sizer. Note that we user a proportion of 1 here to max
    // it out all the way to the bottom.
    sizer->Add(p, wxSizerFlags(1).Expand().Border(wxALL, 5));
    
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

    // create a systray icon.
    m_taskBarIcon = new SystrayIcon(this);
    wxBitmap bm(wxT("./data/icons/navi.png"), wxBITMAP_TYPE_PNG);
    wxIcon icon;
    icon.CopyFromBitmap(bm);
    m_taskBarIcon->SetIcon(icon, wxT("Navi - Hey, listen!"));

}

NaviMainFrame::~NaviMainFrame() {
    // must delete this pointer, or else the program will not exit.
    delete m_taskBarIcon;
}

void NaviMainFrame::initMenu() {
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(wxID_PREFERENCES, wxT("&Preferences"));
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT, wxT("E&xit"));

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT, wxT("&About"));

    wxMenuBar* bar = new wxMenuBar;
    bar->Append(menuFile, wxT("&File"));
    bar->Append(menuHelp, wxT("&Help"));

    bar->SetHelpString(wxID_PREFERENCES, wxT("Navi properties and preferences"));
    bar->SetHelpString(wxID_ABOUT, wxT("About Navi"));
    
    SetMenuBar(bar);
}

wxPanel* NaviMainFrame::createDirBrowserPanel(wxWindow* parent) {
    NaviPreferences* prefs = static_cast<NaviPreferences*>(wxConfigBase::Get()); 

    wxPanel* p = new wxPanel(parent);
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    p->SetSizer(sizer);

    // Create the splitter component here.
    wxSplitterWindow* split = new wxSplitterWindow(p, wxID_ANY);

    // directory browser (left side)
    wxString directory;
    wxStandardPathsBase& stdpath = wxStandardPaths::Get();
    prefs->Read(NaviPreferences::MEDIA_DIRECTORY, &directory, stdpath.GetUserConfigDir());
    m_dirBrowser = new DirBrowserContainer(split, this);
    m_dirBrowser->getDirBrowser()->setBase(directory);
    m_dirBrowser->getDirBrowser()->setFilesVisible(false);

    // track table (right side)
    m_trackTable = new TrackTable(split);

    // FIXME: all of a sudden, setting the MINIMUM pane size to 20, explicitly
    // sets the actual pane size to 20... Thats why I increased it to 200, or else...
    split->SetMinimumPaneSize(200);
    split->SplitVertically(m_dirBrowser, m_trackTable);

    sizer->Add(split, wxSizerFlags(1).Expand());
    
    return p;
}

wxPanel* NaviMainFrame::createBottom(wxWindow* parent) {
    // XXX: images are now just for demonstration purposes.
    // attempt to initialize image lists:
    m_imageList = new wxImageList(16, 16);
    m_imageList->Add(wxArtProvider::GetIcon(wxART_HARDDISK));
    m_imageList->Add(wxArtProvider::GetIcon(wxART_ADD_BOOKMARK));
    m_imageList->Add(wxArtProvider::GetIcon(wxT("gtk-network")));


    wxPanel* panelWrapper = new wxPanel(parent);
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    panelWrapper->SetSizer(boxSizer);

    m_noteBook = new wxNotebook(panelWrapper, wxID_ANY);
    m_noteBook->AssignImageList(m_imageList);

    wxPanel* panelDirBrPanel = createDirBrowserPanel(m_noteBook);
    m_noteBook->AddPage(panelDirBrPanel, wxT("Browser"), true, 0);

    m_streamBrowser = new StreamBrowserContainer(m_noteBook, this);

    m_noteBook->AddPage(new wxButton(m_noteBook, wxID_ANY, wxT("template")), wxT("Favorites"), false, 1);
    m_noteBook->AddPage(m_streamBrowser, wxT("Streams"), false, 2);

    boxSizer->Add(m_noteBook, wxSizerFlags(1).Expand());

    return panelWrapper;
}

wxStatusBar* NaviMainFrame::OnCreateStatusBar(int number, long style, wxWindowID id, const wxString& name) {
    wxStatusBar* bar = new wxStatusBar(this, id, style, name);
    bar->SetStatusText(wxT("Navi started. Hey, listen!"));

    return bar;
}

void NaviMainFrame::onResize(wxSizeEvent& event) {
    // wx quirk? If we don't call "Refresh", the wxSplitWindow's resize-handle
    // will not refresh properly (graphical glitch).
    Refresh();
    event.Skip();
}

void NaviMainFrame::onAbout(wxCommandEvent& event) {
    wxAboutDialogInfo info;
    info.SetName(wxT("Navi"));
    info.SetVersion(wxT("0.1a"));
    info.SetDescription(wxT("Hey, listen! Hello! Hey, listen!\nNavi is a directory based music player for Linux."));
    info.AddArtist(wxT("James 'adamorjames' Corley"));
    info.AddDeveloper(wxT("Kevin 'Azzkikr' Pors"));
    info.SetWebSite(wxT("http://github.com/krpors/navi"));

    wxBitmap bm(wxT("./data/icons/navi.png"), wxBITMAP_TYPE_PNG);
    wxIcon icon;
    icon.CopyFromBitmap(bm);
    info.SetIcon(icon);

    wxAboutBox(info);
}

void NaviMainFrame::onExit(wxCommandEvent& event) {
    NaviPreferences* lol = static_cast<NaviPreferences*>(wxConfigBase::Get()); 
    bool ask;
    lol->Read(NaviPreferences::ASK_ON_EXIT, &ask, false);
    if (ask) {
        wxMessageDialog dlg(this, wxT("Hey, listen! Do you really want to exit Navi?"), wxT("Exit Navi?"), wxYES_NO | wxNO_DEFAULT);
        if (dlg.ShowModal() == wxID_NO) {
            return;
        }
    }

    Close(true);
}

void NaviMainFrame::onIconize(wxIconizeEvent& event) {
    Show(!event.Iconized());
}

void NaviMainFrame::onPreferences(wxCommandEvent& event) {

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
    EVT_MENU(wxID_PREFERENCES, NaviMainFrame::onPreferences)
    EVT_MENU(wxID_ABOUT, NaviMainFrame::onAbout)
    EVT_MENU(wxID_EXIT, NaviMainFrame::onExit)
    EVT_ICONIZE(NaviMainFrame::onIconize)
END_EVENT_TABLE()


//================================================================================

TrackStatusHandler::TrackStatusHandler(NaviMainFrame* frame) throw() :
        m_mainFrame(frame),
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
    TrackInfo info = tt->getPrev(true);
    if (!info.isValid()) {
        m_playedTrack = info;
        play();
    }
}

void TrackStatusHandler::onNext(wxCommandEvent& event) {
    TrackTable* tt = m_mainFrame->getTrackTable();
    TrackInfo info = tt->getNext(true);
    if (info.isValid()) {
        std::cout << "Invoked by wxThread." << std::endl;
        m_playedTrack = info;
        play();
    }
}

void TrackStatusHandler::onPosChange(wxScrollEvent& event) {
    // only allow scrolling if not a stream.
    if (m_pipelineType == PIPELINE_STREAM) {
        return;
    }

    // else, allow position seeking.
    m_scrolling = true;
    if (event.GetEventType() == wxEVT_SCROLL_CHANGED) {
        if (m_pipeline != NULL) {
            m_pipeline->seekSeconds(event.GetPosition());
            m_scrolling = false;
        }
    }
}

void TrackStatusHandler::onTrackActivated(wxListEvent& event) {
    m_pipelineType = PIPELINE_TRACK;

    // 'data' holds the selected index of the list control.
    long data = event.GetData();    
    TrackTable* tt = m_mainFrame->getTrackTable();
    TrackInfo trax = tt->getTrackInfo(data);
    m_playedTrack = trax;

    play();
}

void TrackStatusHandler::onStreamItemActivated(wxListEvent& event) {
    m_pipelineType = PIPELINE_STREAM;

    TrackInfo* info = static_cast<TrackInfo*>(event.GetClientObject());
    m_playedTrack = *info;

    play();

    delete info;
}

void TrackStatusHandler::onTagRead(wxCommandEvent& event) {
    // for now, just dont do anything when we're not a stream.
    if (m_pipelineType != PIPELINE_STREAM) {
        return;
    }

    NavigationContainer* nav = m_mainFrame->getNavigationContainer();
    StreamTagData* td = static_cast<StreamTagData*>(event.GetClientObject());
    // dbl check if not null
    if (td != NULL) {
        if (td->m_type == TrackInfo::TITLE) {
            nav->setInfo(td->m_value, m_playedTrack.getLocation());
       }
        delete td;
    }
}

void TrackStatusHandler::play() throw() {
    if (m_playedTrack.isValid()) {
        wxLogMessage(wxT("Houston, meet Problem."));
    }

    NavigationContainer* nav = m_mainFrame->getNavigationContainer();
    
    const wxString& loc = m_playedTrack.getLocation();
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
    nav->setPauseVisible();
    if (m_pipelineType == PIPELINE_STREAM) {
        nav->setInfo(wxT("Reading stream..."), wxT(""));
        nav->setSeekerValues(0, 1, false);
        nav->setPlayPauseButtonEnabled(false);
    } else {
        nav->setTrack(m_playedTrack);
        nav->setSeekerValues(0, m_pipeline->getDurationSeconds(), true);
        nav->setPlayPauseButtonEnabled(true);
    }
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
        TrackInfo empty;
        nav->setTrack(empty); // this will reset the 'display'.

        // same story as play(): mutexes.
        s_pipelineListenerMutex.Lock(); 
        delete m_pipeline;
        m_pipeline = NULL;
        s_pipelineListenerMutex.Unlock(); 
    }    
}

void TrackStatusHandler::pipelineTagRead(Pipeline* const pipeline, const char* type, const wxString& value) throw() {
    // NOTE: this function is called from a gst thread.
    wxCommandEvent evt(NAVI_EVENT_TAG_READ);
    std::cout << "pipelineTagRead: " << type << " -> " << value.mb_str() << std::endl;
    StreamTagData* td = new StreamTagData(type, value);
    evt.SetClientObject(td);
    AddPendingEvent(evt);
    // will call onTagRead() in turn.
}

void TrackStatusHandler::pipelineStreamEnd(Pipeline* const pipeline) throw() {
    // NOTE: this function is called from a gst thread.
    wxCommandEvent evt(NAVI_EVENT_TRACK_NEXT);
    AddPendingEvent(evt);
}

void TrackStatusHandler::pipelineError(Pipeline* const pipeline, const wxString& error) throw() {
    // NOTE: this function is called from a gst thread.
    // never had a pipeline error before, need to test this though.
    std::cout << "Error: " << error.mb_str() << std::endl;
}

void TrackStatusHandler::pipelinePosChanged(Pipeline* const pipeline, unsigned int pos, unsigned int len) throw() {
    // NOTE: this function is called from a gst thread.
    wxCommandEvent evt(NAVI_EVENT_POS_CHANGED);
    StreamPositionData* d = new StreamPositionData(pos, len);
    evt.SetClientObject(d);
    AddPendingEvent(evt);
    // this will in turn call doUpdateSlider.
}

void TrackStatusHandler::doUpdateSlider(wxCommandEvent& evt) {
    // called because of AddPendingEvent()
    StreamPositionData* derpity = static_cast<StreamPositionData*>(evt.GetClientObject());
    // !m_scrolling determines whether we are currently dragging the slider.
    // If so, do not dynamically update the seeker values.
    if (derpity != NULL && !m_scrolling) {
        if (m_pipelineType == PIPELINE_STREAM) {
        } else {
            NavigationContainer* nav = m_mainFrame->getNavigationContainer();
            nav->setSeekerValues(derpity->m_pos, derpity->m_max);
        }
    }
    delete derpity;
}

BEGIN_EVENT_TABLE(TrackStatusHandler, wxEvtHandler)
    EVT_BUTTON(NavigationContainer::ID_MEDIA_PLAY, TrackStatusHandler::onPlay)
    EVT_BUTTON(NavigationContainer::ID_MEDIA_STOP, TrackStatusHandler::onStop)
    EVT_BUTTON(NavigationContainer::ID_MEDIA_PREV, TrackStatusHandler::onPrev)
    EVT_BUTTON(NavigationContainer::ID_MEDIA_NEXT, TrackStatusHandler::onNext)

    EVT_COMMAND_SCROLL(NavigationContainer::ID_MEDIA_SEEKER, TrackStatusHandler::onPosChange)

    EVT_LIST_ITEM_ACTIVATED(TrackTable::ID_TRACKTABLE, TrackStatusHandler::onTrackActivated)
    EVT_LIST_ITEM_ACTIVATED(StreamTable::ID_STREAMTABLE, TrackStatusHandler::onStreamItemActivated)

    EVT_COMMAND(wxID_ANY, NAVI_EVENT_POS_CHANGED, TrackStatusHandler::doUpdateSlider)
    EVT_COMMAND(wxID_ANY, NAVI_EVENT_STREAM_STOP, TrackStatusHandler::onStop)
    EVT_COMMAND(wxID_ANY, NAVI_EVENT_TRACK_NEXT, TrackStatusHandler::onNext)
    EVT_COMMAND(wxID_ANY, NAVI_EVENT_TAG_READ, TrackStatusHandler::onTagRead)
END_EVENT_TABLE()

} // namespace navi 


// This is main() replacement.
IMPLEMENT_APP(navi::NaviApp)
