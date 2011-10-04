//      misc.hpp
//      
//      Copyright 2011 Kevin Pors <krpors@users.sf.net>
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

#ifndef MISC_HPP 
#define MISC_HPP 

#include <wx/wx.h>
#include <wx/fileconf.h>
#include <wx/wfstream.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
namespace navi {

// Utility functions etc. first:

/**
 * Converts seconds (as integer) to a proper formatted time like mm:ss.
 * @param seconds The amount of seconds to format.
 */
const wxString formatSeconds(int seconds);

//================================================================================

class NaviPreferences : public wxFileConfig {
private:
    NaviPreferences(wxInputStream& is);

public:
    static const wxString CONFIG_FILE;
    static const wxString MINIMIZE_TO_TRAY;
    
    static NaviPreferences* getInstance();


    void createDefaults();
};

} // namespace navi

#endif // MISC_HPP
