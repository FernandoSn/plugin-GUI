/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef __NETWORKEVENT_H_91811541__
#define __NETWORKEVENT_H_91811541__
//#define ZEROMQ

#ifdef ZEROMQ
    #ifdef WIN32
        //#pragma comment( lib, "../../Resources/windows-libs/ZeroMQ/lib_x64/libzmq-v120-mt-4_0_4.lib" )
        #include <zmq.h>
        #include <zmq_utils.h>
    #else
        #include <zmq.h>
    #endif
#endif

#include <ProcessorHeaders.h>

#include <list>
#include <queue>


/**
 Sends incoming TCP/IP messages from 0MQ to the events buffer

  @see GenericProcessor
*/
class NetworkEvents : public GenericProcessor
                    , public Thread
{
public:
    NetworkEvents();
    ~NetworkEvents();

    // GenericProcessor methods
    // =========================================================================
    AudioProcessorEditor* createEditor() override;

    void process (AudioSampleBuffer& buffer) override;

    void createEventChannels() override;

    void setEnabledState (bool newState) override;

    void saveCustomParametersToXml (XmlElement* parentElement) override;
    void loadCustomParametersFromXml() override;

    bool isReady() override;

    float getDefaultSampleRate() const override;
    float getDefaultBitVolts()   const override;

    // =========================================================================

    void run() override;

    // passing 0 corresponds to wildcard ("*") and picks any available port
    // returns true on success, false on failure
    bool setNewListeningPort (uint16 port);

    // gets a string for the editor's port input to reflect current urlport
    String getPortString() const;

    void restartConnection();

private:
    struct StringTS
    {
        String str;
        int64 timestamp;
    };

    class ZMQContext : public ReferenceCountedObject
    {
    public:
        ZMQContext(const ScopedLock& lock);
        ~ZMQContext() override;
        void* createSocket();

        typedef ReferenceCountedObjectPtr<ZMQContext> Ptr;

    private:
        void* context;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZMQContext);
    };

    // RAII wrapper for REP socket
    class Responder
    {
    public:
        // tries to create a responder and bind to given port; returns nullptr on failure.
        static ScopedPointer<Responder> makeResponder(uint16 port);
        ~Responder();

        // returns the latest errno value
        int getErr() const;

        // output last error on stdout and status bar, including the passed message
        void reportErr(const String& message) const;

        // returns the port if the socket was successfully bound to one.
        // if not, or if the socket is invalid, returns 0.
        uint16 getBoundPort() const;

        // receives message into buf (blocking call).
        // returns the number of bytes actually received, or -1 if there is an error.
        int receive(void* buf);

        // sends a message. returns the same as zmq_send.
        int send(StringRef response);

    private:
        // creates socket from given context and tries to bind to port.
        // if port is 0, chooses an available ephemeral port.
        Responder(uint16 port);

        ZMQContext::Ptr context;
        void* socket;
        uint16 boundPort; // 0 indicates not bound
        int lastErrno;

        static const int RECV_TIMEOUT_MS;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Responder);
    };

    void postTimestamppedStringToMidiBuffer(const StringTS& s);
    
    String handleSpecialMessages(const String& s);

    //* Split network message into name/value pairs (name1=val1 name2=val2 etc) */
    StringPairArray parseNetworkMessage(StringRef msg);

    // updates urlport and the port input on the editor (0 indicates not connected)
    void updatePort(uint16 port);

    // get an endpoint url for the given port (using 0 to represent *)
    static String getEndpoint(uint16 port);


    // share a "dumb" pointer that doesn't take part in reference counting.
    // want the context to be terminated by the time the static members are
    // destroyed (see: https://github.com/zeromq/libzmq/issues/1708)
    static ZMQContext* sharedContext;
    static CriticalSection sharedContextLock;

    // To switch ports, a new socket is created and (if successful) assigned to this pointer,
    // and then the thread will switch to using this socket at the next opportunity.
    ScopedPointer<Responder> nextResponder;
    CriticalSection nextResponderLock;

    bool restart;
    bool changeResponder;

    uint16 urlport;   // 0 indicates not connected

    std::queue<StringTS> networkMessagesQueue;
    CriticalSection queueLock;
    
	const EventChannel* messageChannel{ nullptr };
    const EventChannel* TTLChannel{ nullptr };

    bool triggerEvent(juce::int64 bufferTs, int eventChannel, bool OnOff);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NetworkEvents);
};

#endif  // __NETWORKEVENT_H_91811541__
