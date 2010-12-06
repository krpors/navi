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

Pipeline::Pipeline() throw() :
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
} 

gboolean Pipeline::busWatcher(GstBus* bus, GstMessage* message, gpointer userdata) {
    // get the name of the message. 
    const gchar* name = GST_MESSAGE_TYPE_NAME(message);

    std::cout << "Got GST message: " << name << std::endl;

    switch (GST_MESSAGE_TYPE (message)) {
        case GST_MESSAGE_EOS:
            std::cout << "End of stream" << std::endl;
            break;

        case GST_MESSAGE_ERROR:
            gchar* debug;
            GError* error;

            gst_message_parse_error (message, &error, &debug);
            g_free (debug);

            g_printerr ("Error: %s\n", error->message);
            g_error_free (error);

            break;

        default:
            break;
    }
  
    return TRUE;
}

void Pipeline::play() throw() {
    gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
}

void Pipeline::pause() throw() {
    gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
}

void Pipeline::stop() throw() {
    // pause first (i.e. stop playback)
    pause();
    // then 'seek' to the start of the file.
    gboolean seekSuccess = gst_element_seek (m_pipeline, 1.0, 
        GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
        GST_SEEK_TYPE_SET, 0,
        GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);

    if (!seekSuccess) {
        std::cerr << "Seek failed!" << std::endl;
    }
}

const wxString& Pipeline::getLocation() const throw() {
    return m_location;
}

void Pipeline::seekSeconds(const unsigned short seconds) throw(AudioException) {
    // default pipeline implementation allows seeking in a file
    //gboolean success = gst_element_seek_simple(
    //    m_pipeline, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, 60 * GST_SECOND);
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


//==============================================================================

MP3FilePipeline::MP3FilePipeline(const wxString& location) throw (AudioException) {
    // set the location in the parent class `Pipeline':
    m_location = location;
    // initialize the pipeline further
    try {
        init();
    } catch (const AudioException& ex) {
        // rethrow
        throw;
    }
}

MP3FilePipeline::~MP3FilePipeline() {
}


void MP3FilePipeline::init() throw (AudioException) {
    // This init function uses element factories to make the gst elements. All
    // the element names are given the value NULL to let gst decide what name
    // the elements get. 

    // Element filesrc (gst-inspect filesrc)
    m_filesrc = gst_element_factory_make("filesrc", NULL);
    if (!m_filesrc) {
        throw AudioException(wxT("Failed to create `filesrc' GST element"));
    }

    // Element MP3 parser allows us to successfully seek into the stream. This 
    // thanks to a hint provided by MikeS-tp from #gstreamer at Freenode.
    m_mp3parser = gst_element_factory_make("mp3parse", NULL);
    if (!m_mp3parser) {
        throw AudioException(wxT("Failed to create `mp3parse' GST element"));
    }

    // Element filesrc (gst-inspect mad)
    m_maddec = gst_element_factory_make("mad", NULL);
    if (!m_maddec) {
        throw AudioException(wxT("Failed to create `mad' GST element"));
    }
    
    // Element audioconvert (gst-inspect audioconvert)
    m_aconvert = gst_element_factory_make("audioconvert", NULL);
    if (!m_aconvert) {
        throw AudioException(wxT("Failed to create `audioconvert' GST element"));
    }

    // Element autoaudiosink (gst-inspect autoaudiosink)
    m_sink = gst_element_factory_make("autoaudiosink", NULL);
    if (!m_sink) {
        throw AudioException(wxT("Failed to create `autoaudiosink' GST element"));
    }

    // m_pipeline and m_bus are protected in parent class Pipeline.
    m_pipeline = gst_pipeline_new(NULL);

    m_bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
    // Add a watch to this bus (get notified of events using callbacks).
    // See http://www.parashift.com/c++-faq-lite/pointers-to-members.html, section
    // 33.2 for more details why it's done like this.
    gst_bus_add_watch (m_bus, Pipeline::busWatcher, this);

    gst_bin_add_many(GST_BIN(m_pipeline), m_filesrc, m_mp3parser, m_maddec, m_aconvert, m_sink, NULL);
    gst_element_link_many(m_filesrc, m_mp3parser, m_maddec, m_aconvert, m_sink, NULL);

    // set the "location" property on the filesrc element.
    std::string s = std::string(m_location.mb_str());
    g_object_set(m_filesrc, "location", s.c_str(), NULL);

    // We set the state of the element as paused, so we can succesfully query
    // duration and other stuff. If the state is still not PAUSED or PLAYING, 
    // fetching the duration has no (real and useful) effect. It may return random
    // data.
    pause(); 
}

//================================================================================

OGGFilePipeline::OGGFilePipeline(const wxString& location) throw (AudioException) {
    m_location = location;

    try {
        init();
    } catch (const AudioException& ex) {
        // rethrow
        throw;
    }
}

OGGFilePipeline::~OGGFilePipeline() {
}

void OGGFilePipeline::init() throw (AudioException) {
    // This init function uses element factories to make the gst elements. All
    // the element names are given the value NULL to let gst decide what name
    // the elements get. 

    // Element filesrc (gst-inspect filesrc)
    m_filesrc = gst_element_factory_make("filesrc", NULL);
    if (!m_filesrc) {
        throw AudioException(wxT("Failed to create `filsrc' GST element"));
    }

    // Element filesrc (gst-inspect oggdemux)
    m_demux = gst_element_factory_make("oggdemux", NULL);
    if (!m_demux) {
        throw AudioException(wxT("Failed to create `oggdemux' GST element"));
    }
    
    // Element decoder (gst-inspect decoder)
    m_decoder = gst_element_factory_make("vorbisdec", NULL);
    if (!m_decoder) {
        throw AudioException(wxT("Failed to create `vorbisdec' GST element"));
    }

    // Element audioconvert (gst-inspect audioconvert)
    m_aconvert = gst_element_factory_make("audioconvert", NULL);
    if (!m_aconvert) {
        throw AudioException(wxT("Failed to create `audioconvert' GST element"));
    }

    // Element alsasink (gst-inspect alsasink)
    m_sink = gst_element_factory_make("autoaudiosink", NULL);
    if (!m_sink) {
        throw AudioException(wxT("Failed to create `alsasink' GST element"));
    }

    // m_pipeline and m_bus are protected in parent class Pipeline.
    m_pipeline = gst_pipeline_new(NULL);

    m_bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));

    gst_bin_add_many(GST_BIN(m_pipeline), m_filesrc, m_demux, m_decoder, m_aconvert, m_sink, NULL);

    gst_element_link(m_filesrc, m_demux);
    gst_element_link_many(m_decoder, m_aconvert, m_sink, NULL);
    g_signal_connect (m_demux, "pad-added", G_CALLBACK (onPadAdded), m_decoder);

    // set location
    std::string s = std::string(m_location.mb_str());
    g_object_set(m_filesrc, "location", s.c_str(), NULL);

    pause();
}

void OGGFilePipeline::onPadAdded(GstElement* element, GstPad* pad, gpointer data) throw() {
    GstPad* sinkpad;
    GstElement* decoder = (GstElement*) data;

    sinkpad = gst_element_get_static_pad (decoder, "sink");

    gst_pad_link (pad, sinkpad);
    gst_object_unref (sinkpad);
}

//==============================================================================

const char* TagReader::TAG_TITLE = GST_TAG_TITLE;
const char* TagReader::TAG_ARTIST = GST_TAG_ARTIST;
const char* TagReader::TAG_ALBUM = GST_TAG_ALBUM;
const char* TagReader::TAG_ALBUM_ARTIST = GST_TAG_ALBUM_ARTIST;
const char* TagReader::TAG_GENRE = GST_TAG_GENRE;
const char* TagReader::TAG_COMMENT = GST_TAG_COMMENT;
const char* TagReader::TAG_COMPOSER = GST_TAG_COMPOSER;
const char* TagReader::TAG_TRACK_NUMBER = GST_TAG_TRACK_NUMBER;
const char* TagReader::TAG_DISC_NUMBER = GST_TAG_ALBUM_VOLUME_NUMBER;
const char* TagReader::TAG_DATE = GST_TAG_DATE;

TagReader::TagReader(const wxString& location) throw(AudioException) {
    m_location = location;
    try {
        init();
    } catch (const AudioException& ex) {
        throw;
    }
}

TagReader::~TagReader() {
    // not really necessary iirc... :
    m_tags.clear();

    std::cout << " Destroying tagreader " << std::endl;
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
    gchar* albumArtist;
    gchar* genre;
    gchar* comment;
    gchar* composer;
    guint trackNum;
    guint discNum;
    GDate* date;

    // Before attempting to set a tag in the reader*, make sure if the tag
    // fetching _really_ succeeded. This is simply done by checking for TRUE
    // or FALSE by the gst_tag_list_get_xyz() funcs.
    if (gst_tag_list_get_string(list, TAG_TITLE, &title)) {
        (*reader)[TAG_TITLE] = wxString(title, wxConvUTF8);
    }
    if (gst_tag_list_get_string(list, TAG_ARTIST, &artist)) {
        (*reader)[TAG_ARTIST] = wxString(artist, wxConvUTF8);
    }
    if (gst_tag_list_get_string(list, TAG_ALBUM, &album)) {
        (*reader)[TAG_ALBUM] = wxString(album, wxConvUTF8);
    }
    if (gst_tag_list_get_string(list, TAG_ALBUM_ARTIST, &albumArtist)) {
        (*reader)[TAG_ALBUM_ARTIST] = wxString(albumArtist, wxConvUTF8);
    }
    if (gst_tag_list_get_string(list, TAG_GENRE, &genre)) {
        (*reader)[TAG_GENRE] = wxString(genre, wxConvUTF8);
    }
    if (gst_tag_list_get_string(list, TAG_COMMENT, &comment)) {
        (*reader)[TAG_COMMENT] = wxString(comment, wxConvUTF8);
    }
    if (gst_tag_list_get_string(list, TAG_COMPOSER, &composer)) {
        (*reader)[TAG_COMPOSER] = wxString(composer, wxConvUTF8);
    }
    if (gst_tag_list_get_uint(list, TAG_TRACK_NUMBER, &trackNum)) {
        (*reader)[TAG_TRACK_NUMBER] = wxString::Format(wxT("%i"), trackNum);
    }
    if (gst_tag_list_get_uint(list, TAG_DISC_NUMBER, &discNum)) {
        (*reader)[TAG_DISC_NUMBER] = wxString::Format(wxT("%i"), discNum);
    }
    if (gst_tag_list_get_date(list, TAG_DATE, &date)) {
        // XXX: date also contains month and day.. Include  this, or just year?
        unsigned int ddate = g_date_get_year(date);
        (*reader)[TAG_DATE] = wxString::Format(wxT("%i"), ddate);
    }

#if 0
    int i, num;

    num = gst_tag_list_get_tag_size (list, tag);
    for (i = 0; i < num; ++i) {
        const GValue *val;

        /* Note: when looking for specific tags, use the g_tag_list_get_xyz() API,
        * we only use the GValue approach here because it is more generic */
        val = gst_tag_list_get_value_index (list, tag, i);
        if (G_VALUE_HOLDS_STRING (val)) {
            g_print ("Str:\t%20s : %s\n", tag, g_value_get_string (val));
        } else if (G_VALUE_HOLDS_UINT (val)) {
            g_print ("Uint: \t%20s : %u\n", tag, g_value_get_uint (val));
        } else if (G_VALUE_HOLDS_DOUBLE (val)) {
            g_print ("Double: \t%20s : %g\n", tag, g_value_get_double (val));
        } else if (G_VALUE_HOLDS_BOOLEAN (val)) {
            g_print ("Bool: \t%20s : %s\n", tag,
                (g_value_get_boolean (val)) ? "true" : "false");
        } else if (GST_VALUE_HOLDS_BUFFER (val)) {
            g_print ("\t%20s : buffer of size %u\n", tag,
                GST_BUFFER_SIZE (gst_value_get_buffer (val)));
        } else if (GST_VALUE_HOLDS_DATE (val)) {
            g_print ("Date: \t%20s : date (year=%u,...)\n", tag,
                g_date_get_year (gst_value_get_date (val)));
        } else {
            g_print ("\t%20s : tag of type '%s'\n", tag, G_VALUE_TYPE_NAME (val));
        }
    }
#endif
}

void TagReader::play() throw() {
    // override, no need to set state to GST_STATE_PLAYING.
}

void TagReader::stop() throw() {
    // override, no need to set state to GST_STATE_PAUSED and seek to 0.
}

void TagReader::initTags() throw(AudioException) {
    GstMessage* msg = NULL;
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

        // std::cout << "Got tags from element: " << GST_OBJECT_NAME(msg->src) << std::endl;
        gst_tag_list_foreach (tags, onTagRead, this);
        gst_tag_list_free (tags);
        gst_message_unref (msg);
    }

    if (GST_MESSAGE_TYPE (msg) == GST_MESSAGE_ERROR) {
        gchar* debug;
        GError* error;

        gst_message_parse_error (msg, &error, &debug);
        std::cout << debug << std::endl;
        g_free (debug);

        //g_printerr ("Error lulz: %s\n", error->message);
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
    m_uridecodebin= gst_element_factory_make("uridecodebin", NULL);
    if (!m_uridecodebin) {
        throw AudioException(wxT("Failed to create `filesrc' GST element"));
    }

    // Element filesrc (gst-inspect fakesink)
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

wxString& TagReader::operator[](const char* key) {
    return m_tags[key];
}

const wxString TagReader::getAsString() throw() {
    wxString str;
    str << wxT("Artist: ") << m_tags[TAG_ARTIST];
    str << wxT(", Title: ") << m_tags[TAG_TITLE];
    str << wxT(", Album: ") << m_tags[TAG_ALBUM];
    str << wxT(", Genre: ") << m_tags[TAG_GENRE];
    str << wxT(", Track number: ") << m_tags[TAG_TRACK_NUMBER];
    str << wxT(", Disc number: ") << m_tags[TAG_DISC_NUMBER];
    str << wxT(", Date: ") << m_tags[TAG_DATE];
    
    return str;
}

} // namespace pl
