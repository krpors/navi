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

StreamTable::StreamTable(wxWindow* parent) :
    wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
        wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_VRULES | wxVSCROLL) {

    wxListItem item;

    item.SetText(wxT("Description"));
    InsertColumn(0, item);
    SetColumnWidth(0, 340);

    item.SetText(wxT("URL"));
    InsertColumn(1, item);
    SetColumnWidth(1, 340);

    long index = InsertItem(item);
    SetItem(index, 0, wxT("Fresh.FM"));
    SetItem(index, 1, wxT("http://crap.fm"));

    SetItemData(index, index);
}

void StreamTable::onResize(wxSizeEvent& event) {
    int width, height;
    GetSize(&width, &height);

    // automatically set some widths here after resizing
    SetColumnWidth(0, 0.5 * width);
    SetColumnWidth(1, 0.5 * width);

    // re-layout the control, to the column sizes are actually being done.
    Layout();

    // propagate this event up the chain. If we don't do this, the control
    // will not be automatically resized.
    event.Skip();
}

BEGIN_EVENT_TABLE(StreamTable, wxListCtrl)
    //EVT_LIST_ITEM_ACTIVATED(TrackTable::ID_TRACKTABLE, TrackTable::onActivate)   
    //EVT_LIST_ITEM_SELECTED(TrackTable::ID_TRACKTABLE, TrackTable::onSelected)
    //EVT_LIST_COL_CLICK(TrackTable::ID_TRACKTABLE, TrackTable::onColumnClick)
    //EVT_COMMAND(wxID_ANY, naviDirTraversedEvent, TrackTable::onAddTrackInfo)
    EVT_SIZE(StreamTable::onResize)
END_EVENT_TABLE()

//================================================================================

StreamBrowserContainer::StreamBrowserContainer(wxWindow* parent, NaviMainFrame* frame) :
        wxPanel(parent, wxID_ANY) {

     // Panel with the buttons
    wxPanel* panelBtns = new wxPanel(this);
    wxBoxSizer* sizerBtns = new wxBoxSizer(wxHORIZONTAL);

    wxBitmap imgAdd = wxArtProvider::GetBitmap(wxT("gtk-add"));
    wxBitmap imgDel = wxArtProvider::GetBitmap(wxT("gtk-remove"));
 
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
    sizer->Add(panelBtns, wxSizerFlags().Border(wxALL, 3));
    sizer->Add(createStreamPanel(this), wxSizerFlags(1).Expand());
   
}

wxPanel* StreamBrowserContainer::createStreamPanel(wxWindow* parent) {
    wxPanel* panel = new wxPanel(parent);
    wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(sizer);

    m_streamTable = new StreamTable(panel); 
    sizer->Add(m_streamTable, wxSizerFlags(1).Expand());

    return panel;
}

// Event table.
BEGIN_EVENT_TABLE(StreamBrowserContainer, wxPanel)
END_EVENT_TABLE()

//================================================================================

} //namespace navi 

