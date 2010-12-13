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
#include "filetree.hpp"
#include "tracktable.hpp"

#include <wx/wx.h>
#include <wx/treectrl.h>
#include <wx/tglbtn.h>
#include <wx/button.h>

namespace navi {
    
/**
 * The application class.
 */
class NaviApp : public wxApp {
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
    OGGFilePipeline* m_ogg;

    FileTree* m_tree;

    TrackTable* m_trackTable;

    UpdateThread* m_updateThread;

    void initMenu();

    void play(wxListEvent& event);

    void dostuff(wxTreeEvent& event);

    void onResize(wxSizeEvent& event);

    wxStatusBar* OnCreateStatusBar(int number, long style, wxWindowID id, const wxString& name);

public:
    NaviMainFrame();

    DECLARE_EVENT_TABLE()
};

    
} // namespace pl

#endif // MAIN_HPP

