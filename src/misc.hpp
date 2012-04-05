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

#include <vector>
#include <utility> // for pair

#include "audio.hpp"

#include <wx/wx.h>
#include <wx/xml/xml.h>
#include <wx/fileconf.h>
#include <wx/wfstream.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/uri.h>


namespace navi {

// Utility functions etc. first:

/**
 * Converts seconds (as integer) to a proper formatted time like mm:ss.
 * @param seconds The amount of seconds to format.
 */
const wxString formatSeconds(int seconds);

/**
 * Convert a wxString to an integer, or return the default when
 * parsing the string fails (i.e. not parseable to an int).
 *
 * @param str The string
 * @param def Default return value if parsing fails.
 * @return The default integer if parsing fails, or the parsed int.
 */
long strToInt(const wxString& str, int def);

/**
 * Escapes any single ampersand with a double ampersand, to escape
 * any possible menu mnemonics.
 * @param str The string to convert. A copy of a new string will be returned.
 */
wxString escapeMnemonics(const wxString& str);

//================================================================================

class StreamConfiguration {
private:
    std::vector<std::pair<wxString, wxString> > m_streams;

    wxFileName m_naviDir;
    wxFileName m_configFile;

    void createInitialConfig();
public:
    static const wxString CONFIG_FILE;

    StreamConfiguration();

    void load();
    void save();
    void addStream(const wxString& desc, const wxString& loc);
    std::vector<std::pair<wxString, wxString> >& getStreams();
};

//================================================================================

/**
 * This class represents the global preferences of this application. It extends
 * the functionality of wxFileConfig, and thus can be used as such. At start of
 * Navi, the "global" wxConfigBase is set in the NaviApp constructor by using the
 * wxConfigBase::Set() static method. The preferences can then be retrieved by
 * invoking wxConfigBase::Get().
 */
class Preferences : public wxFileConfig {
private:
    /// The configuration file string.
    wxString m_configFile;

    /**
     * Private constructor for use in createInstance().
     *
     * @param is The wxInputStream, will be passed to the superclass' constructor.
     * @parma configFile the configuration filename, built using the standard paths.
     */
    Preferences(wxInputStream& is, const wxString& configFile);

public:
    /**
     * Destructor. Once it gets destroyed, the preferences will be saved.
     */
    ~Preferences();

    /// The configuration file (preferences)
    static const wxString CONFIG_FILE;

/**
 * @name Preference keys. 
 * The keys of the preferences file.
 */
///@{

    /// Whether to minimize to tray. Holds a boolean (0, 1).
    static const wxString MINIMIZE_TO_TRAY;
    /// Whether to ask the user to really exit. Holds a boolean (0, 1).
    static const wxString ASK_ON_EXIT;
    /// The chosen base media directory. 
    static const wxString MEDIA_DIRECTORY;
    /// Whether to automatically sort on track number when loading a new dir.
    /// Holds a boolean (0, 1).
    static const wxString AUTO_SORT;
///@}    

    /**
     * Createa new instance of the Preferences. Usually, you only need
     * to call this once, and use wxConfigBase::Set().
     */
    static Preferences* createInstance();

    /**
     * Set the default values.
     */
    void setDefaults();

    /**
     * Convenience function to save the preferences using a wxInputStream.
     */
    void save();
};


} // namespace navi

#endif // MISC_HPP
