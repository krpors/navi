
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

StreamTagData::StreamTagData(const char* type, const wxString& value) :
        m_type(type),
        m_value(value) {
}

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
    wxBitmap random = wxArtProvider::GetBitmap(wxT("stock_volume"));
   
    m_btnPrev = new wxBitmapButton(panelTop, ID_MEDIA_PREV, prev, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    m_btnPlay = new wxBitmapButton(panelTop, ID_MEDIA_PLAY, play, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    m_btnStop = new wxBitmapButton(panelTop, ID_MEDIA_STOP, stop, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    m_btnNext = new wxBitmapButton(panelTop, ID_MEDIA_NEXT, next, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    wxBitmapButton* btn5 = new wxBitmapButton(panelTop, wxID_ANY, random, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    m_txtTimeIndicator = new wxStaticText(panelTop, wxID_ANY, wxT("-:- of -:-"));

    // the play button is initially disabled, because there's nothing to play
    // Same thing with the stop button (nothing to stop)
    m_btnPlay->Enable(false);
    m_btnStop->Enable(false);

    topSizer->Add(m_btnPrev);
    topSizer->Add(m_btnPlay);
    topSizer->Add(m_btnStop);
    topSizer->Add(m_btnNext);
    topSizer->Add(btn5);
    topSizer->AddStretchSpacer(1);
    topSizer->Add(m_txtTimeIndicator, wxSizerFlags(0).Center().Border(wxALL, 5));
    panelTop->SetSizer(topSizer);

    // middle part:
    wxPanel* panelMiddle = new wxPanel(this);
    wxBoxSizer* middleSizer = new wxBoxSizer(wxHORIZONTAL);
    m_positionSlider = new wxSlider(panelMiddle, ID_MEDIA_SEEKER, 0, 0, 100);
    m_volumeSlider = new wxSlider(panelMiddle, ID_MEDIA_VOLUME, 100, 0, 100);
    middleSizer->Add(m_positionSlider, wxSizerFlags(1).Expand());
    middleSizer->Add(m_volumeSlider, wxSizerFlags(1).Expand());
    panelMiddle->SetSizer(middleSizer);

    // bottom part (name of the song etc.)
    wxPanel* panelBottom = new wxPanel(this);
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxVERTICAL);
    m_txtTrackTitle = new wxStaticText(panelBottom, wxID_ANY, wxT("-"));
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
    lolsizer->Add(panelMiddle, wxSizerFlags(1).Expand());
    lolsizer->Add(panelBottom, wxSizerFlags(1).Expand());
    SetSizer(lolsizer);

    // disable at first.
    setPlayPauseButtonEnabled(false);
    setStopButtonEnabled(false);
    setSeekerValues(0, 1, false);
}

void NavigationContainer::setTrack(TrackInfo& info) {
    if (!info.isValid()) {
        m_txtTrackTitle->SetLabel(wxT("-"));
        m_txtArtistAlbum->SetLabel(wxT("-"));
        return;
    }

    if (info[TrackInfo::TITLE].IsEmpty()) {
        m_txtTrackTitle->SetLabel(info.getSimpleName());
    } else {
        m_txtTrackTitle->SetLabel(escapeMnemonics(info[TrackInfo::TITLE]));
    }

    wxString alb;
    alb.Append(info[TrackInfo::ARTIST]);
    alb.Append(wxT(" from "));
    if (info[TrackInfo::ALBUM].IsEmpty()) {
        alb.Append(wxT("unknown album"));
    } else {
        alb.Append(info[TrackInfo::ALBUM]);
    }
    m_txtArtistAlbum->SetLabel(escapeMnemonics(alb));


}

void NavigationContainer::setInfo(const wxString& top, const wxString& bottom) {
    m_txtTrackTitle->SetLabel(top);
    m_txtArtistAlbum->SetLabel(bottom);
}

void NavigationContainer::setPlayPauseButtonEnabled(bool enabled) {
    m_btnPlay->Enable(enabled);
    m_btnPrev->Enable(enabled);
    m_btnNext->Enable(enabled);
}

void NavigationContainer::setPlayVisible() {
    m_btnPlay->SetBitmapLabel(wxArtProvider::GetBitmap(wxT("gtk-media-play")));
}

void NavigationContainer::setPauseVisible() {
    m_btnPlay->SetBitmapLabel(wxArtProvider::GetBitmap(wxT("gtk-media-pause")));
}

void NavigationContainer::setStopButtonEnabled(bool enabled) {
    m_btnStop->Enable(enabled);
    m_btnPrev->Enable(enabled);
    m_btnNext->Enable(enabled);
}

void NavigationContainer::setSeekerValues(unsigned int pos, unsigned int max, bool enabled) {
    m_positionSlider->SetRange(0, max);
    m_positionSlider->SetValue(pos);
    m_positionSlider->Enable(enabled);
    
    if (enabled) {
        wxString lol;
        lol.Append(formatSeconds(pos));
        lol.Append(wxT(" of "));
        lol.Append(formatSeconds(max));
        m_txtTimeIndicator->SetLabel(lol);
        // XXX: re-layouting every second... sounds a bit overkill, doesnt it.
        m_txtTimeIndicator->GetParent()->Layout(); 
    } else {
        m_txtTimeIndicator->SetLabel(wxT("-:- of -:-"));
        m_txtTimeIndicator->GetParent()->Layout(); 
    }
}

unsigned short NavigationContainer::getVolume() throw() {
    return m_volumeSlider->GetValue();
}

BEGIN_EVENT_TABLE(NavigationContainer, wxPanel)
END_EVENT_TABLE()

} //namespace navi 

