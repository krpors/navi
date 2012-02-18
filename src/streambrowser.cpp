//      streambrowser.cpp
//      
//      Copyright 2012 Kevin Pors <krpors@users.sf.net>
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

#include "streambrowser.hpp"

#include <iostream>

namespace navi {

StreamBrowserContainer::StreamBrowserContainer(wxWindow* parent, NaviMainFrame* frame) :
        wxPanel(parent, wxID_ANY) {

     // Panel with the buttons
    wxPanel* panelBtns = new wxPanel(this);
    wxBoxSizer* sizerBtns = new wxBoxSizer(wxHORIZONTAL);

    wxBitmap imgAdd = wxArtProvider::GetBitmap(wxART_ADD_BOOKMARK);
    wxBitmap imgDel = wxArtProvider::GetBitmap(wxART_DEL_BOOKMARK);
 
    wxBitmapButton* btnAdd = new wxBitmapButton(panelBtns, wxID_ANY, imgAdd, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    btnAdd->SetToolTip(wxT("Add network stream"));

    wxBitmapButton* btnDel = new wxBitmapButton(panelBtns, wxID_ANY, imgDel, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    btnDel->SetToolTip(wxT("Delete selected network stream"));

    panelBtns->SetSizer(sizerBtns);
    sizerBtns->Add(btnAdd);
    sizerBtns->Add(btnDel);

    // Sizer of the main thing
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);
    sizer->Add(panelBtns, wxSizerFlags().Left().Border(wxALL, 3));
   
}

// Event table.
BEGIN_EVENT_TABLE(StreamBrowserContainer, wxPanel)
END_EVENT_TABLE()

//================================================================================

} //namespace navi 

