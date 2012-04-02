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

wxString escapeMnemonics(const wxString& str) {
    wxString s(str);
    s.Replace(wxT("&"), wxT("&&"));
    return s;
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

const wxString Preferences::CONFIG_FILE      = wxT("preferences");
const wxString Preferences::MINIMIZE_TO_TRAY = wxT("/Preferences/MinimizeToTray");
const wxString Preferences::ASK_ON_EXIT      = wxT("/Preferences/AskOnExit");
const wxString Preferences::MEDIA_DIRECTORY  = wxT("/Preferences/MediaDirectory");
const wxString Preferences::AUTO_SORT        = wxT("/Preferences/AutoSortOnTrackNum");

Preferences::Preferences(wxInputStream& is, const wxString& configFile) :
        wxFileConfig(is),
        m_configFile(configFile) {

    DontCreateOnDemand();
}

Preferences::~Preferences() {
    save();
}

void Preferences::setDefaults() {
    Write(MINIMIZE_TO_TRAY, true);
    Write(ASK_ON_EXIT,      false);
    Write(MEDIA_DIRECTORY,  wxT("/"));
    Write(AUTO_SORT,        true);

    save();
}

void Preferences::save() {
    wxFileOutputStream fos(m_configFile);
    Save(fos);
}

Preferences* Preferences::createInstance() {
    // get the standard base path, and make a path with the configuration file.
    wxStandardPathsBase& wxsp = wxStandardPaths::Get();

    wxFileName naviDir(wxsp.GetUserConfigDir(), wxT(".navi"));
    // check if the .navi dir exists. If not, create it.
    if (!wxDirExists(naviDir.GetFullPath())) {
        wxMkdir(naviDir.GetFullPath());
    } 

    wxFileName fn(naviDir.GetFullPath(), CONFIG_FILE);
    wxString configFile = fn.GetFullPath();

    Preferences* config;
    // create new file, if it doesn't exist yet.
    if (!wxFileExists(configFile)) {
        // will create it:
        wxFile file(configFile, wxFile::write);
        
        wxFileInputStream fis(configFile);
        // wxFileConfig has to be created after we know the file exists, or some
        // stupid dialog box pops up, saying the file cannot be found. How quaint.
        config = new Preferences(fis, configFile);

        // write some default values to the CONFIG_FILE.
        config->setDefaults();
    } else {
        // load from the file too.
        wxFileInputStream fis(configFile);
        // same story as above:
        config = new Preferences(fis, configFile);
    }
    
    return config;
}

//================================================================================

#if 0

Notification::Notification(const wxString& body) :
        m_body(body) {
}

Notification::Notification(TrackInfo& info) {
    m_body = wxT("<b>");
    m_body += info[TrackInfo::ARTIST];
    m_body += wxT("</b>\n");
    m_body += wxT("<u>");
    m_body += info[TrackInfo::TITLE];
    m_body += wxT("</u>");
}

Notification::~Notification() {
}

void Notification::show(unsigned int timeoutSeconds) {
    // libnotify 0.5.0 does not handle ampersands (&) correctly in the summary.
    // Therefore, I'm replacing them temporarily with something else.
    m_body.Replace(wxT("&"), wxT(" and "));

    NotifyNotification* nn = notify_notification_new("Now playing:", m_body.mb_str(), "gtk-yes", NULL); 
    // XXX: on my Linux Mint, the timeout does not have effect?
    notify_notification_set_timeout(nn, timeoutSeconds * 1000);
    notify_notification_set_urgency(nn, NOTIFY_URGENCY_LOW);
    GError* error = NULL;
    notify_notification_show(nn, &error);
    if (error != NULL) {
        g_error_free(error);
    }

    // by immediately closing it, we don't add it to the stack of notifications
    // in GNOME 3 (I guess). If we don't do this, the user must manually close all
    // the notifications... or else they won't disappear.
    #if 0
    error = NULL;
    notify_notification_close(nn, &error);
    if (error != NULL) {
        g_error_free(error);
    }
    #endif

}

#endif

} // namespace navi
