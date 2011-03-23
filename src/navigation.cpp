
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
    
    wxBitmapButton* btn1 = new wxBitmapButton(panelTop, wxID_ANY, prev, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    m_btnPlay = new wxBitmapButton(panelTop, ID_MEDIA_PLAY, play, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    m_btnStop = new wxBitmapButton(panelTop, ID_MEDIA_STOP, stop, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    wxBitmapButton* btn4 = new wxBitmapButton(panelTop, wxID_ANY, next, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    wxBitmapButton* btn5 = new wxBitmapButton(panelTop, wxID_ANY, random, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);

    // the play button is initially disabled, because there's nothing to play
    // Same thing with the stop button (nothing to stop)
    m_btnPlay->Enable(false);
    m_btnStop->Enable(false);

    wxSlider* slider = new wxSlider(panelTop, wxID_ANY, 0, 0, 100);

    topSizer->Add(btn1);
    topSizer->Add(m_btnPlay);
    topSizer->Add(m_btnStop);
    topSizer->Add(btn4);
    topSizer->Add(btn5);
    topSizer->Add(slider, wxSizerFlags(1).Expand());

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

} //namespace navi 

