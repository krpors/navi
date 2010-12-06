//      audio.hpp
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

#ifndef AUDIO_HPP 
#define AUDIO_HPP 

#include <map>

#include <wx/wx.h>
#include <gst/gst.h>

namespace navi {

//================================================================================

/**
 * Base exception for audio failures, errors, exceptions. These include things
 * like failing to create a GstElement*, failure at playback, etc.
 */
class AudioException : std::exception {
private:
    /// Description of the exception.
    wxString m_what;
public:
    /**
     * Creates an AudioException object with descriptive text.
     * 
     * @param wut The exception's description.
     */
    AudioException(const wxString& wut) throw();

    /**
     * Destructor.
     */
    ~AudioException() throw();

    /**
     * Returns the exception's description as a const char*.
     * 
     * @return The description as a const char pointer (overload from
     *  std::exception)
     */
    const char* what() const throw();

    /**
     * Gets this exception as a wxString, which is ideal to immediately use
     * in a user interface popup, message box, debug location or whatever.
     *
     * @return The exception's description as a const wxString.
     */
    const wxString& getAsWxString() const throw();
};



//================================================================================

/**
 * Base class for every Pipeline implementation for Navi. It's a tiny
 * wrapper around the Gst library.
 */
class Pipeline {
protected:
    /// The location of the file or stream to play.
    wxString m_location;

    /// The gstreamer bus (one per pipeline).
    GstBus* m_bus;

    /// The main pipeline element (with GstElement pointer members).
    GstElement* m_pipeline;

    /**
     * Callback for Gst bus messages. Allows us to listen to messages and 
     * possibly notify users about those messages, or for other debugging
     * purposes.
     *
     * @param bus The GstBus pointer.
     * @param message The GstMessage pointer ("what happened?").
     * @param userdata Custom data for usage. In this case it will be the Pipeline
     *  instance which is registering the callback.
     * @return TRUE when the watch should still be active. FALSE if it should
     * be removed.
     */
    static gboolean busWatcher(GstBus* bus, GstMessage* message, gpointer userdata);

    /// Initialization for a pipeline. Pure virtual.
    virtual void init() throw (AudioException) = 0;

public:
    /**
     * Constructs a pipeline.
     */
    Pipeline() throw();
    
    /**
     * Pipeline destructor. This will set the state of the current pipeline
     * pointer m_pipeline to NULL, and will 'unreference' the GstBus. After
     * that, the m_pipeline will be unreffed, which automatically unrefs the
     * pipeline members. Therefore, derived classes of Pipeline do not have to
     * manually unref their own pipeline elements.
     */
    virtual ~Pipeline();

    /**
     * Plays the pipeline (GST_STATE_PLAYING). Declared virtual, so derived
     * classes can implement their own way of playing the audio streams.
     */
    virtual void play() throw();

    /**
     * Pauses the pipeline (GST_STATE_PAUSED). Declared virtual as well.
     */
    virtual void pause() throw();

    /**
     * Stops playback of the pipeline. This actually sets the current pipeline
     * state to GST_STATE_PAUSED, and then seeks to the start
     */
    virtual void stop() throw();

    /**
     * Seeks in the current pipeline using an amount of seconds in the
     * pipeline. In Pipelines which are unseekable (such as live streams), this
     * function should do nothing to the pipeline (perhaps giving a warning message,
     * but this is implementation dependent).
     *
     * @param seconds The amount of seconds to 'seek' (i.e. jump) to.
     */
    virtual void seekSeconds(const unsigned short seconds) throw (AudioException);

    /**
     * Gets the duration in seconds from the stream, if applicable. For instance,
     * live streams cannot report a duration and will return -1 as duration. 
     * File streams (filesrc based) can. This function requires the pipeline to 
     * be set on state PAUSED (by invoking pause()) or PLAYING (by invoking 
     * play()) first!
     *
     * Subclasses can override this function to provide specialized behaviour
     * (like live streamed pipelines).
     *
     * @throw AudioException when fetching the duration failed (meaning that 
     *   gst_element_get_state returned GST_STATE_CHANGE_FAILURE), or the
     *   duration itself was -1 (-1 was actually returned from the 
     *   gst_element_query_duration call).
     */
    virtual int getDurationSeconds() throw(AudioException);

    /**
     * Returns the location, whether it be a URL or file location.
     */
    const wxString& getLocation() const throw();

};

//================================================================================

/**
 * MP3 file pipeline object. Creates a pipeline that can read an MPEG1 layer 3
 * audio file using the MAD decoder, audio converter and sink.
 */
class MP3FilePipeline: public Pipeline {
private:

    /// The file source.
    GstElement* m_filesrc;

    /// MP3 parser (MikeS-tp@#gstreamer irc.freenode.org) 
    GstElement* m_mp3parser;

    /// The MAD decoder.
    GstElement* m_maddec;

    /// The audio converter.
    GstElement* m_aconvert;

    /// Sink.
    GstElement* m_sink;

protected:
    /**
     * Initializes the pipeline.
     * 
     * @throws AudioException when any of the GstElements can not be created.
     */
    void init() throw (AudioException);

public:
    /**
     * Creates a new MP3 file pipeline, using location as file source.
     *
     * @param location The location on disk.
     */
    MP3FilePipeline(const wxString& location) throw (AudioException) ;

    /**
     * Destructor.
     */
    ~MP3FilePipeline(); 

};

//================================================================================

/**
 * An OGG file pipeline object. This can create a pipeline using a (local) OGG
 * file as a source. Using a demuxer, decoder, audioconverter and a sink, it will
 * play the file.
 */
class OGGFilePipeline : public Pipeline {
private:

    /// File source
    GstElement* m_filesrc; 

    /// OGG Demuxer
    GstElement* m_demux;

    /// Vorbis Decoder
    GstElement* m_decoder;

    /// Audio converter
    GstElement* m_aconvert;

    /// Audio sink.
    GstElement* m_sink;
    
    /**
     * Necessary to play OGG files. See Section 8.1.1 in the development manual and
     * the `Hello World' example application of the gstreamer documentation on why 
     * this is needed for Ogg Vorbis. Basically:
     * 
     * "Note that the demuxer will be linked to the decoder dynamically. The reason
     * is that Ogg may contain various streams (for example audio and video). The 
     * source pad(s) will be created at run time by the demuxer when it detects the
     * mount and nature of streams. Therefore we connect a callback function which 
     * will be executed when the "pad-added" is emitted."
     *
     * @param element the element (usually the demuxer)
     * @param pad The sinkpad?
     * @param data The decoder GstElement pointer.
     */
    static void onPadAdded(GstElement* element, GstPad* pad, gpointer data) throw();

protected:
    /**
     * Override from class Pipeline.
     */
    void init() throw (AudioException);

public:
    /**
     * Create a new OGG file pipeline, given a location on disk.
     * 
     * @param location The OGG file's location.
     */
    OGGFilePipeline(const wxString& location) throw (AudioException);

    /**
     * Destructor.
     */
    ~OGGFilePipeline();

};

//================================================================================

/**
 * Base class for tag reading. Since reading tags from a file or stream requires
 * a pipeline, TagReader is a derived class from Pipeline. The location used in
 * this class is expected to be a valid URI, for example file:///home/user/file.mp3
 * or file:///tmp/otherfile.ogg etc.
 */
class TagReader : public Pipeline {
private:

    /// Map with const char* (TagReader::TAG_XYZ) to wxString (value) mapping
    std::map<const char*, wxString> m_tags;

    /// The URI Decode bin. Loads `proto://path/like.mp3' structures
    GstElement* m_uridecodebin;

    /// The fake sink. We do not need output to read tags.
    GstElement* m_fakesink;
    
    /**
     * This is necessary for tag reading, apparently. See the gstreamer documentation
     * at http://www.gstreamer.net/data/doc/gstreamer/head/manual/html/chapter-metadata.html .
     * I used that source code for this TagReader class.
     *
     * @param element the element (usually the demuxer)
     * @param pad The sinkpad?
     * @param fakesink The fakesink.
     */
    static void onPadAdded(GstElement* element, GstPad* pad, GstElement* fakesink) throw();

    /**
     * This function is called as a callback in initTags(). For each tag found, this
     * function will function will be invoked.
     *
     * @param list The tag list pointer
     * @param tag The tag which is read?
     * @param 
     */
    static void onTagRead(const GstTagList* list, const gchar* tag, gpointer data) throw();

    /**
     * Initializes the tag reading thangs.
     * 
     * @throws AudioException when some stupid things happen with fetching
     *  messages types and shit.
     */
    virtual void initTags() throw (AudioException);

protected:
    /**
     * Initializes a default pipeline (filesrc and fakesink). Also declared virtual.
     */
    virtual void init() throw (AudioException);

public:
    /// Title of the stream (GST_TAG_TITLE)
    static const char* TAG_TITLE;

    /// Artist of the stream (GST_TAG_ARTIST)
    static const char* TAG_ARTIST;

    /// Album of the stream (GST_TAG_ALBUM)
    static const char* TAG_ALBUM;
    
    /// Individual artist of the stream (GST_TAG_ALBUM_ARTIST)
    static const char* TAG_ALBUM_ARTIST;

    /// Genre of the stream (GST_TAG_GENRE)
    static const char* TAG_GENRE;
    
    /// Comment of the stream (GST_TAG_COMMENT)
    static const char* TAG_COMMENT;

    /// Composer of the stream (GST_TAG_COMPOSER)
    static const char* TAG_COMPOSER;

    /// Track number of the stream (GST_TRACK_TITLE)
    static const char* TAG_TRACK_NUMBER;

    /// Disc number of the stream (GST_TAG_VOLUME_NUMBER)
    static const char* TAG_DISC_NUMBER;

    /// Date (currently, only the yeard) of the stream (GST_TAG_DATE)
    static const char* TAG_DATE;

    /**
     * Constructs a TagReader instance. The location parameter should be
     * a URI, in the form of file:///home/user/file.mp3 or the like.
     *
     * @param location The location URI to use
     * @throw AudioException when initializing failed (like the pipeline).
     */
    TagReader(const wxString& location) throw(AudioException);

    /**
     * Krush, Kill 'n Destroy.
     */
    virtual ~TagReader();

    /**
     * Allows us to get and set tags using tag[TAG_XYZ] and tag[TAG_XYZ] = "lol".
     *
     * @param key The key get or set.
     */
    wxString& operator[](const char* key);

    /**
     * play() is overridden to do nothing.
     */
    void play() throw();

    /**
     * stop() is overridden to do nothing.
     */
    void stop() throw();

    /**
     * Gets all parsed tags as a const wxString ref.
     *
     * @return The wxString with parsed metadata tags.
     */
    const wxString getAsString() throw();
};

} // namespace navi 

#endif // AUDIO_HPP