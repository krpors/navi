
//      dirbrowser.cpp
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

#include "navigation.hpp"

#include <iostream>

namespace navi {

//================================================================================

StreamPositionData::StreamPositionData(unsigned int pos, unsigned int max) :
        m_pos(pos),
        m_max(max) {
}

//================================================================================

NavigationContainer::NavigationContainer(wxWindow* parent, NaviMainFrame* naviFrame) :
        wxPanel(parent, wxID_ANY),
        m_naviFrame(naviFrame) {

    //top panel
    wxPanel* panelTop = new wxPanel(this);
    wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);

    wxBitmap prev = wxArtProvider::GetBitmap(wxT("gtk-media-previous"));
    wxBitmap play = wxArtProvider::GetBitmap(wxT("gtk-media-play"));
    wxBitmap pause = wxArtProvider::GetBitmap(wxT("gtk-media-pause"));
    wxBitmap stop = wxArtProvider::GetBitmap(wxT("gtk-media-stop"));
    wxBitmap next = wxArtProvider::GetBitmap(wxT("gtk-media-next"));
    wxBitmap random = wxArtProvider::GetBitmap(wxT("gtk-refresh"));
   
    m_btnPrev = new wxBitmapButton(panelTop, ID_MEDIA_PREV, prev, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    m_btnPlay = new wxBitmapButton(panelTop, ID_MEDIA_PLAY, play, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    m_btnStop = new wxBitmapButton(panelTop, ID_MEDIA_STOP, stop, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    m_btnNext = new wxBitmapButton(panelTop, ID_MEDIA_NEXT, next, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    wxBitmapButton* btn5 = new wxBitmapButton(panelTop, wxID_ANY, random, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);

    // the play button is initially disabled, because there's nothing to play
    // Same thing with the stop button (nothing to stop)
    m_btnPlay->Enable(false);
    m_btnStop->Enable(false);

    m_slider = new wxSlider(panelTop, ID_MEDIA_SEEKER, 0, 0, 100);

    topSizer->Add(m_btnPrev);
    topSizer->Add(m_btnPlay);
    topSizer->Add(m_btnStop);
    topSizer->Add(m_btnNext);
    topSizer->Add(btn5);
    topSizer->Add(m_slider, wxSizerFlags(1).Expand());

    panelTop->SetSizer(topSizer);

    // bottom part (name of the song etc.)
    wxPanel* panelBottom = new wxPanel(this);
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxVERTICAL);
    m_txtTrackTitle = new wxStaticText(panelBottom, wxID_ANY, wxT("Nothing played"));
    wxFont font;
    font.SetWeight(wxFONTWEIGHT_BOLD);
    font.SetPointSize(12);
    m_txtTrackTitle->SetFont(font);
    bottomSizer->Add(m_txtTrackTitle);
    m_txtArtistAlbum = new wxStaticText(panelBottom, wxID_ANY, wxT("-"));
    bottomSizer->Add(m_txtArtistAlbum);
    panelBottom->SetSizer(bottomSizer);

    wxBoxSizer* lolsizer = new wxBoxSizer(wxVERTICAL);
    lolsizer->Add(panelTop, wxSizerFlags(1).Expand());
    lolsizer->Add(panelBottom, wxSizerFlags(1).Expand());
    SetSizer(lolsizer);

    // disable at first.
    setSeekerValues(0, 1, false);
}

void NavigationContainer::setTrack(TrackInfo& info) {
    if (info[TrackInfo::TITLE].IsEmpty()) {
        wxString s(wxT("(no tag) "));
        s.Append(info.getLocation());

        m_txtTrackTitle->SetLabel(s);
    } else {
        m_txtTrackTitle->SetLabel(info[TrackInfo::TITLE]);
    }
    m_txtArtistAlbum->SetLabel(info[TrackInfo::ARTIST]);
}

void NavigationContainer::setPlayPauseButtonEnabled(bool enabled) {
    m_btnPlay->Enable(enabled);
}

void NavigationContainer::setPlayVisible() {
    m_btnPlay->SetBitmapLabel(wxArtProvider::GetBitmap(wxT("gtk-media-play")));
}

void NavigationContainer::setPauseVisible() {
    m_btnPlay->SetBitmapLabel(wxArtProvider::GetBitmap(wxT("gtk-media-pause")));
}

void NavigationContainer::setStopButtonEnabled(bool enabled) {
    m_btnStop->Enable(enabled);
}

void NavigationContainer::setSeekerValues(unsigned int pos, unsigned int max, bool enabled) {
    m_slider->SetRange(0, max);
    m_slider->SetValue(pos);
    m_slider->Enable(enabled);
}

} //namespace navi 

