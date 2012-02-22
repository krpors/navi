//      navigation.hpp
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

#ifndef NAVIGATION_HPP 
#define NAVIGATION_HPP 

#include "main.hpp"
#include "audio.hpp"
#include "misc.hpp"

#include <wx/wx.h>
#include <wx/artprov.h>
#include <wx/slider.h>

namespace navi {

// Forward declaration due to circular dependency. Also see main.hpp.
class NaviMainFrame;
class NavigationContainer;

const wxEventType NAVI_EVENT_TRACK_NEXT  = wxNewEventType();
const wxEventType NAVI_EVENT_POS_CHANGED = wxNewEventType();
const wxEventType NAVI_EVENT_STREAM_STOP = wxNewEventType();

//================================================================================

class StreamPositionData : public wxClientData {
public:
    StreamPositionData(unsigned int pos, unsigned int max);

    unsigned int m_pos;
    unsigned int m_max;
};

//================================================================================

/**
 * This class extends wxPanel, and contains the navigation elements. All events
 * initiated by the widgets of this panel are handled in the TrackStatusHandler.
 * Mainly because the TTH needs access to several other classes, which are contained
 * in the God-class, NaviMainFrame.
 */
class NavigationContainer : public wxPanel {
private:

    /// The main frame.
    NaviMainFrame* m_naviFrame;

    /// Previous track.
    wxBitmapButton* m_btnPrev;

    /// Play/pause
    wxBitmapButton* m_btnPlay;

    /// Stop track playback. LOL THIS RHYMES
    wxBitmapButton* m_btnStop;

    // Next track.
    wxBitmapButton* m_btnNext;

    /// Time indicator
    wxStaticText* m_txtTimeIndicator;

    /// Static text containing the track title.
    wxStaticText* m_txtTrackTitle;

    /// Static text containing the artist/album.
    wxStaticText* m_txtArtistAlbum;

    /// Slider, to control the position of the stream.
    wxSlider* m_slider;

public:
    static const short ID_MEDIA_PREV = 4000; 
    static const short ID_MEDIA_NEXT = 4001; 
    static const short ID_MEDIA_STOP = 4002; 
    static const short ID_MEDIA_PLAY = 4003; 
    static const short ID_MEDIA_RANDOM = 4004; 
    static const short ID_MEDIA_SEEKER = 4005; 

    /**
     * Ctor. Creates the panel widget with child widgets.
     *
     * @param naviFrame The frame parent for Navi.
     */
    NavigationContainer(wxWindow* parent, NaviMainFrame* naviFrame);

    /**
     * Sets the track information to display, or some empty-like string
     * when no id3v2 tag (or similar) could not be found.
     *
     * @param info The TrackInfo reference.
     */
    void setTrack(TrackInfo& info);


    /**
     * Sets whether the play/pause button is enabled.
     *
     * @param enabled True for enabling it, duh. And false if it should be disabled.
     */
    void setPlayPauseButtonEnabled(bool enabled);
    void setPlayVisible();
    void setPauseVisible();
    void setStopButtonEnabled(bool enabled);

    /**
     * Set the seeker (slider) value. This is updated by a thread which 'polls' the
     * current track position. Obviously, when streaming media is used, this thing
     * should be disabled, or not used. You catch the drift, right.
     *
     * @param pos The current position. Must be > max.
     * @param max The maximum (upper limit).
     * @param enabled Whether the seeker should be enabled or not (can the user
     *  interact with it, or not).
     */
    void setSeekerValues(unsigned int pos, unsigned int max, bool enabled = true);


    DECLARE_EVENT_TABLE()
};




} //namespace navi 

#endif // NAVIGATION_HPP
