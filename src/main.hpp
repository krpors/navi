//      main.hpp
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

#ifndef MAIN_HPP
#define MAIN_HPP


#include "audio.hpp"
#include "dirbrowser.hpp"
#include "tracktable.hpp"
#include "navigation.hpp"

#include <wx/wx.h>
#include <wx/debug.h>
#include <wx/treectrl.h>
#include <wx/tglbtn.h>
#include <wx/button.h>
#include <wx/notebook.h>
#include <wx/aboutdlg.h>
#include <wx/bitmap.h>
#include <wx/msgdlg.h>


namespace navi {

// Forward declarations due to circular dependencies:
class NavigationContainer; // from navigation.hpp
class TrackStatusHandler; // from main.hpp below

//==============================================================================
    
/**
 * The application class.
 */
class NaviApp : public wxApp {
public:
    /**
     * Initialization stuff.
     */
    virtual bool OnInit();
};

//==============================================================================

// Declare the application using wxWidgets macro. Makes sure we can retrieve
// the current app with wxGetApp().
DECLARE_APP(NaviApp)

//==============================================================================


class NaviMainFrame : public wxFrame {
private:

    wxNotebook* m_noteBook;

    DirBrowserContainer* m_dirBrowser;

    TrackTable* m_trackTable;

    UpdateThread* m_updateThread;

    NavigationContainer* m_navigation;

    wxImageList* m_imageList;

    TrackStatusHandler* m_trackStatusHandler;

    void initMenu();

    wxPanel* createNavPanel(wxWindow* parent);

    void dostuff(wxTreeEvent& event);

    void onResize(wxSizeEvent& event);

    void onAbout(wxCommandEvent& event);

    wxStatusBar* OnCreateStatusBar(int number, long style, wxWindowID id, const wxString& name);

public:
    NaviMainFrame();

    TrackTable* getTrackTable() const;

    NavigationContainer* getNavigationContainer() const;

    DECLARE_EVENT_TABLE()
};

//================================================================================

class TrackStatusHandler : public wxEvtHandler {
private:
    /// The main frame of the application.
    NaviMainFrame* m_mainFrame;

    /// The currently played track. May be null.
    TrackInfo* m_playedTrack;

    /// Pipeline with the current song.
    GenericPipeline* m_pipeline;

    void onPlay(wxCommandEvent& event);
    void onStop(wxCommandEvent& event);
    void onListItemActivate(wxListEvent& event);

public:
    TrackStatusHandler(NaviMainFrame* frame) throw();
    
    Pipeline* getPipeline() const throw();

    void play() throw();
    void unpause() throw();
    void pause() throw();

    DECLARE_EVENT_TABLE()
};
    
} // namespace pl

#endif // MAIN_HPP

