//
// Created by Raghavasimhan Sankaranarayanan on 7/15/20.
//

#ifndef EMGDATASERVER_SERVER_H
#define EMGDATASERVER_SERVER_H

#include "TCPServer.h"
#include "CoAmp.h"
#include "KeyLogger.h"
#include "pch.h"
#include "MyTime.h"
#include "ErrorDef.h"

class Server {
public:
    enum Message {
        Stop = '0',
        Exit = '1',
        Start = '2',
    };

    // Deleting Copy constructor
    Server (const Server&) = delete;
    Server& operator= (const Server&) = delete;

    ~Server();
    static Server* getInstance();
    Error_t init(CoAmp* pSensor, int port=8080, int iPacketSize=128);
    Error_t run(int iTimeoutInSec=10);
    Error_t stop();
    bool isRunning();

private:
    Server();
    void threadCallback();
    static void signalHandler(int sig);
    void receiveCallback();

    void joinThreads();

    uint16_t* m_iTransmissionData = nullptr;

    static Server* pInstance;
    CoAmp* m_pSensor = nullptr;
    KeyLogger* m_pKeyLogger = nullptr;

    TCPServer m_server;
    std::thread* m_pThread = nullptr;
    std::thread* m_pReceiveThread = nullptr;

    std::atomic<bool> m_bRunning;

    Data_t<uint16_t>* m_piDataChunk = nullptr;
    int m_iBlockSize    = 0;
    int m_iNumChannels  = 0;

    std::mutex m_mtx;
    std::condition_variable m_cv;

    uint8_t m_iReceivedMsg = 0;
};


#endif //EMGDATASERVER_SERVER_H
