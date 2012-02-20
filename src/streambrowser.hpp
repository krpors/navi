//      streambrowser.hpp
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

#ifndef STREAMBROWSER_HPP 
#define STREAMBROWSER_HPP

#include "audio.hpp"
#include "main.hpp"

#include <wx/wx.h>
#include <wx/app.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/imaglist.h>
#include <wx/treectrl.h>
#include <wx/mimetype.h>
#include <wx/icon.h>
#include <wx/iconloc.h>
#include <wx/artprov.h>
#include <wx/dirdlg.h>

namespace navi {

class NaviMainFrame;


class StreamTable: public wxListCtrl {
private:
public:
    StreamTable(wxWindow* parent); 

    void onResize(wxSizeEvent& event);

    DECLARE_EVENT_TABLE()
};

/**
 * This DirBrowserContainer is actually the container of the DirBrowser class
 * itself, with a few additional action buttons, all neatly layed out.
 */
class StreamBrowserContainer : public wxPanel {
private:
    StreamTable* m_streamTable;
    wxPanel* createStreamPanel(wxWindow* parent);
public:
    StreamBrowserContainer(wxWindow* parent, NaviMainFrame* frame);

    // Events plx for the buttons hurr durr
    DECLARE_EVENT_TABLE()
};

} //namespace navi 

#endif // STREAMBROWSER_HPP 
