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
#include "streambrowser.hpp"
#include "tracktable.hpp"
#include "navigation.hpp"
#include "misc.hpp"

#include <wx/wx.h>
#include <wx/taskbar.h>
#include <wx/debug.h>
#include <wx/treectrl.h>
#include <wx/tglbtn.h>
#include <wx/button.h>
#include <wx/notebook.h>
#include <wx/aboutdlg.h>
#include <wx/bitmap.h>
#include <wx/msgdlg.h>
#include <wx/splitter.h>


namespace navi {


// Forward declarations due to circular dependencies:
class NavigationContainer; // from navigation.hpp
class TrackStatusHandler; // from main.hpp below
class DirBrowserContainer; // from dirbrowser.hpp
class StreamBrowserContainer; // from streambrowser.hpp

//==============================================================================
    
/**
 * The application class.
 */
class NaviApp : public wxApp {
private:
    GenericPipeline* m_p;
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

/**
 * The SystrayIcon is a `system-tray' icon which is optionally visible. When the
 * user minimizes Navi, it will reside there, instead of in the 'task-bar'.
 */
class SystrayIcon : public wxTaskBarIcon {
private:
    /// The navi main frame.
    NaviMainFrame* m_mainFrame;
public:
    /**
     * Constructor.
     *
     * @param frame The navi main frame.
     */
    SystrayIcon(NaviMainFrame* frame);

    /**
     * Invoked when the thing is double clicked.
     */
    void onLeftDblClick(wxTaskBarIconEvent& event);    

    DECLARE_EVENT_TABLE()
};

//==============================================================================

class NaviMainFrame : public wxFrame {
private:

    wxNotebook* m_noteBook;

    DirBrowserContainer* m_dirBrowser;
    
    StreamBrowserContainer* m_streamBrowser;

    TrackTable* m_trackTable;

    NavigationContainer* m_navigation;

    wxImageList* m_imageList;

    TrackStatusHandler* m_trackStatusHandler;

    /// 'System tray' icon.
    SystrayIcon* m_taskBarIcon;

    void initMenu();

    wxPanel* createDirBrowserPanel(wxWindow* parent);
    wxPanel* createBottom(wxWindow* parent);

    wxStatusBar* OnCreateStatusBar(int number, long style, wxWindowID id, const wxString& name);

    void onResize(wxSizeEvent& event);

    void onAbout(wxCommandEvent& event);

    void onIconize(wxIconizeEvent& event);
    
    void onExit(wxCommandEvent& event);

    void onPreferences(wxCommandEvent& event);

    void onClose(wxCloseEvent& event);

public:
    NaviMainFrame();
    ~NaviMainFrame();

    TrackTable* getTrackTable() const;

    NavigationContainer* getNavigationContainer() const;

    DECLARE_EVENT_TABLE()
};

//================================================================================

/**
 * Preferences dialog, with user settable prefs. Shown as modal.
 */
class PreferencesDialog : public wxDialog {
private:
    wxCheckBox* m_chkMinimizeToTray;
    wxCheckBox* m_chkAskOnExit;
    wxCheckBox* m_chkSortOnTrackNum;

    wxPanel* createTopPanel(wxWindow* parent);
    wxPanel* createButtonPanel(wxWindow* parent);

    void onOK(wxCommandEvent& event);
public:
    /**
     * Constructor for ths dialog.
     */
    PreferencesDialog(wxWindow* parent);

    DECLARE_EVENT_TABLE()
};

//================================================================================


/**
 * This class can be seen as quite some meat of the playability of Navi. It makes
 * sure the play, stop, next etc. buttons do its work, and update the UI accordingly.
 * It is also a listener to any pipeline changes (due to it subclassing the
 * PipelineListener).
 */
class TrackStatusHandler : public wxEvtHandler, public PipelineListener {
private:
    const static unsigned short PIPELINE_STREAM = 0;
    const static unsigned short PIPELINE_TRACK = 1;


    /// Boolean to indicate whether we are currently scrolling with the
    /// slider. XXX: this may be a butt fugly hack!?!?! BUT WORKS
    bool m_scrolling;

    unsigned short m_pipelineType;

    /// The main frame of the application.
    NaviMainFrame* m_mainFrame;

    /// So... that pointer thing wasn't robust at all. We're just now passing
    /// TrackInfos as references, or just plain copies. We can use the isValid
    /// on this instance to check whether we are 'able' to play it.
    TrackInfo m_playedTrack;

    /// Pipeline with the current song.
    GenericPipeline* m_pipeline;

/**
 * @name UI callbacks
 * User Interface callback functions. These are functions which respond
 * to wxWidgets event table entries, like button presses and so on.
 */
///@{
    /**
     * Invoked when the play button is pressed on the navigation panel.
     */
    void onPlay(wxCommandEvent& event);

    /**
     * Invoked when the stop button is pressed on the navigation panel.
     */
    void onStop(wxCommandEvent& event);

    /**
     * Invoked when the prev button is pressed on the navigation panel. This
     * will obviously proceed to the previous track.
     */
    void onPrev(wxCommandEvent& event);

    /**
     * Invoked when the next button is pressed on the navigation panel. This
     * will obviously proceed to the next track.
     */
    void onNext(wxCommandEvent& event);

    /**
     * Invoked when the slider is slided. Any type of scroll event is 'caught'.
     */
    void onPosChange(wxScrollEvent& event);

    /**
     * Invoked when the volume is about to be changed.
     */
    void onVolChange(wxScrollEvent& event);

    /**
     * Invoked when a track is actived (i.e. double clicked, enter-ed) from the
     * track table widget.
     */
    void onTrackActivated(wxListEvent& event);

    /**
     * Invoked when a stream is activated (from that stream table).
     */
    void onStreamItemActivated(wxListEvent& event);

    /**
     * Invoked when a tag is read from a stream or just a plain local playable
     * file.
     */
    void onTagRead(wxCommandEvent& event);
///@}


/**
 * @name Pipeline callbacks
 * Pipeline object callbacks, which are the callbacks due to this TrackStatusHandler
 * being registered as a listener. All of these functions are called from a GST thread.
 * So all updates to the UI of wx need to be done with an `AddPendingEvent' call!!
 */
///@{

    void pipelineTagRead(Pipeline* const pipeline, const char* type, const wxString& value) throw();

    /**
     * Callback listener function from a Pipeline object. Invoked when the
     * pipeline has reached its end.
     *
     * @param pipeline The pipeline instance pointer.
     */
    void pipelineStreamEnd(Pipeline* const pipeline) throw();

    /**
     * Callback listener function from a Pipeline object. Invoked when the
     * pipeline has found an error.
     *
     * @param pipeline The pipeline instance pointer.
     * @param error The error string returned from gstreamer.
     */
    void pipelineError(Pipeline* const pipeline, const wxString& error) throw();

    /**
     * Callback listener function from a Pipeline object. Invoked as soon
     * as the position was changed in a stream (caused by playback). This
     * function will update the user interface accordingly (by displaying
     * the new time and updating the slider and whatnot). This function
     * may be tricky at first, because it adds a pending wxCommand event
     * to the instance of this TrackStatusHandler object. The callback
     * function then responds to this event to update the UI. Why like this?
     * Because wxWidgets requires us to update the UI in the main thread, and
     * this pipelinePosChanged function is called by a GStreamer thread.
     *
     * @param pipeline The pipeline instance pointar.
     * @param pos The current position in the pipeline, measured in seconds.
     * @param len The length of the pipeline, measured in seconds 
     *  (if appropriate, because live streams for instance don't have a 
     *  specific length or duration).
     */
    void pipelinePosChanged(Pipeline* const pipeline, unsigned int pos, unsigned int len) throw();
///@}    

    /**
     * Being called by the main wx thread to periodically (every second) update
     * the slider position.
     *
     * @param evt The command event. The client object passed is a StreamPositionData
     *   pointer, and is deleted within this function.
     */
    void doUpdateSlider(wxCommandEvent& evt);

public:
    TrackStatusHandler(NaviMainFrame* frame) throw();
    
    Pipeline* getPipeline() const throw();

    void play() throw();
    void unpause() throw();
    void pause() throw();
    void stop() throw();

    DECLARE_EVENT_TABLE()
};
    
} // namespace pl

#endif // MAIN_HPP

