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

#include <wx/wx.h>
#include <wx/artprov.h>
#include <wx/slider.h>

namespace navi {

// Forward declaration due to circular dependency. Also see main.hpp.
class NaviMainFrame;

class NavigationContainer : public wxPanel {
private:
    NaviMainFrame* m_naviFrame;

    wxBitmapButton* m_btnPlay;
    wxBitmapButton* m_btnStop;

public:
    static const short ID_MEDIA_PREV = 4000; 
    static const short ID_MEDIA_NEXT = 4001; 
    static const short ID_MEDIA_STOP = 4002; 
    static const short ID_MEDIA_PLAY = 4003; 
    static const short ID_MEDIA_RANDOM = 4004; 
    static const short ID_MEDIA_SEEKER = 4005; 

    NavigationContainer(wxWindow* parent, NaviMainFrame* naviFrame);

    void setPlayPauseStatus(bool playing);
};

} //namespace navi 

#endif // NAVIGATION_HPP
