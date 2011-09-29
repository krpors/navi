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

const wxString secsToMins(int);

// Forward declarations due to circular dependencies:
class NavigationContainer; // from navigation.hpp
class TrackStatusHandler; // from main.hpp below

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


class NaviMainFrame : public wxFrame {
private:

    wxNotebook* m_noteBook;

    DirBrowserContainer* m_dirBrowser;

    TrackTable* m_trackTable;

    DirTraversalThread* m_dirTraversalThread;

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


/**
 * This class can be seen as quite some meat of the playability of Navi. It makes
 * sure the play, stop, next etc. buttons do its work, and update the UI accordingly.
 * It is also a listener to any pipeline changes (due to it subclassing the
 * PipelineListener).
 */
class TrackStatusHandler : public wxEvtHandler, public PipelineListener {
private:
    /// Boolean to indicate whether we are currently scrolling with the
    /// slider. XXX: this may be a butt fugly hack!?!?! BUT WORKS
    bool m_scrolling;

    /// The main frame of the application.
    NaviMainFrame* m_mainFrame;

    /// The currently played track. May be null. Don't delete. It's a pointer
    /// to a TrackInfo from the TrackTable. The TrackTable will delete it.
    /// XXX: inspect of this pointer-to-tracktable-trackinfo is robust... I
    /// suspect NOT, since it may be deleted by DeleteAllItems()
    TrackInfo* m_playedTrack;

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
     * Invoked when an item is actived (i.e. double clicked, enter-ed) from the
     * track table widget.
     */
    void onListItemActivate(wxListEvent& event);
///@}


/**
 * @name Pipeline callbacks
 * Pipeline object callbacks, which are the callbacks due to this 
 * TrackStatusHandlerbeing registered as a listener.
 */
///@{
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

