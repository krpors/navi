//      misc.cpp
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

#include "misc.hpp"

#include <iostream>

namespace navi {

extern const wxEventType naviDirTraversedEvent = wxNewEventType();

// seconds to minutes formatting.
const wxString formatSeconds(int secs) {
    wxString s;
    int mins = secs / 60;
    int leftover = secs % 60;

    if (mins <= 9) {
        s.Append(wxString::Format(wxT("%i"), mins).Pad(1, '0', false));
    } else {
        s.Append(wxString::Format(wxT("%i"), mins));
    }
    
    s.Append(wxT(":"));

    if (leftover <= 9) {
        s.Append(wxString::Format(wxT("%i"), leftover).Pad(1, '0', false));
    } else {
        s.Append(wxString::Format(wxT("%i"), leftover));
    }

    return s;
}

long strToInt(const wxString& str, int def) {
    long val;
    if(!str.ToLong(&val)) {
        val = def;
    }

    return val;
}

//================================================================================

const wxString StreamConfiguration::CONFIG_FILE = wxT("streams");

StreamConfiguration::StreamConfiguration() {
    // get the standard base path, and make a path with the configuration file.
    wxStandardPathsBase& wxsp = wxStandardPaths::Get();

    m_naviDir = wxFileName(wxsp.GetUserConfigDir(), wxT(".navi"));
    // check if the .navi dir exists. If not, create it.
    if (!wxDirExists(m_naviDir.GetFullPath())) {
        wxMkdir(m_naviDir.GetFullPath());
    } 

    m_configFile = wxFileName(m_naviDir.GetFullPath(), CONFIG_FILE);
    if(!wxFileExists(m_configFile.GetFullPath())) {
        createInitialConfig();
    } 
}

void StreamConfiguration::createInitialConfig() {
    wxXmlDocument doc;
    wxXmlNode* root = new wxXmlNode(NULL, wxXML_ELEMENT_NODE, wxT("stream-configuration"));
    doc.SetRoot(root);
    doc.Save(m_configFile.GetFullPath());
}

void StreamConfiguration::load() {
    m_streams.clear(); // empty the vector

    wxXmlDocument doc;
    doc.Load(m_configFile.GetFullPath());
    wxXmlNode* streams = doc.GetRoot()->GetChildren();
    while(streams != NULL) {
        if (streams->GetName() == wxT("stream")) {
            wxString desc = streams->GetPropVal(wxT("description"), wxT(""));
            wxString loc  = streams->GetPropVal(wxT("location"), wxT(""));

            addStream(desc, loc);
        }

        streams = streams->GetNext();
    }
    std::cout << std::endl;
}

void StreamConfiguration::save() {
    std::cout << "save" << std::endl;
    wxXmlDocument doc;
    wxXmlNode* root = new wxXmlNode(NULL, wxXML_ELEMENT_NODE, wxT("stream-configuration"));

    std::vector<std::pair<wxString, wxString> >::iterator it;
    it = m_streams.begin();
    while(it < m_streams.end()) {
        std::pair<wxString, wxString> meh = *it;

        wxXmlNode* stream = new wxXmlNode(NULL, wxXML_ELEMENT_NODE, wxT("stream"));

        stream->AddProperty(wxT("description"), meh.first);
        stream->AddProperty(wxT("location"), meh.second);

        root->AddChild(stream);

        it++;
    }

    doc.SetRoot(root);

    doc.Save(m_configFile.GetFullPath());
}

void StreamConfiguration::addStream(const wxString& desc, const wxString& loc) {
    std::pair<wxString, wxString> p(desc,loc);
    m_streams.push_back(p);
}

std::vector<std::pair<wxString, wxString> >& StreamConfiguration::getStreams() {
    return m_streams;
}

//================================================================================

const wxString NaviPreferences::CONFIG_FILE      = wxT("preferences");
const wxString NaviPreferences::MINIMIZE_TO_TRAY = wxT("/NaviPreferences/MinimizeToTray");
const wxString NaviPreferences::ASK_ON_EXIT      = wxT("/NaviPreferences/AskOnExit");
const wxString NaviPreferences::MEDIA_DIRECTORY  = wxT("/NaviPreferences/MediaDirectory");
const wxString NaviPreferences::AUTO_SORT        = wxT("/NaviPreferences/AutoSortOnTrackNum");

NaviPreferences::NaviPreferences(wxInputStream& is, const wxString& configFile) :
        wxFileConfig(is),
        m_configFile(configFile) {

    DontCreateOnDemand();
}

NaviPreferences::~NaviPreferences() {
}

void NaviPreferences::setDefaults() {
    Write(MINIMIZE_TO_TRAY, true);
    Write(ASK_ON_EXIT,      false);
    Write(MEDIA_DIRECTORY,  wxT("/"));
    Write(AUTO_SORT,        true);

    save();
}

void NaviPreferences::save() {
    wxFileOutputStream fos(m_configFile);
    Save(fos);
}

NaviPreferences* NaviPreferences::createInstance() {
    // get the standard base path, and make a path with the configuration file.
    wxStandardPathsBase& wxsp = wxStandardPaths::Get();

    wxFileName naviDir(wxsp.GetUserConfigDir(), wxT(".navi"));
    // check if the .navi dir exists. If not, create it.
    if (!wxDirExists(naviDir.GetFullPath())) {
        wxMkdir(naviDir.GetFullPath());
    } 

    wxFileName fn(naviDir.GetFullPath(), CONFIG_FILE);
    wxString configFile = fn.GetFullPath();

    NaviPreferences* config;
    // create new file, if it doesn't exist yet.
    if (!wxFileExists(configFile)) {
        // will create it:
        wxFile file(configFile, wxFile::write);
        
        wxFileInputStream fis(configFile);
        // wxFileConfig has to be created after we know the file exists, or some
        // stupid dialog box pops up, saying the file cannot be found. How quaint.
        config = new NaviPreferences(fis, configFile);

        // write some default values to the CONFIG_FILE.
        config->setDefaults();
    } else {
        // load from the file too.
        wxFileInputStream fis(configFile);
        // same story as above:
        config = new NaviPreferences(fis, configFile);
    }
    
    return config;
}

} // namespace navi
