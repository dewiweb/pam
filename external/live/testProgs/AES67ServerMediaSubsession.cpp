#include "AES67ServerMediaSubsession.h"
#include <GroupsockHelper.hh>
#include <sstream>
#include <iostream>

////////// AES67ServerMediaSubsession //////////

AES67ServerMediaSubsession* AES67ServerMediaSubsession::createNew(RTPSink& rtpSink, RTCPInstance* rtcpInstance)
{
    return new AES67ServerMediaSubsession(rtpSink, rtcpInstance);
}

AES67ServerMediaSubsession::AES67ServerMediaSubsession(RTPSink& rtpSink, RTCPInstance* rtcpInstance)
    : ServerMediaSubsession(rtpSink.envir()),
    fSDPLines(NULL), fRTPSink(rtpSink), fRTCPInstance(rtcpInstance)
{
  //fClientRTCPSourceRecords = HashTable::create(ONE_WORD_HASH_KEYS);
}

AES67ServerMediaSubsession::~AES67ServerMediaSubsession()
{
    delete[] fSDPLines;
}

Boolean AES67ServerMediaSubsession::rtcpIsMuxed()
{
  if (fRTCPInstance == NULL) return False;

  // Check whether RTP and RTCP use the same "groupsock" object:
  return &(fRTPSink.groupsockBeingUsed()) == fRTCPInstance->RTCPgs();
}

char const* AES67ServerMediaSubsession::sdpLines()
{
    if (fSDPLines == NULL )
    {
        // Construct a set of SDP lines that describe this subsession:
        // Use the components from "rtpSink":
        Groupsock const& gs = fRTPSink.groupsockBeingUsed();
        AddressString groupAddressStr(gs.groupAddress());
        unsigned short portNum = ntohs(gs.port().num());
        unsigned char ttl = gs.ttl();
        unsigned char rtpPayloadType = fRTPSink.rtpPayloadType();
        char const* mediaType = fRTPSink.sdpMediaType();
        unsigned estBitrate
            = fRTCPInstance == NULL ? 50 : fRTCPInstance->totSessionBW();
        char* rtpmapLine = fRTPSink.rtpmapLine();
        char const* rtcpmuxLine = rtcpIsMuxed() ? "a=rtcp-mux\r\n" : "";
        char const* rangeLine = rangeSDPLine();
        char const* auxSDPLine = fRTPSink.auxSDPLine();

        char const* ptime = "a=ptime:1\r\n";
        char const* maxptime = "a=maxptime:1\r\n";
        char const* tsref = "a=ts-refclk:ptp=IEEE1588-2008:39-A7-94-FF-FE-07-CB-D0:0\r\n";

        std::stringstream ss;
        ss << "a=mediaclk:direct=" << GetEpochTimestamp() << "\r\n";
        std::string sMedia(ss.str());

        if (auxSDPLine == NULL) auxSDPLine = "";

        char const* const sdpFmt =
            "m=%s %d RTP/AVP %d\r\n"
            "c=IN IP4 %s/%d\r\n"
            "b=AS:%u\r\n"
            "%s"
            "%s"
            "%s"
            "%s"
            "%s"    //ptime
            "%s"    //maxptime
            "%s"    //ts-refclk
            "%s"    //mediaclk
            "a=control:%s\r\n";
        unsigned sdpFmtSize = strlen(sdpFmt)
                              + strlen(mediaType) + 5 /* max short len */ + 3 /* max char len */
                              + strlen(groupAddressStr.val()) + 3 /* max char len */
                              + 20 /* max int len */
                              + strlen(rtpmapLine)
                              + strlen(rtcpmuxLine)
                              + strlen(rangeLine)
                              + strlen(auxSDPLine)
                              + strlen(ptime)
                              + strlen(maxptime)
                              + strlen(tsref)
                              + strlen(sMedia.c_str())
                              + strlen(trackId());
        char* sdpLines = new char[sdpFmtSize];
        sprintf(sdpLines, sdpFmt,
                mediaType, // m= <media>
                portNum, // m= <port>
                rtpPayloadType, // m= <fmt list>
                groupAddressStr.val(), // c= <connection address>
                ttl, // c= TTL
                estBitrate, // b=AS:<bandwidth>
                rtpmapLine, // a=rtpmap:... (if present)
                rtcpmuxLine, // a=rtcp-mux:... (if present)
                rangeLine, // a=range:... (if present)
                auxSDPLine, // optional extra SDP line
                ptime,      // a=ptime:
                maxptime,   // a=maxptime:
                tsref,      // a=ts-refclk
                sMedia.c_str(),  // a=mediaclk
                trackId());  // a=control:<track-id>
        delete[] (char*)rangeLine;
        delete[] rtpmapLine;

        fSDPLines = strDup(sdpLines);
        delete[] sdpLines;
    }

    return fSDPLines;
}

void AES67ServerMediaSubsession
::getStreamParameters(unsigned clientSessionId,
                      netAddressBits clientAddress,
                      Port const& /*clientRTPPort*/,
                      Port const& clientRTCPPort,
                      int /*tcpSocketNum*/,
                      unsigned char /*rtpChannelId*/,
                      unsigned char /*rtcpChannelId*/,
                      netAddressBits& destinationAddress,
                      u_int8_t& destinationTTL,
                      Boolean& isMulticast,
                      Port& serverRTPPort,
                      Port& serverRTCPPort,
                      void*& streamToken)
{
    isMulticast = True;
    Groupsock& gs = fRTPSink.groupsockBeingUsed();
    if (destinationTTL == 255) destinationTTL = gs.ttl();
    if (destinationAddress == 0)   // normal case
    {
        destinationAddress = gs.groupAddress().s_addr;
    }
    else     // use the client-specified destination address instead:
    {
        struct in_addr destinationAddr;
        destinationAddr.s_addr = destinationAddress;
        gs.changeDestinationParameters(destinationAddr, 0, destinationTTL);
        if (fRTCPInstance != NULL)
        {
            Groupsock* rtcpGS = fRTCPInstance->RTCPgs();
            rtcpGS->changeDestinationParameters(destinationAddr, 0, destinationTTL);
        }
    }
    serverRTPPort = gs.port();
    if (fRTCPInstance != NULL)
    {
        Groupsock* rtcpGS = fRTCPInstance->RTCPgs();
        serverRTCPPort = rtcpGS->port();
    }
    streamToken = NULL; // not used

    // Make a record of this client's source - for RTCP RR handling:
    //RTCPSourceRecord* source = new RTCPSourceRecord(clientAddress, clientRTCPPort);
    //fClientRTCPSourceRecords->Add((char const*)clientSessionId, source);
}

void AES67ServerMediaSubsession::startStream(unsigned clientSessionId,
        void* /*streamToken*/,
        TaskFunc* rtcpRRHandler,
        void* rtcpRRHandlerClientData,
        unsigned short& rtpSeqNum,
        unsigned& rtpTimestamp,
        ServerRequestAlternativeByteHandler* /*serverRequestAlternativeByteHandler*/,
        void* /*serverRequestAlternativeByteHandlerClientData*/)
{
    rtpSeqNum = fRTPSink.currentSeqNo();
    rtpTimestamp = fRTPSink.presetNextTimestamp();

    // Try to use a big send buffer for RTP -  at least 0.1 second of
    // specified bandwidth and at least 50 KB
    unsigned streamBitrate = fRTCPInstance == NULL ? 50 : fRTCPInstance->totSessionBW(); // in kbps
    unsigned rtpBufSize = streamBitrate * 25 / 2; // 1 kbps * 0.1 s = 12.5 bytes
    if (rtpBufSize < 50 * 1024) rtpBufSize = 50 * 1024;
    increaseSendBufferTo(envir(), fRTPSink.groupsockBeingUsed().socketNum(), rtpBufSize);

    if (fRTCPInstance != NULL)
    {
        // Hack: Send a RTCP "SR" packet now, so that receivers will (likely) be able to
        // get RTCP-synchronized presentation times immediately:
        fRTCPInstance->sendReport();

        // Set up the handler for incoming RTCP "RR" packets from this client:
//        RTCPSourceRecord* source = (RTCPSourceRecord*)(fClientRTCPSourceRecords->Lookup((char const*)clientSessionId));
//        if (source != NULL)
//        {
//            fRTCPInstance->setSpecificRRHandler(source->addr, source->port,
//                                                rtcpRRHandler, rtcpRRHandlerClientData);
//        }
  }
}

float AES67ServerMediaSubsession::getCurrentNPT(void* streamToken)
{
    // Return the elapsed time between our "RTPSink"s creation time, and the current time:
    struct timeval const& creationTime  = fRTPSink.creationTime(); // alias

    struct timeval timeNow;
    gettimeofday(&timeNow, NULL);

    return (float)(timeNow.tv_sec - creationTime.tv_sec + (timeNow.tv_usec - creationTime.tv_usec)/1000000.0);
}

void AES67ServerMediaSubsession
::getRTPSinkandRTCP(void* streamToken,
                    RTPSink const*& rtpSink, RTCPInstance const*& rtcp)
{
    rtpSink = &fRTPSink;
    rtcp = fRTCPInstance;
}

void AES67ServerMediaSubsession::deleteStream(unsigned clientSessionId, void*& /*streamToken*/)
{
    // Lookup and remove the 'RTCPSourceRecord' for this client.  Also turn off RTCP "RR" handling:
//    RTCPSourceRecord* source = (RTCPSourceRecord*)(fClientRTCPSourceRecords->Lookup((char const*)clientSessionId));
//    if (source != NULL)
//    {
//        if (fRTCPInstance != NULL)
//        {
//            fRTCPInstance->unsetSpecificRRHandler(source->addr, source->port);
//        }
//
//        fClientRTCPSourceRecords->Remove((char const*)clientSessionId);
//        delete source;
//    }
}


int AES67ServerMediaSubsession::GetEpochTimestamp()
{
    int nTimestamp = fRTPSink.presetNextTimestamp();

    std::cout << "Timestamp: " << nTimestamp << std::endl;

    //get the current time
    timeval tvNow;
    gettimeofday(&tvNow, 0);

    double dNow = static_cast<double>(tvNow.tv_sec);//*1000000.0;
    dNow += static_cast<double>(tvNow.tv_usec) / 1000000.0;

    std::cout << "Frequency: " << fRTPSink.rtpTimestampFrequency() << std::endl;


    int64_t nTimestampsToMinus = static_cast<int64_t>(dNow*static_cast<double>(fRTPSink.rtpTimestampFrequency()));

    nTimestampsToMinus = nTimestampsToMinus % 42949672960;

    std::cout << "To Minus: " << nTimestampsToMinus << std::endl;

    return nTimestamp-static_cast<int>(nTimestampsToMinus);



}
