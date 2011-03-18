
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
    
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

    wxBitmap prev = wxArtProvider::GetBitmap(wxT("gtk-media-previous"));
    wxBitmap play = wxArtProvider::GetBitmap(wxT("gtk-media-play"));
    wxBitmap pause = wxArtProvider::GetBitmap(wxT("gtk-media-pause"));
    wxBitmap stop = wxArtProvider::GetBitmap(wxT("gtk-media-stop"));
    wxBitmap next = wxArtProvider::GetBitmap(wxT("gtk-media-next"));
    wxBitmap random = wxArtProvider::GetBitmap(wxT("gtk-refresh"));
    
    wxBitmapButton* btn1 = new wxBitmapButton(this, wxID_ANY, prev, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    m_btnPlay = new wxBitmapButton(this, ID_MEDIA_PLAY, play, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    m_btnStop = new wxBitmapButton(this, ID_MEDIA_STOP, stop, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    wxBitmapButton* btn4 = new wxBitmapButton(this, wxID_ANY, next, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    wxBitmapButton* btn5 = new wxBitmapButton(this, wxID_ANY, random, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);

    // the play button is initially disabled, because there's nothing to play
    // Same thing with the stop button (nothing to stop)
    m_btnPlay->Enable(false);
    m_btnStop->Enable(false);

    wxSlider* slider = new wxSlider(this, wxID_ANY, 0, 0, 100);

    sizer->Add(btn1);
    sizer->Add(m_btnPlay);
    sizer->Add(m_btnStop);
    sizer->Add(btn4);
    sizer->Add(btn5);
    sizer->Add(slider, wxSizerFlags(1).Expand());

    SetSizer(sizer);
}

void NavigationContainer::setStatePlaying(bool playing) throw() {
    m_btnPlay->Enable(true);
    m_btnStop->Enable(true);
    // visually change the buttons.
    if(playing) {
        m_btnPlay->SetBitmapLabel(wxArtProvider::GetBitmap(wxT("gtk-media-pause")));
    } else {
        m_btnPlay->SetBitmapLabel(wxArtProvider::GetBitmap(wxT("gtk-media-play")));
    }
}

void NavigationContainer::setButtonStoppedEnabled(bool enabled) throw() {
    m_btnStop->Enable(enabled);
}


} //namespace navi 

