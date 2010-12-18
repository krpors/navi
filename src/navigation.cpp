
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

NavigationContainer::NavigationContainer(wxWindow* parent) :
        wxPanel(parent, wxID_ANY) {
    
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

    wxBitmap prev = wxArtProvider::GetBitmap(wxT("gtk-media-previous"));
    wxBitmap play = wxArtProvider::GetBitmap(wxT("gtk-media-play"));
    wxBitmap pause = wxArtProvider::GetBitmap(wxT("gtk-media-pause"));
    wxBitmap stop = wxArtProvider::GetBitmap(wxT("gtk-media-stop"));
    wxBitmap next = wxArtProvider::GetBitmap(wxT("gtk-media-next"));
    wxBitmap random = wxArtProvider::GetBitmap(wxT("gtk-refresh"));
    
    wxBitmapButton* btn1 = new wxBitmapButton(this, wxID_ANY, prev, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    wxBitmapButton* btn2 = new wxBitmapButton(this, wxID_ANY, play, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    wxBitmapButton* btn3 = new wxBitmapButton(this, wxID_ANY, stop, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    wxBitmapButton* btn4 = new wxBitmapButton(this, wxID_ANY, next, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    wxBitmapButton* btn5 = new wxBitmapButton(this, wxID_ANY, random, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);

    wxSlider* slider = new wxSlider(this, wxID_ANY, 0, 0, 100);

    sizer->Add(btn1);
    sizer->Add(btn2);
    sizer->Add(btn3);
    sizer->Add(btn4);
    sizer->Add(btn5);
    sizer->Add(slider, wxSizerFlags(1).Expand());

    SetSizer(sizer);
}



} //namespace navi 

