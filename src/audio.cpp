//      audio.cpp
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

#include "audio.hpp"

#include <iostream>

namespace navi {


//==============================================================================

AudioException::AudioException(const wxString& what) throw() :
        m_what(what) {
}

AudioException::~AudioException() throw() {
}   

const char* AudioException::what() const throw() {
    std::string converter(m_what.char_str());
    return converter.c_str();
}

const wxString& AudioException::getAsWxString() const throw() {
    return m_what;
}

//==============================================================================

PipelineListener::PipelineListener() {
}

//==============================================================================

Pipeline::Pipeline() throw() :
        m_intervalTag(0),
        m_location(wxT("")),
        m_bus(NULL), 
        m_pipeline(NULL) {
}

Pipeline::~Pipeline() {
    // Set state of pipeline to GST_STATE_NULL. We also do an explicit checking
    // of m_pipeline NULLage, or else GST_IS_ELEMENT will brabble about assertions
    // failed etc. etc.
    if (m_pipeline != NULL) {
        if(GST_IS_ELEMENT(m_pipeline)) {
            gst_element_set_state(m_pipeline, GST_STATE_NULL);
        }
    }
    // Unref the bus. Same NULL check as m_pipeline.
    if (m_bus != NULL) {
        if(GST_IS_BUS(m_bus)) {
            gst_object_unref(m_bus); 
        }
    }

    // and unref the pipeline. Don't individually unref it's child elements,
    // meaning the filesrc, mad, audioconverter, sink and whatever GstElement, 
    // or else it will spew funky warning messages on stderr.
    if (m_pipeline != NULL) {
        if (GST_IS_ELEMENT(m_pipeline)) {
            gst_object_unref(m_pipeline); 
        }
    }

    // by removing the timeout, we remove the callback we initially registered
    // using the function registerInterval(). Tag must be > 0, or else we'll get
    // some assertion errors from glib. 
    if(m_intervalTag > 0) {
        g_source_remove(m_intervalTag);
    }
}

void Pipeline::addListener(PipelineListener* const listener) throw() { 
    m_listeners.push_back(listener);
}

void Pipeline::fireError(const wxString& error) throw() {
    wxMutexLocker lock(s_pipelineListenerMutex);  

    std::vector<PipelineListener*>::iterator it = m_listeners.begin();
    while(it < m_listeners.end()) {
        (*it)->pipelineError(this, error);
        it++;
    }
}

void Pipeline::fireTagRead(const char* type, const wxString& value) throw() {
     wxMutexLocker lock(s_pipelineListenerMutex);  

    std::vector<PipelineListener*>::iterator it = m_listeners.begin();
    while(it < m_listeners.end()) {
        (*it)->pipelineTagRead(this, type, value);
        it++;
    }   
}

void Pipeline::firePositionChanged(gint64 pos, gint64 len) throw() {
    wxMutexLocker lock(s_pipelineListenerMutex);  

    std::vector<PipelineListener*>::iterator it = m_listeners.begin();
    while(it < m_listeners.end()) {
        // convert nanoseconds to seconds here plx.
        (*it)->pipelinePosChanged(this, pos / GST_SECOND, len / GST_SECOND);
        it++;
    }
}

void Pipeline::fireStreamEnd() throw() {
    // lolmutex
    wxMutexLocker lock(s_pipelineListenerMutex);  

    std::vector<PipelineListener*>::iterator it = m_listeners.begin();
    while(it < m_listeners.end()) {
        // FIXME: this may horribly segfault when skipping to a next song
        (*it)->pipelineStreamEnd(this);
        it++;
    }   
}

bool Pipeline::onInterval(Pipeline* pipeline) {
    GstFormat fmt = GST_FORMAT_TIME;
    // nanoseconds:
    gint64 pos, len;

    // remember, m_pipeline (member of Pipeline) is protected.
    // therefore we can access it easily.

    if (gst_element_query_position (pipeline->m_pipeline, &fmt, &pos)
            && gst_element_query_duration (pipeline->m_pipeline, &fmt, &len)) {
        pipeline->firePositionChanged(pos, len);
    }    

    // by returning true, we ensure this interval function gets called
    // periodically. If we return false, the timeout will be discarded.
    // Instead of returning false to stop this timeout, we 'unregister'
    // the timeout by calling g_source_remove (gint tag) in the destructor.
    return true;
}

void Pipeline::registerInterval() {
    // ensure that a pipeline is set by the subclass:
    wxASSERT(m_pipeline != NULL);

    // This comes from GObject. We add a timeout to query the current position
    // every second. Use m_pipeline from the class Pipeline as the callback
    // data. The timeout tag can be used later on to disable/destroy the
    // callback.
    m_intervalTag = g_timeout_add(1000, (GSourceFunc) onInterval, this);

}

gboolean Pipeline::busWatcher(GstBus* bus, GstMessage* message, gpointer userdata) {
    // get the name of the message. 
    Pipeline* pipeline = static_cast<Pipeline*>(userdata);
    //const gchar* name = GST_MESSAGE_TYPE_NAME(message);
    //std::cout << "Got GST message: " << name << std::endl;

    GstMessageType type = GST_MESSAGE_TYPE(message);
    if(type == GST_MESSAGE_EOS) {
        pipeline->fireStreamEnd(); 
    } else if (type == GST_MESSAGE_ERROR) {
        gchar* debug;
        GError* error;

        gst_message_parse_error (message, &error, &debug);
        g_free (debug);

        wxString err = wxString::FromAscii(error->message);
        pipeline->fireError(err);

        g_error_free (error);
    } else if (type == GST_MESSAGE_TAG) {
        GstTagList* tags = NULL;
        gst_message_parse_tag (message, &tags);
        gst_tag_list_foreach(tags, handleTags, userdata);
        gst_tag_list_free (tags);
    } else if (type == GST_MESSAGE_BUFFERING) {
        //std::cout << "Everybody's bufferin'" << std::endl;
    }
  
    return true;
}

void Pipeline::handleTags(const GstTagList* list, const gchar* tag, gpointer userdata) {
    Pipeline* pipeline = static_cast<Pipeline*>(userdata);

    // These values will get filled by the gst_tag_list_get_xyz() functions
    gchar* title;
    gchar* artist;
    gchar* album;
    gchar* genre;
    gchar* comment;
    gchar* composer;
    guint trackNum;
    guint discNum;
    GDate* date;

#ifdef DEBUG
    gchar* misc;
    if (gst_tag_list_get_string(list, tag, &misc)) {
        std::cout << "Tag " << tag << " = " << misc << std::endl;
    }
#endif
    // XXX: this can be done better. We actually just have to check whether
    // the found tag is a string, then fire an event. If its an int, convert
    // to string, then fire event. If it's a date, convert to string, then fire event...

    if (gst_tag_list_get_string(list, TrackInfo::TITLE, &title)) {
        pipeline->fireTagRead(TrackInfo::TITLE, wxString(title, wxConvUTF8));
        g_free(title);
    }
    if (gst_tag_list_get_string(list, TrackInfo::ARTIST, &artist)) {
        pipeline->fireTagRead(TrackInfo::ARTIST, wxString(artist, wxConvUTF8));
        g_free(artist);
    }
    if (gst_tag_list_get_string(list, TrackInfo::ALBUM, &album)) {
        pipeline->fireTagRead(TrackInfo::ALBUM, wxString(album, wxConvUTF8));
        g_free(album);
    }
    if (gst_tag_list_get_string(list, TrackInfo::GENRE, &genre)) {
        pipeline->fireTagRead(TrackInfo::GENRE, wxString(genre, wxConvUTF8));
        g_free(genre);
    }
    if (gst_tag_list_get_string(list, TrackInfo::COMMENT, &comment)) {
        pipeline->fireTagRead(TrackInfo::COMMENT, wxString(comment, wxConvUTF8));
        g_free(comment);
    }
    if (gst_tag_list_get_string(list, TrackInfo::COMPOSER, &composer)) {
        pipeline->fireTagRead(TrackInfo::COMPOSER, wxString(composer, wxConvUTF8));
        g_free(composer);
    }
    if (gst_tag_list_get_uint(list, TrackInfo::TRACK_NUMBER, &trackNum)) {
        pipeline->fireTagRead(TrackInfo::TRACK_NUMBER, wxString::Format(wxT("%i"), trackNum));
    }
    if (gst_tag_list_get_uint(list, TrackInfo::DISC_NUMBER, &discNum)) {
        pipeline->fireTagRead(TrackInfo::DISC_NUMBER, wxString::Format(wxT("%i"), discNum));
    }
    if (gst_tag_list_get_date(list, TrackInfo::DATE, &date)) {
        // XXX: date also contains month and day.. Include  this, or just year?
        unsigned int ddate = g_date_get_year(date);
        pipeline->fireTagRead(TrackInfo::DISC_NUMBER, wxString::Format(wxT("%i"), ddate));
        g_date_free(date);
    }
}

void Pipeline::play() throw() {
    gst_element_set_state(m_pipeline, GST_STATE_PLAYING);

    registerInterval();
}

void Pipeline::pause() throw() {
    gst_element_set_state(m_pipeline, GST_STATE_PAUSED);

    // we dont need to get notified of the pipeline's progress every .5 seconds
    // if we are paused.
    if(m_intervalTag > 0) {
        g_source_remove(m_intervalTag);
    }

}

void Pipeline::stop() throw() {
    // pause first (i.e. stop playback), which will implicitly also stop the
    // interval callback.
    pause();
    // then 'seek' to the start of the file.
    gboolean seekSuccess = gst_element_seek (m_pipeline, 1.0, 
        GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
        GST_SEEK_TYPE_SET, 0,
        GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);

    if (!seekSuccess) {
        // XXX: seek always fails when a stream is stopped. Obviously though,
        // since we can not 'seek' to the start of the stream. It's no biggie
        // though, but annoying if you keep seeing it. Let's hide it for now...
        //std::cerr << "Seek failed!" << std::endl;
    }
}

short Pipeline::getState() const throw(AudioException) {
    // query the state with a 5 second timeout.
    GstState state;
    GstStateChangeReturn st = gst_element_get_state(m_pipeline, &state, NULL, 5 * GST_SECOND);
    if (st == GST_STATE_CHANGE_FAILURE) {
        throw AudioException(wxT("State change failure occured"));
    }

    return state;
}

const wxString& Pipeline::getLocation() const throw() {
    return m_location;
}

void Pipeline::seekSeconds(const unsigned int seconds) throw(AudioException) {
    // default pipeline implementation allows seeking in a file
    
    gboolean success = gst_element_seek 
        (m_pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
        GST_SEEK_TYPE_SET, seconds * GST_SECOND, GST_SEEK_TYPE_NONE, 
        GST_CLOCK_TIME_NONE);

    if (!success) {
        throw AudioException(wxT("Seek failed"));
    }
}

int Pipeline::getDurationSeconds() throw(AudioException) {
    // Query duration. Use time formatting
    GstFormat fmt = GST_FORMAT_TIME;
    // unsigned long int to hold the length of the stream
    gint64 len;
    // First, wait for state change to happen (WITH!! TIMEOUT): This will 
    // actually block until it returns, or the timeout has expired.
    // XXX: I AM NOT SURE IF THIS IS OKAY AT ALL TIMES, BUT IT WORKS. Please test
    // this thorougly, or request help from a GST expert!!! This has been done with
    // some pointers from __tim from #gstreamer at Freenode.
    GstStateChangeReturn st = gst_element_get_state(m_pipeline, NULL, NULL, 5 * GST_SECOND);
    // query element duration, put it in the address of `len'.
    gst_element_query_duration (m_pipeline, &fmt, &len);

    if (st == GST_STATE_CHANGE_FAILURE) {
        throw AudioException(wxT("State change failure occured"));
    }

    return len / GST_SECOND;
}

void Pipeline::setVolume(unsigned short percentage) throw() {
}

//==============================================================================

GenericPipeline::GenericPipeline(const wxString& location) throw (AudioException) {
    m_location = location;

    try {
        init();
    } catch (const AudioException& ex) {
        // rethrow
        throw;
    }
}

void GenericPipeline::init() throw (AudioException) {
    // Element playbin2 (gst-inspect playbin2)
    m_playbin = gst_element_factory_make("playbin2", NULL);
    if (!m_playbin) {
        throw AudioException(wxT("Failed to create `playbin2' GST element"));
    }

    // m_pipeline and m_bus are protected in parent class Pipeline.
    m_pipeline = gst_pipeline_new(NULL);

    m_bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
    // Add a watch to this bus (get notified of events using callbacks).
    // See http://www.parashift.com/c++-faq-lite/pointers-to-members.html, section
    // 33.2 for more details why it's done like this.
    gst_bus_add_watch (m_bus, Pipeline::busWatcher, this);

    gst_bin_add(GST_BIN(m_pipeline), m_playbin);

    // set the "location" property on the filesrc element.
    std::string s = std::string(m_location.mb_str());
    g_object_set(G_OBJECT(m_playbin), "uri", s.c_str(), NULL);
    
    /*
    We're gonna set some flags here. I don't need Navi to play video files as 
    well (that's automatically done by the the playbin2 element), so we're gonna
    set some flags. Default is 0x00000017.

    (0x00000001): video            - Render the video stream
    (0x00000002): audio            - Render the audio stream
    (0x00000004): text             - Render subtitles
    (0x00000008): vis              - Render visualisation when no video is present
    (0x00000010): soft-volume      - Use software volume
    (0x00000020): native-audio     - Only use native audio formats
    (0x00000040): native-video     - Only use native video formats
    (0x00000080): download         - Attempt progressive download buffering
    (0x00000100): buffering        - Buffer demuxed/parsed data
    (0x00000200): deinterlace      - Deinterlace video if necessary
    */

    unsigned short audio   = 0x00000002;
    unsigned short softvol = 0x00000010;
    unsigned short render = audio | softvol;
    g_object_set(G_OBJECT(m_playbin), "flags", render, NULL);

    // We set the state of the element as paused, so we can succesfully query
    // duration and other stuff. If the state is still not PAUSED or PLAYING, 
    // fetching the duration has no (real and useful) effect. It may return random
    // data.
    pause(); 

}

void GenericPipeline::setVolume(unsigned short percentage) throw() {
    g_object_set(G_OBJECT(m_playbin), "volume", (percentage / 100.0), NULL);
}

//================================================================================


const char* TrackInfo::TITLE = GST_TAG_TITLE;
const char* TrackInfo::ARTIST = GST_TAG_ARTIST;
const char* TrackInfo::ALBUM = GST_TAG_ALBUM;
//const char* TrackInfo::ALBUM_ARTIST = "";//GST_TAG_ALBUM_ARTIST;
const char* TrackInfo::GENRE = GST_TAG_GENRE;
const char* TrackInfo::COMMENT = GST_TAG_COMMENT;
const char* TrackInfo::COMPOSER = GST_TAG_COMPOSER;
const char* TrackInfo::TRACK_NUMBER = GST_TAG_TRACK_NUMBER;
const char* TrackInfo::DISC_NUMBER = GST_TAG_ALBUM_VOLUME_NUMBER;
const char* TrackInfo::DATE = GST_TAG_DATE;

TrackInfo::TrackInfo() :
    m_durationSeconds(-1) {
}

wxString& TrackInfo::operator[](const char* key) {
    return m_tags[key];
}


void TrackInfo::setLocation(const wxString& location) {
    m_location = location;
}

const wxString& TrackInfo::getLocation() const {
    return m_location;
}

int TrackInfo::getDurationSeconds() const throw() {
    return m_durationSeconds;
}

void TrackInfo::setDurationSeconds(int durrrr) throw() {
    m_durationSeconds = durrrr;
}

bool TrackInfo::isValid() const {
    return !m_location.IsEmpty();
}

const wxString TrackInfo::getSimpleName() throw() {
    wxURI theuri(m_location);
    const wxString path = theuri.Unescape(theuri.GetPath());
    int lastindexofslash = path.Find('/', true);
    if (lastindexofslash != wxNOT_FOUND) {
        // remove the front part, up until the filename itself.
        wxString meh = path.Mid(lastindexofslash + 1);
        return meh;
    } 
    // if not found, return the original path eh.
    return path;
}



//==============================================================================


TagReader::TagReader(const wxString& location) throw(AudioException) {
    m_location = location;
    try {
        init();
    } catch (const AudioException& ex) {
        throw;
    }
}

TagReader::~TagReader() {
}

void TagReader::onPadAdded(GstElement* element, GstPad* pad, GstElement* fakesink) throw() {
    // "sink" is a capability of "fakesink". See gst-inspect fakesink , then look
    // at the available PADS.
    GstPad* sinkpad = gst_element_get_static_pad (fakesink, "sink");

    if (!gst_pad_is_linked (sinkpad)) {
        if (gst_pad_link (pad, sinkpad) != GST_PAD_LINK_OK) {
            std::cerr << "Failed to link pads!" << std::endl;
        }
    }
    gst_object_unref (sinkpad);
}

void TagReader::onTagRead(const GstTagList* list, const gchar* tag, gpointer data) throw() {
    // The gpointer data is always a TagReader instance (from initTags()).
    // So make sure we cast it as such!
    TagReader* reader = static_cast<TagReader*>(data);

    // These values will get filled by the gst_tag_list_get_xyz() functions
    gchar* title;
    gchar* artist;
    gchar* album;
    //gchar* albumArtist;
    gchar* genre;
    gchar* comment;
    gchar* composer;
    guint trackNum;
    guint discNum;
    GDate* date;

#ifdef DEBUG
    gchar* misc;
    if (gst_tag_list_get_string(list, tag, &misc)) {
        std::cout << "Tag content: " << misc << std::endl;
    }
#endif 

    TrackInfo& trackInfo = reader->getTrackInfo();

    // Before attempting to set a tag in the reader*, make sure if the tag
    // fetching _really_ succeeded. This is simply done by checking for TRUE
    // or FALSE by the gst_tag_list_get_xyz() funcs.
    if (gst_tag_list_get_string(list, TrackInfo::TITLE, &title)) {
        trackInfo[TrackInfo::TITLE] = wxString(title, wxConvUTF8);
        g_free(title);
    }
    if (gst_tag_list_get_string(list, TrackInfo::ARTIST, &artist)) {
        trackInfo[TrackInfo::ARTIST] = wxString(artist, wxConvUTF8);
        g_free(artist);
    }
    if (gst_tag_list_get_string(list, TrackInfo::ALBUM, &album)) {
        trackInfo[TrackInfo::ALBUM] = wxString(album, wxConvUTF8);
        g_free(album);
    }
    if (gst_tag_list_get_string(list, TrackInfo::GENRE, &genre)) {
        trackInfo[TrackInfo::GENRE] = wxString(genre, wxConvUTF8);
        g_free(genre);
    }
    if (gst_tag_list_get_string(list, TrackInfo::COMMENT, &comment)) {
        trackInfo[TrackInfo::COMMENT] = wxString(comment, wxConvUTF8);
        g_free(comment);
    }
    if (gst_tag_list_get_string(list, TrackInfo::COMPOSER, &composer)) {
        trackInfo[TrackInfo::COMPOSER] = wxString(composer, wxConvUTF8);
        g_free(composer);
    }
    if (gst_tag_list_get_uint(list, TrackInfo::TRACK_NUMBER, &trackNum)) {
        trackInfo[TrackInfo::TRACK_NUMBER] = wxString::Format(wxT("%i"), trackNum);
    }
    if (gst_tag_list_get_uint(list, TrackInfo::DISC_NUMBER, &discNum)) {
        trackInfo[TrackInfo::DISC_NUMBER] = wxString::Format(wxT("%i"), discNum);
    }
    if (gst_tag_list_get_date(list, TrackInfo::DATE, &date)) {
        // XXX: date also contains month and day.. Include  this, or just year?
        unsigned int ddate = g_date_get_year(date);
        trackInfo[TrackInfo::DATE] = wxString::Format(wxT("%i"), ddate);
        g_date_free(date);
    }

    reader->setTrackInfo(trackInfo);
}

void TagReader::play() throw() {
    // override, no need to set state to GST_STATE_PLAYING.
}

void TagReader::stop() throw() {
    // override, no need to set state to GST_STATE_PAUSED and seek to 0.
}

void TagReader::initTags() throw(AudioException) {
    // set location beforehand, and not in the onTagRead, because that could be 
    // called quite a lot of times. We need to only set the location once, and
    // not 18 times in a row. Saves a few nanoseconds :p
    m_trackInfo.setLocation(getLocation());
    try {
        // try to fetch the duration by querying the pipeline.
        m_trackInfo.setDurationSeconds(getDurationSeconds());
    } catch(const AudioException& ex) {
        std::cerr << "Failed to get the track duration!" << std::endl;
    }

    GstMessage* msg = NULL;
    // XXX: making using of while(true) sounds dangerous...?
    while (true) {
        GstTagList* tags = NULL;

        // Stupid gstreamer using enum fields for bit operations... Bad boy, bad!
        GstMessageType fuEnumForBitFields = 
            static_cast<GstMessageType>(
                (unsigned) GST_MESSAGE_ASYNC_DONE | 
                (unsigned) GST_MESSAGE_TAG |
                (unsigned) GST_MESSAGE_ERROR);

        msg = gst_bus_timed_pop_filtered (
            GST_ELEMENT_BUS (m_pipeline),
            GST_CLOCK_TIME_NONE,
            fuEnumForBitFields);

        // error or async_done...
        if (GST_MESSAGE_TYPE (msg) != GST_MESSAGE_TAG) {
            break;
        }

        gst_message_parse_tag (msg, &tags);

        //std::cout << "Got tags from element: " << GST_OBJECT_NAME(msg->src) << std::endl;
        gst_tag_list_foreach (tags, onTagRead, this);
        gst_tag_list_free (tags);
        gst_message_unref (msg);
    }

    if (GST_MESSAGE_TYPE (msg) == GST_MESSAGE_ERROR) {
        gchar* debug;
        GError* error;

        gst_message_parse_error (msg, &error, &debug);
        g_free (debug);

        wxString err = wxString(error->message, wxConvUTF8);
        // Free the error BEFORE throwing the exception! We got it in 
        // the wxString anyway. 
        g_error_free (error);
        // also, unref the message.
        gst_message_unref(msg);

        // Last but not least, throw the exception
        throw AudioException(err);
    }

    // if message type wasn't error, also unref the msg pointar lulz.
    gst_message_unref(msg);
}

void TagReader::init() throw (AudioException) {
    // Element uridecodebin (gst-inspect uridecodebin)
    m_uridecodebin = gst_element_factory_make("uridecodebin", NULL);
    if (!m_uridecodebin) {
        throw AudioException(wxT("Failed to create `uridecodebin' GST element"));
    }

    // Element fakesink (gst-inspect fakesink)
    m_fakesink = gst_element_factory_make("fakesink", NULL);
    if (!m_fakesink) {
        throw AudioException(wxT("Failed to create `fakesink' GST element"));
    }

    // m_pipeline is protected in parent class Pipeline.
    m_pipeline = gst_pipeline_new(NULL);

    gst_bin_add_many(GST_BIN(m_pipeline), m_uridecodebin, m_fakesink, NULL);

    g_signal_connect (m_uridecodebin, "pad-added", G_CALLBACK (onPadAdded), m_fakesink);

    // set the uri on the uridecodebin element:
    std::string s = std::string(m_location.mb_str());
    g_object_set(m_uridecodebin, "uri", s.c_str(), NULL);

    pause();

    initTags();
}

void TagReader::setTrackInfo(const TrackInfo& trackinfo) {
    m_trackInfo = trackinfo;
}

TrackInfo& TagReader::getTrackInfo() {
    return m_trackInfo;
}

} // namespace pl
